#include "gl_functions.h"
#include "gl_rotation_sphere.h"


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
    GL_SAFE_CALL(glGetBooleanv(GL_DEPTH_TEST, &this->depthTestEnabled));
    GL_SAFE_CALL(glGetBooleanv(GL_LINE_SMOOTH, &this->lineSmoothEnabled));
    GL_SAFE_CALL(glGetIntegerv(GL_LINE_SMOOTH_HINT, &this->lineSmoothHint));
    GL_SAFE_CALL(glGetBooleanv(GL_NORMALIZE, &this->normalizeEnabled));
    GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &this->lightingEnabled));
    GL_SAFE_CALL(glGetFloatv(GL_LINE_WIDTH, &this->lineWidth));
    GL_SAFE_CALL(glGetBooleanv(GL_CULL_FACE, &this->cullFaceEnabled));
    // Initialize environment
    GL_SAFE_CALL(glEnable(GL_DEPTH_TEST));
    GL_SAFE_CALL(glEnable(GL_LINE_SMOOTH));
    GL_SAFE_CALL(glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE));
    GL_SAFE_CALL(glEnable(GL_NORMALIZE));
    GL_SAFE_CALL(glDisable(GL_LIGHTING));
    GL_SAFE_CALL(glDisable(GL_CULL_FACE));
    GL_SAFE_CALL(glLineWidth(1.0f));
}

void GLRotationSphere::finalize()
{
    GL_SAFE_CALL(this->depthTestEnabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
    GL_SAFE_CALL(this->lineSmoothEnabled ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH));
    GL_SAFE_CALL(this->normalizeEnabled ? glEnable(GL_NORMALIZE) :glDisable(GL_NORMALIZE));
    GL_SAFE_CALL(this->lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
    GL_SAFE_CALL(this->cullFaceEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));
    GL_SAFE_CALL(glLineWidth(this->lineWidth));
    GL_SAFE_CALL(glHint(GL_LINE_SMOOTH_HINT, GLenum(this->lineSmoothHint)));
}

void GLRotationSphere::draw()
{
    GLboolean stipple;
    GL_SAFE_CALL(glGetBooleanv(GL_LINE_STIPPLE,&stipple));
    GL_SAFE_CALL(glPushAttrib(GL_ENABLE_BIT));
    GL_SAFE_CALL(glLineStipple(6, 0xAAAA));
    GL_SAFE_CALL(glEnable(GL_LINE_STIPPLE));

    GLFunctions::begin(GL_LINE_LOOP);
    for (uint i=0;i<360;i++)
    {
        double ang = R_DEG_TO_RAD(i);
        double r1 = cos(ang)*this->scale;
        double r2 = sin(ang)*this->scale;

        double x = r1 + this->position[0];
        double y = r2 + this->position[1];
        double z = this->position[2];

        GL_SAFE_CALL(glVertex3d(x,y,z));
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

        GL_SAFE_CALL(glVertex3d(x,y,z));
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

        GL_SAFE_CALL(glVertex3d(x,y,z));
    }
    GLFunctions::end();

    GLFunctions::begin(GL_LINES);

    GL_SAFE_CALL(glVertex3d(-this->scale+this->position[0],this->position[1],this->position[2]));
    GL_SAFE_CALL(glVertex3d( this->scale+this->position[0],this->position[1],this->position[2]));

    GL_SAFE_CALL(glVertex3d(this->position[0],-this->scale+this->position[1],this->position[2]));
    GL_SAFE_CALL(glVertex3d(this->position[0], this->scale+this->position[1],this->position[2]));

    GL_SAFE_CALL(glVertex3d(this->position[0],this->position[1],-this->scale+this->position[2]));
    GL_SAFE_CALL(glVertex3d(this->position[0],this->position[1], this->scale+this->position[2]));

    GLFunctions::end();

    GL_SAFE_CALL(glPopAttrib());
    if (!stipple)
    {
        GL_SAFE_CALL(glDisable(GL_LINE_STIPPLE));
    }
}
