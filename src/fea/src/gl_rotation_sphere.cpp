#include "gl_functions.h"
#include "gl_rotation_sphere.h"
#include "gl_state_cache.h"


void GLRotationSphere::_init(const GLRotationSphere *pGlRotationSphere)
{
    if (pGlRotationSphere)
    {
        this->position = pGlRotationSphere->position;
        this->scale = pGlRotationSphere->scale;
    }
}

GLRotationSphere::GLRotationSphere(GLWidget *glWidget, const RR3Vector &position, double scale)
    : GLObject(glWidget)
    , position(position)
    , scale(scale)
{
    this->_init();
}

GLRotationSphere::GLRotationSphere(const GLRotationSphere &glRotationSphere)
    : GLObject(glRotationSphere)
{
    this->_init(&glRotationSphere);
}

GLRotationSphere::~GLRotationSphere()
{

}

GLRotationSphere &GLRotationSphere::operator =(const GLRotationSphere &glRotationSphere)
{
    this->GLObject::operator =(glRotationSphere);
    this->_init(&glRotationSphere);
    return (*this);
}

void GLRotationSphere::initialize()
{
    // Save current settings
    this->depthTestEnabled = GLStateCache::instance().getDepthTest();
    this->lineSmoothEnabled = GLStateCache::instance().getLineSmooth();
    this->normalizeEnabled = GLStateCache::instance().getNormalize();
    this->lightingEnabled = GLStateCache::instance().getLighting();
    this->lineWidth = GLStateCache::instance().getLineWidth();
    this->cullFaceEnabled = GLStateCache::instance().getCullFace();
    // Initialize environment
    GLStateCache::instance().setDepthTest(GL_TRUE);
    GLStateCache::instance().setLineSmooth(GL_TRUE);
    GLStateCache::instance().setNormalize(GL_TRUE);
    GLStateCache::instance().disableLighting();
    GLStateCache::instance().setCullFace(GL_FALSE);
    GLStateCache::instance().setLineWidth(1.0f);
}

void GLRotationSphere::finalize()
{
    GLStateCache::instance().setDepthTest(this->depthTestEnabled);
    GLStateCache::instance().setLineSmooth(this->lineSmoothEnabled);
    GLStateCache::instance().setNormalize(this->normalizeEnabled);
    GLStateCache::instance().setLighting(this->lightingEnabled);
    GLStateCache::instance().setCullFace(this->cullFaceEnabled);
    GLStateCache::instance().setLineWidth(this->lineWidth);
}

void GLRotationSphere::draw()
{
    GLboolean stipple;
    stipple = GLStateCache::instance().getLineStipple();
    GLStateCache::instance().setLineStipple(GL_TRUE, 6, 0xAAAA);

    GLFunctions::begin(GL_LINE_LOOP);
    for (uint i=0;i<360;i++)
    {
        double ang = R_DEG_TO_RAD(i);
        double r1 = cos(ang)*this->scale;
        double r2 = sin(ang)*this->scale;

        double x = r1 + this->position[0];
        double y = r2 + this->position[1];
        double z = this->position[2];

        GLFunctions::vertex3d(x,y,z);
    }
    GLFunctions::end();

    GLFunctions::begin(GL_LINE_LOOP);
    for (uint i=0;i<360;i++)
    {
        double ang = R_DEG_TO_RAD(i);
        double r1 = cos(ang)*this->scale;
        double r2 = sin(ang)*this->scale;

        double x = r1 + this->position[0];
        double y = this->position[1];
        double z = r2 + this->position[2];

        GLFunctions::vertex3d(x,y,z);
    }
    GLFunctions::end();

    GLFunctions::begin(GL_LINE_LOOP);
    for (uint i=0;i<360;i++)
    {
        double ang = R_DEG_TO_RAD(i);
        double r1 = cos(ang)*this->scale;
        double r2 = sin(ang)*this->scale;

        double x = this->position[0];
        double y = r1 + this->position[1];
        double z = r2 + this->position[2];

        GLFunctions::vertex3d(x,y,z);
    }
    GLFunctions::end();

    GLFunctions::begin(GL_LINES);

    GLFunctions::vertex3d(-this->scale+this->position[0],this->position[1],this->position[2]);
    GLFunctions::vertex3d( this->scale+this->position[0],this->position[1],this->position[2]);

    GLFunctions::vertex3d(this->position[0],-this->scale+this->position[1],this->position[2]);
    GLFunctions::vertex3d(this->position[0], this->scale+this->position[1],this->position[2]);

    GLFunctions::vertex3d(this->position[0],this->position[1],-this->scale+this->position[2]);
    GLFunctions::vertex3d(this->position[0],this->position[1], this->scale+this->position[2]);

    GLFunctions::end();

    if (!stipple)
    {
        GLStateCache::instance().setLineStipple(GL_FALSE);
    }
}
