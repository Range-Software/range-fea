#include "gl_functions.h"
#include "gl_point.h"
#include "gl_state_cache.h"

void GLPoint::_init(const GLPoint *pGlPoint)
{
    if (pGlPoint)
    {
        this->position = pGlPoint->position;
        this->pointSize = pGlPoint->pointSize;
    }
}

GLPoint::GLPoint(GLWidget *glWidget, const RR3Vector &position, float pointSize)
    : GLObject(glWidget)
    , position(position)
    , pointSize(pointSize)
{
    this->_init();
}

GLPoint::GLPoint(const GLPoint &glPoint)
    : GLObject(glPoint)
{
    this->_init(&glPoint);
}

GLPoint::~GLPoint()
{
}

GLPoint &GLPoint::operator =(const GLPoint &glPoint)
{
    this->GLObject::operator =(glPoint);
    this->_init(&glPoint);
    return (*this);
}

void GLPoint::initialize()
{
    // Save current settings
    GL_SAFE_CALL(glGetFloatv(GL_POINT_SIZE, &this->prevPointSize));
    this->lightingEnabled = GLStateCache::instance().getLighting();
    // Initialize environment
    GL_SAFE_CALL(glPointSize(this->pointSize));
    GLStateCache::instance().disableLighting();
}

void GLPoint::finalize()
{
    GL_SAFE_CALL(glPointSize(this->prevPointSize));

    GLStateCache::instance().setLighting(this->lightingEnabled);
}

void GLPoint::draw()
{
    GLFunctions::begin(GL_POINTS);
    GL_SAFE_CALL(glVertex3d(this->position[0],this->position[1],this->position[2]));
    GLFunctions::end();
}
