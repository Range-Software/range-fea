#include "gl_state_cache.h"
#include "gl_functions.h"
#include "gl_shader_program.h"
#include "gl_vertex_buffer.h"
#include "render_backend.h"

//! Issue a GL call only when the legacy OpenGL backend is active.
//! With the QRhi backend the cached value alone drives pipeline selection.
#define GL_STATE_CALL(__call) { if (RenderBackend::isOpenGL()) { GL_SAFE_CALL(__call); } }

GLStateCache::GLStateCache()
    : lineSmooth(GL_FALSE)
    , lighting(GL_TRUE)
    , normalize(GL_TRUE)
    , depthTest(GL_TRUE)
    , depthMask(GL_TRUE)
    , blend(GL_FALSE)
    , texture1D(GL_FALSE)
    , texture2D(GL_FALSE)
    , cullFace(GL_FALSE)
    , lineStipple(GL_FALSE)
    , polygonOffsetFill(GL_FALSE)
    , cullFaceMode(GL_BACK)
    , depthFunc(GL_LEQUAL)
    , pointSize(1.0f)
    , lineWidth(1.0f)
    , polygonOffsetFactor(0.0f)
    , polygonOffsetUnits(0.0f)
    , twoSided(false)
    , clipPlaneEnabled(false)
    , initialized(false)
    , shaderProgram(nullptr)
{
    this->clipPlane[0] = 0.0;
    this->clipPlane[1] = 0.0;
    this->clipPlane[2] = 0.0;
    this->clipPlane[3] = 0.0;
}

GLStateCache &GLStateCache::instance()
{
    static GLStateCache cache;
    return cache;
}

void GLStateCache::initialize()
{
    // Write-through cache: set known default state without GPU queries.
    // GPU state is set in GLWidget::paintGL() at frame start, so we just
    // mirror those values here. This avoids expensive CPU-GPU sync points.
    this->lineSmooth = GL_FALSE;
    this->lighting = GL_TRUE;
    this->normalize = GL_FALSE;
    this->depthTest = GL_TRUE;
    this->depthMask = GL_TRUE;
    this->blend = GL_TRUE;
    this->texture1D = GL_FALSE;
    this->texture2D = GL_FALSE;
    this->cullFace = GL_FALSE;
    this->lineStipple = GL_FALSE;
    this->polygonOffsetFill = GL_FALSE;
    this->cullFaceMode = GL_BACK;
    this->depthFunc = GL_LEQUAL;
    this->pointSize = 10.0f;
    this->lineWidth = 1.0f;
    this->polygonOffsetFactor = 0.0f;
    this->polygonOffsetUnits = 0.0f;
    this->twoSided = false;
    this->initialized = true;
}

void GLStateCache::reset()
{
    this->initialized = false;
}

bool GLStateCache::isInitialized() const
{
    return this->initialized;
}

// Boolean state getters
GLboolean GLStateCache::getLineSmooth() const { return this->lineSmooth; }
GLboolean GLStateCache::getLighting() const { return this->lighting; }
GLboolean GLStateCache::getNormalize() const { return this->normalize; }
GLboolean GLStateCache::getDepthTest() const { return this->depthTest; }
GLboolean GLStateCache::getDepthMask() const { return this->depthMask; }
GLboolean GLStateCache::getBlend() const { return this->blend; }
GLboolean GLStateCache::getTexture1D() const { return this->texture1D; }
GLboolean GLStateCache::getTexture2D() const { return this->texture2D; }
GLboolean GLStateCache::getCullFace() const { return this->cullFace; }
GLboolean GLStateCache::getLineStipple() const { return this->lineStipple; }
GLboolean GLStateCache::getPolygonOffsetFill() const { return this->polygonOffsetFill; }

// Integer state getters
GLenum GLStateCache::getCullFaceMode() const { return this->cullFaceMode; }
GLenum GLStateCache::getDepthFunc() const { return this->depthFunc; }

// Float state getters
GLfloat GLStateCache::getPointSize() const { return this->pointSize; }
GLfloat GLStateCache::getLineWidth() const { return this->lineWidth; }
GLfloat GLStateCache::getPolygonOffsetFactor() const { return this->polygonOffsetFactor; }
GLfloat GLStateCache::getPolygonOffsetUnits() const { return this->polygonOffsetUnits; }

bool GLStateCache::getTwoSided() const { return this->twoSided; }
bool GLStateCache::getClipPlaneEnabled() const { return this->clipPlaneEnabled; }
const double *GLStateCache::getClipPlane() const { return this->clipPlane; }

// State setters - only make GL call if state actually changes
void GLStateCache::setLineSmooth(GLboolean enabled)
{
    if (this->lineSmooth != enabled)
    {
        this->lineSmooth = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH));
    }
}

void GLStateCache::setLighting(GLboolean enabled)
{
    if (this->lighting != enabled)
    {
        this->lighting = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
    }
    // Keep the shader uniform in sync (mirrors setTexture1D() pattern).
    // Guard against VBO recording: per-element enable/disable should not corrupt
    // the per-draw-call uniform.
    if (this->shaderProgram && !GLFunctions::isRecordingVBO())
    {
        this->shaderProgram->setUniformBool("uUseLighting", enabled == GL_TRUE);
    }
}

void GLStateCache::setShaderProgram(GLShaderProgram *prog)
{
    this->shaderProgram = prog;
}

GLShaderProgram *GLStateCache::getShaderProgram() const
{
    return this->shaderProgram;
}

void GLStateCache::setNormalize(GLboolean enabled)
{
    if (this->normalize != enabled)
    {
        this->normalize = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_NORMALIZE) : glDisable(GL_NORMALIZE));
    }
}

void GLStateCache::setDepthTest(GLboolean enabled)
{
    if (this->depthTest != enabled)
    {
        this->depthTest = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
    }
}

void GLStateCache::setDepthMask(GLboolean enabled)
{
    if (this->depthMask != enabled)
    {
        this->depthMask = enabled;
        GL_STATE_CALL(glDepthMask(enabled));
    }
}

void GLStateCache::setBlend(GLboolean enabled)
{
    if (this->blend != enabled)
    {
        this->blend = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND));
    }
}

void GLStateCache::setTexture1D(GLboolean enabled)
{
    if (this->texture1D != enabled)
    {
        this->texture1D = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_TEXTURE_1D) : glDisable(GL_TEXTURE_1D));
    }
    if (GLFunctions::isRecordingVBO())
    {
        // During recording: mark the current VBO so callList() can set uUseTexture
        // correctly at render time.  The uniform is per-draw-call and cannot be set
        // mid-recording, so we defer it to render time via the usesTexture flag.
        if (enabled == GL_TRUE && GLFunctions::getCurrentVBO())
        {
            GLFunctions::getCurrentVBO()->setUsesTexture(true);
        }
    }
    else if (this->shaderProgram)
    {
        // Not recording: update the shader uniform directly.
        this->shaderProgram->setUniformBool("uUseTexture", enabled == GL_TRUE);
    }
}

void GLStateCache::setTexture2D(GLboolean enabled)
{
    if (this->texture2D != enabled)
    {
        this->texture2D = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D));
    }
}

void GLStateCache::setCullFace(GLboolean enabled)
{
    if (this->cullFace != enabled)
    {
        this->cullFace = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));
    }
}

void GLStateCache::setCullFaceMode(GLenum mode)
{
    if (this->cullFaceMode != mode)
    {
        this->cullFaceMode = mode;
        GL_STATE_CALL(glCullFace(mode));
    }
}

void GLStateCache::setDepthFunc(GLenum func)
{
    if (this->depthFunc != func)
    {
        this->depthFunc = func;
        GL_STATE_CALL(glDepthFunc(func));
    }
}

void GLStateCache::setLineStipple(GLboolean enabled, GLint factor, GLushort pattern)
{
    // Line stipple has no QRhi equivalent — the pattern is simply ignored there.
    if (RenderBackend::isOpenGL())
    {
        if (enabled)
        {
            GL_SAFE_CALL(glLineStipple(factor, pattern));
            GL_SAFE_CALL(glEnable(GL_LINE_STIPPLE));
        }
        else
        {
            GL_SAFE_CALL(glDisable(GL_LINE_STIPPLE));
        }
    }
    this->lineStipple = enabled;
}

void GLStateCache::setPolygonOffsetFill(GLboolean enabled)
{
    if (this->polygonOffsetFill != enabled)
    {
        this->polygonOffsetFill = enabled;
        GL_STATE_CALL(enabled ? glEnable(GL_POLYGON_OFFSET_FILL) : glDisable(GL_POLYGON_OFFSET_FILL));
    }
}

void GLStateCache::setPolygonOffset(GLfloat factor, GLfloat units)
{
    if (this->polygonOffsetFactor != factor || this->polygonOffsetUnits != units)
    {
        this->polygonOffsetFactor = factor;
        this->polygonOffsetUnits = units;
        GL_STATE_CALL(glPolygonOffset(factor, units));
    }
}

void GLStateCache::setPointSize(GLfloat size)
{
    if (this->pointSize != size)
    {
        this->pointSize = size;
        GL_STATE_CALL(glPointSize(size));
    }
}

void GLStateCache::setLineWidth(GLfloat width)
{
    if (this->lineWidth != width)
    {
        this->lineWidth = width;
        GL_STATE_CALL(glLineWidth(width));
    }
}

void GLStateCache::setTwoSided(bool twoSided)
{
    this->twoSided = twoSided;
    if (this->shaderProgram)
    {
        this->shaderProgram->setUniformBool("uTwoSided", twoSided);
    }
}

void GLStateCache::setClipPlane(bool enabled, const double plane[4])
{
    this->clipPlaneEnabled = enabled;
    if (plane)
    {
        for (int i = 0; i < 4; i++)
        {
            this->clipPlane[i] = plane[i];
        }
    }
    if (RenderBackend::isOpenGL())
    {
        if (plane)
        {
            // glClipPlane transforms the equation by the inverse transposed
            // model-view, so it has to be uploaded while that matrix is identity.
            GL_SAFE_CALL(glClipPlane(GL_CLIP_PLANE0, this->clipPlane));
        }
        GL_SAFE_CALL(enabled ? glEnable(GL_CLIP_PLANE0) : glDisable(GL_CLIP_PLANE0));
    }
}

// Convenience methods
void GLStateCache::enableLineSmooth() { this->setLineSmooth(GL_TRUE); }
void GLStateCache::disableLineSmooth() { this->setLineSmooth(GL_FALSE); }
void GLStateCache::enableLighting() { this->setLighting(GL_TRUE); }
void GLStateCache::disableLighting() { this->setLighting(GL_FALSE); }
void GLStateCache::enableNormalize() { this->setNormalize(GL_TRUE); }
void GLStateCache::disableNormalize() { this->setNormalize(GL_FALSE); }
void GLStateCache::enableDepthTest() { this->setDepthTest(GL_TRUE); }
void GLStateCache::disableDepthTest() { this->setDepthTest(GL_FALSE); }
void GLStateCache::enableBlend() { this->setBlend(GL_TRUE); }
void GLStateCache::disableBlend() { this->setBlend(GL_FALSE); }
void GLStateCache::enableTexture1D() { this->setTexture1D(GL_TRUE); }
void GLStateCache::disableTexture1D() { this->setTexture1D(GL_FALSE); }
void GLStateCache::enableTexture2D() { this->setTexture2D(GL_TRUE); }
void GLStateCache::disableTexture2D() { this->setTexture2D(GL_FALSE); }
void GLStateCache::enableCullFace() { this->setCullFace(GL_TRUE); }
void GLStateCache::disableCullFace() { this->setCullFace(GL_FALSE); }
