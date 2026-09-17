#include <QFile>

#include <rbl_logger.h>

#include "gl_state_cache.h"
#include "render_matrix_stack.h"
#include "rhi_renderer.h"

RhiRenderer *RhiRenderer::currentRenderer = nullptr;
QSet<QRhi*> RhiRenderer::liveDevices;

namespace
{

    //! Vertex attribute locations — must match src/shaders/rhi/main.vert.
    const int kPositionLocation = 0;
    const int kNormalLocation   = 1;
    const int kColorLocation    = 2;
    const int kTexCoordLocation = 3;

    //! Initial number of uniform slots allocated per frame.
    const int kInitialUniformSlots = 1024;

    //! Translate an OpenGL depth comparison function to its QRhi equivalent.
    QRhiGraphicsPipeline::CompareOp toCompareOp(GLenum depthFunc)
    {
        switch (depthFunc)
        {
            case GL_NEVER:    return QRhiGraphicsPipeline::Never;
            case GL_LESS:     return QRhiGraphicsPipeline::Less;
            case GL_EQUAL:    return QRhiGraphicsPipeline::Equal;
            case GL_LEQUAL:   return QRhiGraphicsPipeline::LessOrEqual;
            case GL_GREATER:  return QRhiGraphicsPipeline::Greater;
            case GL_NOTEQUAL: return QRhiGraphicsPipeline::NotEqual;
            case GL_GEQUAL:   return QRhiGraphicsPipeline::GreaterOrEqual;
            case GL_ALWAYS:   return QRhiGraphicsPipeline::Always;
            default:          return QRhiGraphicsPipeline::LessOrEqual;
        }
    }

    //! Write a QMatrix4x4 into a float[16] uniform member.
    void writeMatrix(float *dst, const QMatrix4x4 &matrix)
    {
        const float *src = matrix.constData();
        for (int i = 0; i < 16; i++)
        {
            dst[i] = src[i];
        }
    }

}

RhiRenderer::RhiRenderer()
    : rhi(nullptr)
    , sampleCount(1)
    , renderPassDescriptor(nullptr)
    , uniformBuffer(nullptr)
    , uniformSlotSize(0)
    , uniformSlotCapacity(0)
    , immediateBuffer(nullptr)
    , immediateBufferSize(0)
    , sampler(nullptr)
    , whiteTexture(nullptr)
    , activeColorMap(nullptr)
    , resourceUpdates(nullptr)
    , outputSize(1, 1)
    , minDepth(0.0f)
    , maxDepth(1.0f)
{
}

RhiRenderer::~RhiRenderer()
{
    this->releaseResources();
}

RhiRenderer *RhiRenderer::current()
{
    return RhiRenderer::currentRenderer;
}

void RhiRenderer::setCurrent(RhiRenderer *renderer)
{
    RhiRenderer::currentRenderer = renderer;
}

QRhi *RhiRenderer::getRhi() const
{
    return this->rhi;
}

bool RhiRenderer::isValid() const
{
    return this->rhi != nullptr && this->vertexShader.isValid() && this->fragmentShader.isValid();
}

bool RhiRenderer::loadShaders()
{
    static const char *kVertexShaderPath   = ":/shaders/rhi/main.vert.qsb";
    static const char *kFragmentShaderPath = ":/shaders/rhi/main.frag.qsb";

    auto readShader = [](const char *path, QShader &shader)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            RLogger::error("RhiRenderer: failed to open shader resource \'%s\'\n", path);
            return false;
        }
        shader = QShader::fromSerialized(file.readAll());
        file.close();
        if (!shader.isValid())
        {
            RLogger::error("RhiRenderer: shader resource \'%s\' is not a valid .qsb file\n", path);
            return false;
        }
        return true;
    };

    if (!readShader(kVertexShaderPath, this->vertexShader))
    {
        return false;
    }
    if (!readShader(kFragmentShaderPath, this->fragmentShader))
    {
        return false;
    }
    return true;
}

bool RhiRenderer::initialize(QRhi *rhi, QRhiRenderPassDescriptor *rpDesc, int sampleCount)
{
    if (this->rhi != rhi)
    {
        // The device changed — every resource has to be rebuilt from scratch.
        this->releaseResources();
        this->rhi = rhi;
    }
    else if (this->sampleCount != sampleCount || this->renderPassDescriptor != rpDesc)
    {
        // Only the pipelines are tied to the render pass and the sample count;
        // buffers, textures and the sampler survive a resize untouched.
        for (QRhiGraphicsPipeline *pipeline : std::as_const(this->pipelines))
        {
            delete pipeline;
        }
        this->pipelines.clear();
    }
    this->sampleCount = sampleCount;
    this->renderPassDescriptor = rpDesc;

    if (!this->rhi)
    {
        return false;
    }

    if (!this->vertexShader.isValid() || !this->fragmentShader.isValid())
    {
        if (!this->loadShaders())
        {
            return false;
        }
    }

    if (!this->sampler)
    {
        this->sampler = this->rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                              QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!this->sampler->create())
        {
            RLogger::error("RhiRenderer: failed to create sampler\n");
            return false;
        }
    }

    if (!this->whiteTexture)
    {
        this->whiteTexture = this->rhi->newTexture(QRhiTexture::RGBA8, QSize(1, 1));
        if (!this->whiteTexture->create())
        {
            RLogger::error("RhiRenderer: failed to create fallback texture\n");
            return false;
        }
        QImage white(1, 1, QImage::Format_RGBA8888);
        white.fill(QColor(255, 255, 255, 255));
        this->getResourceUpdates()->uploadTexture(this->whiteTexture, white);
    }

    this->clipCorrection = this->rhi->clipSpaceCorrMatrix();

    RhiRenderer::liveDevices.insert(this->rhi);

    return true;
}

void RhiRenderer::releaseResources()
{
    for (QRhiGraphicsPipeline *pipeline : std::as_const(this->pipelines))
    {
        delete pipeline;
    }
    this->pipelines.clear();

    for (QRhiShaderResourceBindings *srb : std::as_const(this->bindings))
    {
        delete srb;
    }
    this->bindings.clear();

    for (QRhiTexture *texture : std::as_const(this->colorMapTextures))
    {
        delete texture;
    }
    this->colorMapTextures.clear();
    this->activeColorMap = nullptr;

    delete this->whiteTexture;
    this->whiteTexture = nullptr;

    delete this->sampler;
    this->sampler = nullptr;

    delete this->uniformBuffer;
    this->uniformBuffer = nullptr;
    this->uniformSlotCapacity = 0;

    delete this->immediateBuffer;
    this->immediateBuffer = nullptr;
    this->immediateBufferSize = 0;

    this->resourceUpdates = nullptr;
    this->drawItems.clear();
    this->uniformBlocks.clear();
    this->immediateVertices.clear();
    if (this->rhi)
    {
        RhiRenderer::liveDevices.remove(this->rhi);
    }
    this->rhi = nullptr;
    this->renderPassDescriptor = nullptr;
    this->uniformSlotSize = 0;
}

QRhiResourceUpdateBatch *RhiRenderer::getResourceUpdates()
{
    if (!this->resourceUpdates && this->rhi)
    {
        this->resourceUpdates = this->rhi->nextResourceUpdateBatch();
    }
    return this->resourceUpdates;
}

void RhiRenderer::beginFrame(const QSize &outputSize)
{
    this->outputSize = outputSize.isEmpty() ? QSize(1, 1) : outputSize;
    this->drawItems.clear();
    this->uniformBlocks.clear();
    this->immediateVertices.clear();
    this->minDepth = 0.0f;
    this->maxDepth = 1.0f;
    this->activeColorMap = nullptr;
}

void RhiRenderer::setLights(const std::vector<QVector4D> &positions,
                            const std::vector<QVector4D> &ambients,
                            const std::vector<QVector4D> &diffuses)
{
    this->lightPositions = positions;
    this->lightAmbients = ambients;
    this->lightDiffuses = diffuses;
}

void RhiRenderer::setDepthRange(float minDepth, float maxDepth)
{
    this->minDepth = minDepth;
    this->maxDepth = maxDepth;
}

QRhiViewport RhiRenderer::currentViewport() const
{
    int viewport[4];
    RenderMatrixStack::getViewport(viewport);

    float x = float(viewport[0]);
    float y = float(viewport[1]);
    float w = float(viewport[2]);
    float h = float(viewport[3]);

    if (w <= 0.0f || h <= 0.0f)
    {
        x = 0.0f;
        y = 0.0f;
        w = float(this->outputSize.width());
        h = float(this->outputSize.height());
    }

    return QRhiViewport(x, y, w, h, this->minDepth, this->maxDepth);
}

void RhiRenderer::setColorMap(const QString &key, const QImage &image)
{
    this->activeColorMap = this->acquireColorMapTexture(key, image);
}

void RhiRenderer::clearColorMap()
{
    this->activeColorMap = nullptr;
}

QRhiTexture *RhiRenderer::acquireColorMapTexture(const QString &key, const QImage &image)
{
    if (!this->rhi || image.isNull())
    {
        return nullptr;
    }

    QHash<QString,QRhiTexture*>::ConstIterator it = this->colorMapTextures.constFind(key);
    if (it != this->colorMapTextures.constEnd())
    {
        return it.value();
    }

    // The colour map is a horizontal gradient strip.  Mirror what the OpenGL
    // backend does (QOpenGLTexture::Target1D fed with image.flipped()) and take
    // the bottom row of the source image as the single texel row.
    const QImage source(image.flipped().convertToFormat(QImage::Format_RGBA8888));
    const int width = source.width();
    if (width <= 0)
    {
        return nullptr;
    }

    QImage strip(width, 1, QImage::Format_RGBA8888);
    memcpy(strip.bits(), source.constScanLine(0), size_t(width) * 4);

    QRhiTexture *texture = this->rhi->newTexture(QRhiTexture::RGBA8, QSize(width, 1));
    if (!texture->create())
    {
        RLogger::warning("RhiRenderer: failed to create colour map texture for \'%s\'\n", key.toUtf8().constData());
        delete texture;
        return nullptr;
    }
    this->getResourceUpdates()->uploadTexture(texture, strip);

    this->colorMapTextures.insert(key, texture);

    return texture;
}

namespace
{

    //! Append the vertices of one glBegin/glEnd batch to dst, expanding the
    //! primitive into the triangle / line / point list QRhi understands.
    //! Returns the topology the expanded vertices have to be drawn with.
    bool expandBatch(const GLVertexData *src, size_t n, GLenum primitiveType,
                     std::vector<GLVertexData> &dst, RhiBufferData::Topology &topology)
    {
        switch (primitiveType)
        {
            case GL_TRIANGLES:
            {
                topology = RhiBufferData::Triangles;
                for (size_t i = 0; i + 2 < n; i += 3)
                {
                    dst.push_back(src[i]);
                    dst.push_back(src[i+1]);
                    dst.push_back(src[i+2]);
                }
                return true;
            }
            case GL_QUADS:
            {
                topology = RhiBufferData::Triangles;
                for (size_t i = 0; i + 3 < n; i += 4)
                {
                    dst.push_back(src[i]);
                    dst.push_back(src[i+1]);
                    dst.push_back(src[i+2]);
                    dst.push_back(src[i]);
                    dst.push_back(src[i+2]);
                    dst.push_back(src[i+3]);
                }
                return true;
            }
            case GL_QUAD_STRIP:
            {
                topology = RhiBufferData::Triangles;
                for (size_t i = 0; i + 3 < n; i += 2)
                {
                    dst.push_back(src[i]);
                    dst.push_back(src[i+1]);
                    dst.push_back(src[i+2]);
                    dst.push_back(src[i+2]);
                    dst.push_back(src[i+1]);
                    dst.push_back(src[i+3]);
                }
                return true;
            }
            case GL_POLYGON:
            case GL_TRIANGLE_FAN:
            {
                topology = RhiBufferData::Triangles;
                for (size_t i = 1; i + 1 < n; i++)
                {
                    dst.push_back(src[0]);
                    dst.push_back(src[i]);
                    dst.push_back(src[i+1]);
                }
                return true;
            }
            case GL_TRIANGLE_STRIP:
            {
                topology = RhiBufferData::Triangles;
                for (size_t i = 0; i + 2 < n; i++)
                {
                    // Flip the winding of every second triangle so that all
                    // expanded triangles keep the strip's front face.
                    if (i % 2 == 0)
                    {
                        dst.push_back(src[i]);
                        dst.push_back(src[i+1]);
                        dst.push_back(src[i+2]);
                    }
                    else
                    {
                        dst.push_back(src[i+1]);
                        dst.push_back(src[i]);
                        dst.push_back(src[i+2]);
                    }
                }
                return true;
            }
            case GL_LINES:
            {
                topology = RhiBufferData::Lines;
                for (size_t i = 0; i + 1 < n; i += 2)
                {
                    dst.push_back(src[i]);
                    dst.push_back(src[i+1]);
                }
                return true;
            }
            case GL_LINE_STRIP:
            {
                topology = RhiBufferData::Lines;
                for (size_t i = 0; i + 1 < n; i++)
                {
                    dst.push_back(src[i]);
                    dst.push_back(src[i+1]);
                }
                return true;
            }
            case GL_LINE_LOOP:
            {
                topology = RhiBufferData::Lines;
                for (size_t i = 0; i + 1 < n; i++)
                {
                    dst.push_back(src[i]);
                    dst.push_back(src[i+1]);
                }
                if (n > 2)
                {
                    dst.push_back(src[n-1]);
                    dst.push_back(src[0]);
                }
                return true;
            }
            case GL_POINTS:
            {
                // A point size greater than one pixel is an OpenGL and Vulkan
                // only feature; D3D and Metal rasterise every point a single
                // pixel wide.  Each point therefore becomes two triangles which
                // the vertex shader sizes in pixels and turns towards the
                // viewer.  The corner of the quad a vertex stands for is held
                // in its normal, which a point has no other use for.
                topology = RhiBufferData::PointQuads;
                static const float corners[6][2] = {
                    {-1.0f,-1.0f}, { 1.0f,-1.0f}, { 1.0f, 1.0f},
                    {-1.0f,-1.0f}, { 1.0f, 1.0f}, {-1.0f, 1.0f}
                };
                for (size_t i = 0; i < n; i++)
                {
                    for (uint c = 0; c < 6; c++)
                    {
                        GLVertexData vertex = src[i];
                        vertex.normal[0] = corners[c][0];
                        vertex.normal[1] = corners[c][1];
                        vertex.normal[2] = 0.0f;
                        dst.push_back(vertex);
                    }
                }
                return true;
            }
            default:
            {
                RLogger::warning("RhiRenderer: unsupported primitive type 0x%x\n", uint(primitiveType));
                return false;
            }
        }
    }

}

void RhiRenderer::uploadVertexBuffer(const std::vector<GLVertexData> &vertices,
                                     const std::vector<GLVertexBuffer::Batch> &batches,
                                     RhiBufferData &data)
{
    data.batches.clear();

    if (!this->rhi || vertices.empty() || batches.empty())
    {
        return;
    }

    // QRhi only knows triangle / line / point lists, so quads, polygons, fans,
    // strips and loops are expanded into flat lists here.
    std::vector<GLVertexData> expanded;
    expanded.reserve(vertices.size() * 3 / 2);

    // The batch list is handed to the buffer only once the buffer is ready.
    // Growing the buffer destroys it, and destroying it clears the batch list
    // with it, which would leave the geometry uploaded but never drawn.
    std::vector<RhiBufferData::Batch> newBatches;

    for (const GLVertexBuffer::Batch &batch : batches)
    {
        if (batch.count <= 0)
        {
            continue;
        }

        const quint32 first = quint32(expanded.size());
        RhiBufferData::Topology topology = RhiBufferData::Triangles;

        if (!expandBatch(&vertices[size_t(batch.start)], size_t(batch.count), batch.primitiveType, expanded, topology))
        {
            continue;
        }

        const quint32 count = quint32(expanded.size()) - first;
        if (count == 0)
        {
            continue;
        }

        // Merge with the preceding batch whenever the topology and the state
        // driving the pipeline are identical — fewer draw calls per frame.
        if (!newBatches.empty() &&
            newBatches.back().topology == topology &&
            newBatches.back().first + newBatches.back().count == first)
        {
            newBatches.back().count += count;
        }
        else
        {
            newBatches.push_back({topology, first, count});
        }
    }

    if (expanded.empty())
    {
        return;
    }

    const quint32 size = quint32(expanded.size() * sizeof(GLVertexData));

    if (data.buffer && (data.bufferSize < size || data.device != this->rhi))
    {
        RhiRenderer::destroyBuffer(data);
    }
    if (!data.buffer)
    {
        data.buffer = this->rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, size);
        if (!data.buffer->create())
        {
            RLogger::error("RhiRenderer: failed to create vertex buffer (%u bytes)\n", uint(size));
            delete data.buffer;
            data.buffer = nullptr;
            data.batches.clear();
            return;
        }
        data.device = this->rhi;
        data.bufferSize = size;
    }

    this->getResourceUpdates()->uploadStaticBuffer(data.buffer, 0, size, expanded.data());

    data.batches = std::move(newBatches);
}

void RhiRenderer::destroyBuffer(RhiBufferData &data)
{
    if (data.buffer)
    {
        // The device may already be gone (application shutdown), in which case
        // the buffer died with it and touching it would be a use-after-free.
        if (RhiRenderer::liveDevices.contains(data.device))
        {
            data.buffer->deleteLater();
        }
        data.buffer = nullptr;
    }
    data.device = nullptr;
    data.bufferSize = 0;
    data.batches.clear();
}

int RhiRenderer::recordUniformBlock(bool useTexture, bool useLighting, float pointSize, bool pointQuad)
{
    const GLStateCache &stateCache = GLStateCache::instance();

    RhiUniformBlock block;
    memset(&block, 0, sizeof(block));

    const QMatrix4x4 &modelView = RenderMatrixStack::getModelView();
    const QMatrix4x4 mvp = this->clipCorrection * RenderMatrixStack::getProjection() * modelView;

    writeMatrix(block.mvp, mvp);
    writeMatrix(block.modelView, modelView);

    // Normals are transformed by the rotation/scale part of the model-view matrix.
    // Uniform scaling is used throughout, so the transposed inverse is not needed.
    QMatrix4x4 normalMatrix(modelView);
    normalMatrix.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));
    normalMatrix.setRow(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));
    writeMatrix(block.normalMatrix, normalMatrix);

    const double *plane = stateCache.getClipPlane();
    for (int i = 0; i < 4; i++)
    {
        block.clipPlane[i] = float(plane[i]);
    }

    const int nLights = qMin(int(this->lightPositions.size()), RHI_MAX_LIGHTS);
    for (int i = 0; i < nLights; i++)
    {
        const QVector4D &position = this->lightPositions[size_t(i)];
        const QVector4D &ambient  = this->lightAmbients[size_t(i)];
        const QVector4D &diffuse  = this->lightDiffuses[size_t(i)];

        block.lightPosition[i][0] = position.x();
        block.lightPosition[i][1] = position.y();
        block.lightPosition[i][2] = position.z();
        block.lightPosition[i][3] = position.w();

        block.lightAmbient[i][0] = ambient.x();
        block.lightAmbient[i][1] = ambient.y();
        block.lightAmbient[i][2] = ambient.z();
        block.lightAmbient[i][3] = ambient.w();

        block.lightDiffuse[i][0] = diffuse.x();
        block.lightDiffuse[i][1] = diffuse.y();
        block.lightDiffuse[i][2] = diffuse.z();
        block.lightDiffuse[i][3] = diffuse.w();
    }

    block.params[0] = float(nLights);
    block.params[1] = useTexture ? 1.0f : 0.0f;
    block.params[2] = useLighting ? 1.0f : 0.0f;
    // An expanded point has no meaningful facing, so it is always two sided.
    // A back facing quad would otherwise be shaded as the back of a surface.
    block.params[3] = (stateCache.getTwoSided() || pointQuad) ? 1.0f : 0.0f;

    block.params2[0] = stateCache.getClipPlaneEnabled() ? 1.0f : 0.0f;
    block.params2[1] = pointSize;

    // The shader sizes an expanded point in pixels, so it needs the viewport.
    const QRhiViewport vp = this->currentViewport();
    block.params3[0] = pointQuad ? 1.0f : 0.0f;
    block.params3[1] = qMax(1.0f, vp.viewport()[2]);
    block.params3[2] = qMax(1.0f, vp.viewport()[3]);

    this->uniformBlocks.push_back(block);

    return int(this->uniformBlocks.size()) - 1;
}

void RhiRenderer::drawVertexBuffer(const RhiBufferData &data, bool usesTexture)
{
    if (!this->rhi || !data.buffer || data.batches.empty())
    {
        return;
    }

    const GLStateCache &stateCache = GLStateCache::instance();

    for (const RhiBufferData::Batch &batch : data.batches)
    {
        const bool isFilled = (batch.topology == RhiBufferData::Triangles);
        const bool isPointQuad = (batch.topology == RhiBufferData::PointQuads);
        // Lines and points are never lit — this mirrors the OpenGL backend.
        const bool useLighting = isFilled && stateCache.getLighting() == GL_TRUE;
        const bool useTexture = usesTexture && this->activeColorMap != nullptr;

        DrawItem item;
        item.topology    = batch.topology;
        item.buffer      = data.buffer;
        item.firstVertex = batch.first;
        item.vertexCount = batch.count;
        item.uniformSlot = this->recordUniformBlock(useTexture, useLighting, stateCache.getPointSize(), isPointQuad);
        item.texture     = useTexture ? this->activeColorMap : this->whiteTexture;
        item.depthTest   = stateCache.getDepthTest() == GL_TRUE;
        item.depthWrite  = stateCache.getDepthMask() == GL_TRUE;
        item.depthFunc   = toCompareOp(stateCache.getDepthFunc());
        item.cullFace    = isFilled && stateCache.getCullFace() == GL_TRUE;
        item.cullMode    = (stateCache.getCullFaceMode() == GL_FRONT) ? QRhiGraphicsPipeline::Front
                                                                      : QRhiGraphicsPipeline::Back;
        // Push filled primitives back so that edge overlays recorded after the
        // faces win the depth test — same trick as the OpenGL polygon offset.
        item.depthBias   = isFilled;
        item.lineWidth   = stateCache.getLineWidth();
        item.viewport    = this->currentViewport();

        this->drawItems.push_back(item);
    }
}

void RhiRenderer::drawImmediate(const GLVertexData *vertices, size_t count, GLenum glPrimitive)
{
    if (!this->rhi || !vertices || count == 0)
    {
        return;
    }

    const quint32 base = quint32(this->immediateVertices.size());
    RhiBufferData::Topology topology = RhiBufferData::Triangles;

    if (!expandBatch(vertices, count, glPrimitive, this->immediateVertices, topology))
    {
        return;
    }

    const quint32 expandedCount = quint32(this->immediateVertices.size()) - base;
    if (expandedCount == 0)
    {
        return;
    }

    const GLStateCache &stateCache = GLStateCache::instance();
    const bool isFilled = (topology == RhiBufferData::Triangles);
    const bool isPointQuad = (topology == RhiBufferData::PointQuads);
    const bool useLighting = isFilled && stateCache.getLighting() == GL_TRUE;
    const bool useTexture = stateCache.getTexture1D() == GL_TRUE && this->activeColorMap != nullptr;

    DrawItem item;
    item.topology    = topology;
    item.buffer      = nullptr;   // resolved to the streaming buffer at flush time
    item.firstVertex = base;
    item.vertexCount = expandedCount;
    item.uniformSlot = this->recordUniformBlock(useTexture, useLighting, stateCache.getPointSize(), isPointQuad);
    item.texture     = useTexture ? this->activeColorMap : this->whiteTexture;
    item.depthTest   = stateCache.getDepthTest() == GL_TRUE;
    item.depthWrite  = stateCache.getDepthMask() == GL_TRUE;
    item.depthFunc   = toCompareOp(stateCache.getDepthFunc());
    item.cullFace    = isFilled && stateCache.getCullFace() == GL_TRUE;
    item.cullMode    = (stateCache.getCullFaceMode() == GL_FRONT) ? QRhiGraphicsPipeline::Front
                                                                  : QRhiGraphicsPipeline::Back;
    item.depthBias   = isFilled;
    item.lineWidth   = stateCache.getLineWidth();
    item.viewport    = this->currentViewport();

    this->drawItems.push_back(item);
}

QRhiShaderResourceBindings *RhiRenderer::acquireBindings(QRhiTexture *texture)
{
    QRhiTexture *key = texture ? texture : this->whiteTexture;

    QHash<QRhiTexture*,QRhiShaderResourceBindings*>::ConstIterator it = this->bindings.constFind(key);
    if (it != this->bindings.constEnd())
    {
        return it.value();
    }

    QRhiShaderResourceBindings *srb = this->rhi->newShaderResourceBindings();
    srb->setBindings({
        QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(
            0,
            QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
            this->uniformBuffer,
            quint32(sizeof(RhiUniformBlock))),
        QRhiShaderResourceBinding::sampledTexture(
            1,
            QRhiShaderResourceBinding::FragmentStage,
            key,
            this->sampler)
    });
    if (!srb->create())
    {
        RLogger::error("RhiRenderer: failed to create shader resource bindings\n");
        delete srb;
        return nullptr;
    }

    this->bindings.insert(key, srb);

    return srb;
}

QRhiGraphicsPipeline *RhiRenderer::acquirePipeline(const DrawItem &item)
{
    const int lineWidthKey = item.lineWidth > 0.0f ? qBound(1, qRound(item.lineWidth), 63) : 1;

    quint32 bits = 0;
    bits |= quint32(item.topology) & 0x3u;
    bits |= item.depthTest  ? (1u << 2) : 0u;
    bits |= item.depthWrite ? (1u << 3) : 0u;
    bits |= (quint32(item.depthFunc) & 0x7u) << 4;
    bits |= item.cullFace   ? (1u << 7) : 0u;
    bits |= (item.cullMode == QRhiGraphicsPipeline::Front) ? (1u << 8) : 0u;
    bits |= item.depthBias  ? (1u << 9) : 0u;
    bits |= quint32(lineWidthKey) << 10;

    QHash<quint32,QRhiGraphicsPipeline*>::ConstIterator it = this->pipelines.constFind(bits);
    if (it != this->pipelines.constEnd())
    {
        return it.value();
    }

    QRhiShaderResourceBindings *layoutSrb = this->acquireBindings(this->whiteTexture);
    if (!layoutSrb)
    {
        return nullptr;
    }

    QRhiGraphicsPipeline *pipeline = this->rhi->newGraphicsPipeline();

    switch (item.topology)
    {
        case RhiBufferData::Lines:  pipeline->setTopology(QRhiGraphicsPipeline::Lines);     break;
        case RhiBufferData::Points: pipeline->setTopology(QRhiGraphicsPipeline::Points);    break;
        // PointQuads falls through to Triangles - the point state it keeps
        // (unlit, unculled, not depth biased) travels with the draw item.
        default:                    pipeline->setTopology(QRhiGraphicsPipeline::Triangles); break;
    }

    pipeline->setShaderStages({
        { QRhiShaderStage::Vertex,   this->vertexShader   },
        { QRhiShaderStage::Fragment, this->fragmentShader }
    });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ QRhiVertexInputBinding(sizeof(GLVertexData)) });
    inputLayout.setAttributes({
        QRhiVertexInputAttribute(0, kPositionLocation, QRhiVertexInputAttribute::Float3,     quint32(offsetof(GLVertexData, position))),
        QRhiVertexInputAttribute(0, kNormalLocation,   QRhiVertexInputAttribute::Float3,     quint32(offsetof(GLVertexData, normal))),
        QRhiVertexInputAttribute(0, kTexCoordLocation, QRhiVertexInputAttribute::Float,      quint32(offsetof(GLVertexData, texCoord))),
        QRhiVertexInputAttribute(0, kColorLocation,    QRhiVertexInputAttribute::UNormByte4, quint32(offsetof(GLVertexData, color)))
    });
    pipeline->setVertexInputLayout(inputLayout);

    pipeline->setShaderResourceBindings(layoutSrb);
    pipeline->setRenderPassDescriptor(this->renderPassDescriptor);
    pipeline->setSampleCount(this->sampleCount);

    pipeline->setDepthTest(item.depthTest);
    pipeline->setDepthWrite(item.depthWrite);
    pipeline->setDepthOp(item.depthFunc);

    pipeline->setCullMode(item.cullFace ? item.cullMode : QRhiGraphicsPipeline::None);
    pipeline->setFrontFace(QRhiGraphicsPipeline::CCW);

    if (item.depthBias)
    {
        pipeline->setDepthBias(1);
        pipeline->setSlopeScaledDepthBias(1.0f);
    }

    if (item.topology == RhiBufferData::Lines && this->rhi->isFeatureSupported(QRhi::WideLines))
    {
        pipeline->setLineWidth(float(lineWidthKey));
    }

    QRhiGraphicsPipeline::TargetBlend blend;
    blend.enable = true;
    blend.srcColor = QRhiGraphicsPipeline::SrcAlpha;
    blend.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    blend.srcAlpha = QRhiGraphicsPipeline::One;
    blend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    pipeline->setTargetBlends({ blend });

    if (!pipeline->create())
    {
        RLogger::error("RhiRenderer: failed to create graphics pipeline (key 0x%x)\n", uint(bits));
        delete pipeline;
        return nullptr;
    }

    this->pipelines.insert(bits, pipeline);

    return pipeline;
}

void RhiRenderer::flush(QRhiCommandBuffer *cb, QRhiRenderTarget *renderTarget, const QColor &clearColor)
{
    if (!this->rhi || !cb || !renderTarget)
    {
        return;
    }

    QRhiResourceUpdateBatch *updates = this->getResourceUpdates();

    // --- Streaming buffer for immediate mode geometry -----------------------
    if (!this->immediateVertices.empty())
    {
        const quint32 size = quint32(this->immediateVertices.size() * sizeof(GLVertexData));
        if (this->immediateBuffer && this->immediateBufferSize < size)
        {
            this->immediateBuffer->deleteLater();
            this->immediateBuffer = nullptr;
        }
        if (!this->immediateBuffer)
        {
            // Over-allocate so that small frame-to-frame growth does not reallocate.
            this->immediateBufferSize = size + size / 2 + 4096;
            this->immediateBuffer = this->rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, this->immediateBufferSize);
            if (!this->immediateBuffer->create())
            {
                RLogger::error("RhiRenderer: failed to create streaming vertex buffer\n");
                delete this->immediateBuffer;
                this->immediateBuffer = nullptr;
                this->immediateBufferSize = 0;
            }
        }
        if (this->immediateBuffer)
        {
            updates->uploadStaticBuffer(this->immediateBuffer, 0, size, this->immediateVertices.data());
        }
    }

    // --- Uniform buffer -----------------------------------------------------
    if (this->uniformSlotSize == 0)
    {
        const quint32 alignment = quint32(this->rhi->ubufAlignment());
        this->uniformSlotSize = quint32(sizeof(RhiUniformBlock));
        if (alignment > 0)
        {
            this->uniformSlotSize = ((this->uniformSlotSize + alignment - 1) / alignment) * alignment;
        }
    }

    const int requiredSlots = qMax(int(this->uniformBlocks.size()), 1);
    if (!this->uniformBuffer || this->uniformSlotCapacity < requiredSlots)
    {
        int capacity = qMax(this->uniformSlotCapacity, kInitialUniformSlots);
        while (capacity < requiredSlots)
        {
            capacity *= 2;
        }

        // Every shader resource binding references the uniform buffer, so they
        // all have to be rebuilt once the buffer is replaced. Each pipeline was
        // created against one of those bindings and keeps it for its layout, so
        // the pipelines go with them - a cached pipeline left pointing at a
        // deleted binding stops drawing what it was built for.
        for (QRhiGraphicsPipeline *pipeline : std::as_const(this->pipelines))
        {
            delete pipeline;
        }
        this->pipelines.clear();

        for (QRhiShaderResourceBindings *srb : std::as_const(this->bindings))
        {
            delete srb;
        }
        this->bindings.clear();

        if (this->uniformBuffer)
        {
            this->uniformBuffer->deleteLater();
            this->uniformBuffer = nullptr;
        }

        this->uniformBuffer = this->rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer,
                                                   this->uniformSlotSize * quint32(capacity));
        if (!this->uniformBuffer->create())
        {
            RLogger::error("RhiRenderer: failed to create uniform buffer for %d slots\n", capacity);
            delete this->uniformBuffer;
            this->uniformBuffer = nullptr;
            this->uniformSlotCapacity = 0;
            this->resourceUpdates = nullptr;
            return;
        }
        this->uniformSlotCapacity = capacity;
    }

    for (size_t i = 0; i < this->uniformBlocks.size(); i++)
    {
        updates->updateDynamicBuffer(this->uniformBuffer,
                                     this->uniformSlotSize * quint32(i),
                                     quint32(sizeof(RhiUniformBlock)),
                                     &this->uniformBlocks[i]);
    }

    // --- Single render pass -------------------------------------------------
    cb->beginPass(renderTarget,
                  clearColor,
                  QRhiDepthStencilClearValue(1.0f, 0),
                  updates);
    this->resourceUpdates = nullptr;

    QRhiGraphicsPipeline *boundPipeline = nullptr;
    QRhiBuffer *boundBuffer = nullptr;
    QRhiViewport boundViewport;
    bool viewportSet = false;

    for (const DrawItem &item : this->drawItems)
    {
        QRhiBuffer *vertexBuffer = item.buffer ? item.buffer : this->immediateBuffer;
        if (!vertexBuffer || item.vertexCount == 0)
        {
            continue;
        }

        QRhiGraphicsPipeline *pipeline = this->acquirePipeline(item);
        if (!pipeline)
        {
            continue;
        }
        QRhiShaderResourceBindings *srb = this->acquireBindings(item.texture);
        if (!srb)
        {
            continue;
        }

        if (pipeline != boundPipeline)
        {
            cb->setGraphicsPipeline(pipeline);
            boundPipeline = pipeline;
            // A new pipeline resets the viewport binding on some backends.
            viewportSet = false;
        }

        if (!viewportSet ||
            boundViewport.viewport()[0] != item.viewport.viewport()[0] ||
            boundViewport.viewport()[1] != item.viewport.viewport()[1] ||
            boundViewport.viewport()[2] != item.viewport.viewport()[2] ||
            boundViewport.viewport()[3] != item.viewport.viewport()[3] ||
            boundViewport.minDepth() != item.viewport.minDepth() ||
            boundViewport.maxDepth() != item.viewport.maxDepth())
        {
            cb->setViewport(item.viewport);
            boundViewport = item.viewport;
            viewportSet = true;
        }

        const QRhiCommandBuffer::DynamicOffset dynamicOffset(0, this->uniformSlotSize * quint32(item.uniformSlot));
        cb->setShaderResources(srb, 1, &dynamicOffset);

        if (vertexBuffer != boundBuffer)
        {
            const QRhiCommandBuffer::VertexInput vertexInput(vertexBuffer, 0);
            cb->setVertexInput(0, 1, &vertexInput);
            boundBuffer = vertexBuffer;
        }

        cb->draw(item.vertexCount, 1, item.firstVertex);
    }

    cb->endPass();

    int nTriangleItems = 0;
    int nLineItems = 0;
    int nPointItems = 0;
    for (const DrawItem &item : this->drawItems)
    {
        switch (item.topology)
        {
            case RhiBufferData::Lines:      nLineItems++;  break;
            case RhiBufferData::Points:
            case RhiBufferData::PointQuads: nPointItems++; break;
            default:                        nTriangleItems++; break;
        }
    }

    RLogger::trace("RhiRenderer: submitted %d draw items (%d triangle, %d line, %d point), %d uniform slots, %d streamed vertices\n",
                   int(this->drawItems.size()),
                   nTriangleItems,
                   nLineItems,
                   nPointItems,
                   int(this->uniformBlocks.size()),
                   int(this->immediateVertices.size()));

    this->drawItems.clear();
    this->uniformBlocks.clear();
    this->immediateVertices.clear();
}
