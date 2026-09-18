#ifndef RHI_RENDERER_H
#define RHI_RENDERER_H

#include <vector>

#include <QColor>
#include <QHash>
#include <QSet>
#include <QImage>
#include <QMatrix4x4>
#include <QVector4D>

#include <rhi/qrhi.h>

#include "gl_vertex_buffer.h"

//! Maximum number of lights supported by the QRhi shaders.
#define RHI_MAX_LIGHTS 8

//! Uniform block shared by the vertex and fragment stage (std140 layout).
//! Must stay in sync with src/shaders/rhi/main.vert and src/shaders/rhi/main.frag.
struct RhiUniformBlock
{
    float mvp[16];                            //!< clipSpaceCorr * projection * modelView
    float modelView[16];                      //!< model-view matrix (eye space)
    float normalMatrix[16];                   //!< model-view rotation part for normals
    float clipPlane[4];                       //!< clipping plane in eye space
    float lightPosition[RHI_MAX_LIGHTS][4];   //!< light positions (eye space)
    float lightAmbient[RHI_MAX_LIGHTS][4];    //!< light ambient colours
    float lightDiffuse[RHI_MAX_LIGHTS][4];    //!< light diffuse colours
    float params[4];                          //!< x=nLights, y=useTexture, z=useLighting, w=twoSided
    float params2[4];                         //!< x=clipEnabled, y=pointSize, z=highlight, w reserved
    float params3[4];                         //!< x=pointQuad, y/z=viewport size (px), w reserved
};

//! Per-GLVertexBuffer QRhi state.  Kept out of GLVertexBuffer itself so that the
//! OpenGL-only translation units do not need to see the QRhi headers.
struct RhiBufferData
{
    //! Primitive topology of a converted batch.
    enum Topology
    {
        Triangles = 0,
        Lines,
        Points,
        //! Points expanded into screen facing quads.  Drawn as a triangle
        //! list, but state wise a point: unlit, unculled and not offset.
        PointQuads
    };

    //! Converted primitive batch, ready for a single draw call.
    struct Batch
    {
        Topology topology;
        quint32  first;
        quint32  count;
    };

    QRhiBuffer         *buffer = nullptr;
    //! Device the buffer belongs to — checked before releasing it.
    QRhi               *device = nullptr;
    quint32             bufferSize = 0;
    std::vector<Batch>  batches;
};

//! QRhi based renderer.
//!
//! The scene is drawn in two phases.  During the record phase the existing
//! draw traversal (GLWidget::drawModel() and everything it calls) emits draw
//! items and uniform blocks into CPU side containers; vertex data is uploaded
//! through a single resource update batch.  During the flush phase a single
//! render pass is opened and all recorded items are submitted.
//!
//! Two phases are needed because QRhi only accepts resource updates outside of
//! a render pass, while the traversal creates and fills vertex buffers lazily.
class RhiRenderer
{

    public:

        //! One recorded draw call.
        struct DrawItem
        {
            RhiBufferData::Topology topology;
            QRhiBuffer             *buffer;
            quint32                 firstVertex;
            quint32                 vertexCount;
            int                     uniformSlot;
            QRhiTexture            *texture;
            bool                    depthTest;
            bool                    depthWrite;
            QRhiGraphicsPipeline::CompareOp depthFunc;
            bool                    cullFace;
            QRhiGraphicsPipeline::CullMode cullMode;
            bool                    depthBias;
            float                   lineWidth;
            QRhiViewport            viewport;
        };

    private:

        //! Renderer that is currently recording a frame.
        static RhiRenderer *currentRenderer;
        //! Devices with live resources, used to make buffer release safe.
        static QSet<QRhi*> liveDevices;

        //! QRhi instance owned by the widget (not by this class).
        QRhi *rhi;
        //! Sample count of the render target the pipelines are built for.
        int sampleCount;
        //! Render pass descriptor the pipelines are built for.
        QRhiRenderPassDescriptor *renderPassDescriptor;

        //! Compiled shader stages.
        QShader vertexShader;
        QShader fragmentShader;

        //! Uniform buffer holding one RhiUniformBlock per draw item.
        QRhiBuffer *uniformBuffer;
        //! Aligned size of a single uniform slot.
        quint32 uniformSlotSize;
        //! Number of slots the uniform buffer can currently hold.
        int uniformSlotCapacity;

        //! Streaming vertex buffer for immediate mode geometry.
        QRhiBuffer *immediateBuffer;
        //! Capacity of the streaming vertex buffer in bytes.
        quint32 immediateBufferSize;
        //! CPU side immediate mode vertices recorded during the current frame.
        std::vector<GLVertexData> immediateVertices;

        //! Linear sampler used for the colour map.
        QRhiSampler *sampler;
        //! Fallback 1x1 white texture used when no colour map is bound.
        QRhiTexture *whiteTexture;
        //! Colour map textures keyed by the file they were built from.
        QHash<QString,QRhiTexture*> colorMapTextures;
        //! Colour map texture currently selected by GLTexture::load().
        QRhiTexture *activeColorMap;

        //! Graphics pipelines keyed by their state.
        QHash<quint32,QRhiGraphicsPipeline*> pipelines;
        //! Shader resource bindings keyed by the bound texture.
        QHash<QRhiTexture*,QRhiShaderResourceBindings*> bindings;

        //! Resource update batch collecting all uploads of the current frame.
        QRhiResourceUpdateBatch *resourceUpdates;

        //! Draw items recorded during the current frame.
        std::vector<DrawItem> drawItems;
        //! Uniform blocks recorded during the current frame.
        std::vector<RhiUniformBlock> uniformBlocks;

        //! Lights uploaded with every draw item of the current frame.
        std::vector<QVector4D> lightPositions;
        std::vector<QVector4D> lightAmbients;
        std::vector<QVector4D> lightDiffuses;

        //! Render target size in device pixels.
        QSize outputSize;
        //! Depth range applied to recorded draw items.
        float minDepth;
        float maxDepth;
        //! Correction matrix mapping OpenGL clip space to the active backend.
        QMatrix4x4 clipCorrection;

    public:

        //! Constructor.
        RhiRenderer();

        //! Destructor.
        ~RhiRenderer();

        //! Return the renderer recording the current frame (nullptr outside a frame).
        static RhiRenderer *current();

        //! Set the renderer recording the current frame.
        static void setCurrent(RhiRenderer *renderer);

        //! Create the device level resources.  Returns false when the shaders cannot be loaded.
        bool initialize(QRhi *rhi, QRhiRenderPassDescriptor *rpDesc, int sampleCount);

        //! Destroy all device level resources.
        void releaseResources();

        //! Return the QRhi instance the renderer was initialized with.
        QRhi *getRhi() const;

        //! Return true if the renderer holds usable device resources.
        bool isValid() const;

        //! Start recording a frame.
        void beginFrame(const QSize &outputSize);

        //! Submit everything recorded since beginFrame() and finish the frame.
        void flush(QRhiCommandBuffer *cb, QRhiRenderTarget *renderTarget, const QColor &clearColor);

        //! Return the resource update batch of the current frame, creating it on demand.
        QRhiResourceUpdateBatch *getResourceUpdates();

        //! Replace the light set used for subsequent draw items.
        void setLights(const std::vector<QVector4D> &positions,
                       const std::vector<QVector4D> &ambients,
                       const std::vector<QVector4D> &diffuses);

        //! Restrict subsequent draw items to the given depth range.
        //! Used instead of a mid-frame depth buffer clear, which QRhi does not offer.
        void setDepthRange(float minDepth, float maxDepth);

        //! Select the colour map built from the given image for subsequent draw items.
        //! key identifies the image (its file name) and drives the texture cache.
        void setColorMap(const QString &key, const QImage &image);

        //! Deselect the colour map.
        void clearColorMap();

        //! Upload a recorded vertex buffer and convert its batches to QRhi topologies.
        void uploadVertexBuffer(const std::vector<GLVertexData> &vertices,
                                const std::vector<GLVertexBuffer::Batch> &batches,
                                RhiBufferData &data);

        //! Record draw items for a previously uploaded vertex buffer.
        void drawVertexBuffer(const RhiBufferData &data, bool usesTexture);

        //! Record a draw item for immediate mode geometry.
        void drawImmediate(const GLVertexData *vertices, size_t count, GLenum glPrimitive);

        //! Schedule deletion of a buffer created by this renderer.
        static void destroyBuffer(RhiBufferData &data);

    private:

        //! Fill a uniform block from the current matrix stack and state cache.
        int recordUniformBlock(bool useTexture, bool useLighting, float pointSize, bool pointQuad);

        //! Return (creating if needed) the pipeline matching the draw item state.
        QRhiGraphicsPipeline *acquirePipeline(const DrawItem &item);

        //! Return (creating if needed) the shader resource bindings for a texture.
        QRhiShaderResourceBindings *acquireBindings(QRhiTexture *texture);

        //! Return the viewport to be used for recorded draw items.
        QRhiViewport currentViewport() const;

        //! Convert a colour map image to an Nx1 RGBA texture.
        QRhiTexture *acquireColorMapTexture(const QString &key, const QImage &image);

        //! Load the baked shader stages from the Qt resource system.
        bool loadShaders();

};

#endif // RHI_RENDERER_H
