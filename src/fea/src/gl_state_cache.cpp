#include "gl_state_cache.h"
#include "gl_functions.h"

GLStateCache::GLStateCache()
    : lineSmooth(GL_FALSE)
    , lighting(GL_TRUE)
    , normalize(GL_TRUE)
    , depthTest(GL_TRUE)
    , blend(GL_FALSE)
    , texture1D(GL_FALSE)
    , texture2D(GL_FALSE)
    , cullFace(GL_FALSE)
    , cullFaceMode(GL_BACK)
    , pointSize(1.0f)
    , lineWidth(1.0f)
    , initialized(false)
{
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
    this->blend = GL_TRUE;
    this->texture1D = GL_FALSE;
    this->texture2D = GL_FALSE;
    this->cullFace = GL_FALSE;
    this->cullFaceMode = GL_BACK;
    this->pointSize = 10.0f;
    this->lineWidth = 1.0f;
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
GLboolean GLStateCache::getBlend() const { return this->blend; }
GLboolean GLStateCache::getTexture1D() const { return this->texture1D; }
GLboolean GLStateCache::getTexture2D() const { return this->texture2D; }
GLboolean GLStateCache::getCullFace() const { return this->cullFace; }

// Integer state getters
GLenum GLStateCache::getCullFaceMode() const { return this->cullFaceMode; }

// Float state getters
GLfloat GLStateCache::getPointSize() const { return this->pointSize; }
GLfloat GLStateCache::getLineWidth() const { return this->lineWidth; }

// State setters - only make GL call if state actually changes
void GLStateCache::setLineSmooth(GLboolean enabled)
{
    if (this->lineSmooth != enabled)
    {
        this->lineSmooth = enabled;
        GL_SAFE_CALL(enabled ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH));
    }
}

void GLStateCache::setLighting(GLboolean enabled)
{
    if (this->lighting != enabled)
    {
        this->lighting = enabled;
        GL_SAFE_CALL(enabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
    }
}

void GLStateCache::setNormalize(GLboolean enabled)
{
    if (this->normalize != enabled)
    {
        this->normalize = enabled;
        GL_SAFE_CALL(enabled ? glEnable(GL_NORMALIZE) : glDisable(GL_NORMALIZE));
    }
}

void GLStateCache::setDepthTest(GLboolean enabled)
{
    if (this->depthTest != enabled)
    {
        this->depthTest = enabled;
        GL_SAFE_CALL(enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
    }
}

void GLStateCache::setBlend(GLboolean enabled)
{
    if (this->blend != enabled)
    {
        this->blend = enabled;
        GL_SAFE_CALL(enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND));
    }
}

void GLStateCache::setTexture1D(GLboolean enabled)
{
    if (this->texture1D != enabled)
    {
        this->texture1D = enabled;
        GL_SAFE_CALL(enabled ? glEnable(GL_TEXTURE_1D) : glDisable(GL_TEXTURE_1D));
    }
}

void GLStateCache::setTexture2D(GLboolean enabled)
{
    if (this->texture2D != enabled)
    {
        this->texture2D = enabled;
        GL_SAFE_CALL(enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D));
    }
}

void GLStateCache::setCullFace(GLboolean enabled)
{
    if (this->cullFace != enabled)
    {
        this->cullFace = enabled;
        GL_SAFE_CALL(enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));
    }
}

void GLStateCache::setCullFaceMode(GLenum mode)
{
    if (this->cullFaceMode != mode)
    {
        this->cullFaceMode = mode;
        GL_SAFE_CALL(glCullFace(mode));
    }
}

void GLStateCache::setPointSize(GLfloat size)
{
    if (this->pointSize != size)
    {
        this->pointSize = size;
        GL_SAFE_CALL(glPointSize(size));
    }
}

void GLStateCache::setLineWidth(GLfloat width)
{
    if (this->lineWidth != width)
    {
        this->lineWidth = width;
        GL_SAFE_CALL(glLineWidth(width));
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
