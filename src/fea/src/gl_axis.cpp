#include "gl_functions.h"
#include "gl_axis.h"
#include "gl_widget.h"

void GLAxis::_init(const GLAxis *pGlAxis)
{
    if (pGlAxis)
    {
        this->type = pGlAxis->type;
        this->size = pGlAxis->size;
        this->name = pGlAxis->name;
    }
}

GLAxis::GLAxis(GLWidget *glWidget, GLAxisType type, const QString &name)
    : GLObject(glWidget)
    , type(type)
    , size(1.0)
    , name(name)
{
    this->_init();
}

GLAxis::GLAxis(const GLAxis &glAxis) : GLObject(glAxis)
{
    this->_init(&glAxis);
}

GLAxis::~GLAxis()
{
}

GLAxis & GLAxis::operator =(const GLAxis &glAxis)
{
    this->GLObject::operator =(glAxis);
    this->_init(&glAxis);
    return (*this);
}

GLAxisType GLAxis::getType() const
{
    return this->type;
}

void GLAxis::setType(GLAxisType type)
{
    this->type = type;
}

float GLAxis::getSize() const
{
    return this->size;
}

void GLAxis::setSize(float size)
{
    this->size = size;
}

void GLAxis::initialize()
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

void GLAxis::finalize()
{
    // Restore previous environment
    GL_SAFE_CALL(this->depthTestEnabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
    GL_SAFE_CALL(this->lineSmoothEnabled ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH));
    GL_SAFE_CALL(this->normalizeEnabled ? glEnable(GL_NORMALIZE) :glDisable(GL_NORMALIZE));
    GL_SAFE_CALL(this->lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
    GL_SAFE_CALL(this->cullFaceEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));
    GL_SAFE_CALL(glLineWidth(this->lineWidth));
    GL_SAFE_CALL(glHint(GL_LINE_SMOOTH_HINT, GLenum(this->lineSmoothHint)));
//    glBlendFunc(GL_SRC_ALPHA, this->bendAlphaFunc);
}

void GLAxis::draw()
{
    GLfloat axisScale = this->getSize();

    QColor red(Qt::red);
    QColor green(Qt::green);
    QColor blue(Qt::blue);
    QColor gray(Qt::gray);

    QString postFix(this->name.length() == 0 ? QString() : QString(" - ") + this->name);

    switch (this->getType())
    {
        case GL_AXIS_GLOBAL:
        {
            this->getGLWidget()->qglColor(red);
            this->getGLWidget()->renderText(double(axisScale),0.0,0.0,QString("X") + postFix);

            this->getGLWidget()->qglColor(green);
            this->getGLWidget()->renderText(0.0,double(axisScale),0.0,QString("Y") + postFix);

            this->getGLWidget()->qglColor(blue);
            this->getGLWidget()->renderText(0.0,0.0,double(axisScale),QString("Z") + postFix);

            GLboolean stipple;
            GL_SAFE_CALL(glGetBooleanv(GL_LINE_STIPPLE,&stipple));

            GL_SAFE_CALL(glPushAttrib(GL_ENABLE_BIT));

            GL_SAFE_CALL(glLineStipple(6, 0xAAAA));
            GL_SAFE_CALL(glEnable(GL_LINE_STIPPLE));

            GLFunctions::begin(GL_LINES);

            this->getGLWidget()->qglColor(red);
            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f,  0.0f));
            GL_SAFE_CALL(glVertex3f( axisScale,  0.0f,  0.0f));

            this->getGLWidget()->qglColor(green);
            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f,  0.0f));
            GL_SAFE_CALL(glVertex3f(  0.0f, axisScale,  0.0f));

            this->getGLWidget()->qglColor(blue.lighter());
            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f,  0.0f));
            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f, axisScale));

            GLFunctions::end();

            this->getGLWidget()->qglColor(gray);

            GLFunctions::begin(GL_LINES);

            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f,  0.0f));
            GL_SAFE_CALL(glVertex3f( -axisScale,  0.0f,  0.0f));

            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f,  0.0f));
            GL_SAFE_CALL(glVertex3f(  0.0f, -axisScale,  0.0f));

            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f,  0.0f));
            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f, -axisScale));

            GLFunctions::end();

            GL_SAFE_CALL(glPopAttrib());

            if (!stipple)
            {
                GL_SAFE_CALL(glDisable(GL_LINE_STIPPLE));
            }
            break;
        }
        case GL_AXIS_LOCAL:
        {
            GLFunctions::begin(GL_LINES);

            // Lable the X axis.
            this->getGLWidget()->qglColor(red);

            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(axisScale, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 1.1f*axisScale, 0.7f*axisScale));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.9f*axisScale, 0.9f*axisScale));
            GL_SAFE_CALL(glVertex3f(0.0f, 1.1f*axisScale, 0.9f*axisScale));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.9f*axisScale, 0.7f*axisScale));

            // Lable the Y axis.
            this->getGLWidget()->qglColor(green);

            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.7f*axisScale, 0.0f, axisScale));
            GL_SAFE_CALL(glVertex3f(0.8f*axisScale, 0.0f, axisScale));
            GL_SAFE_CALL(glVertex3f(0.8f*axisScale, 0.0f, axisScale));
            GL_SAFE_CALL(glVertex3f(0.9f*axisScale, 0.0f, 1.1f*axisScale));
            GL_SAFE_CALL(glVertex3f(0.8f*axisScale, 0.0f, axisScale));
            GL_SAFE_CALL(glVertex3f(0.9f*axisScale, 0.0f, 0.9f*axisScale));

            // And the Z.
            this->getGLWidget()->qglColor(blue);
            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, axisScale));
            GL_SAFE_CALL(glVertex3f(0.9f*axisScale, 0.9f*axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(1.1f*axisScale, 0.9f*axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(1.1f*axisScale, 0.9f*axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.9f*axisScale, 0.7f*axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.9f*axisScale, 0.7f*axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(1.1f*axisScale, 0.7f*axisScale, 0.0f));

            GLFunctions::end();

            GLFunctions::begin(GL_TRIANGLES);

            this->getGLWidget()->qglColor(red);

            GL_SAFE_CALL(glNormal3f(1.0f, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.7f*axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.7f*axisScale));

            this->getGLWidget()->qglColor(green);

            GL_SAFE_CALL(glNormal3f(0.0f, 1.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.7f*axisScale, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.7f*axisScale));

            this->getGLWidget()->qglColor(blue);

            GL_SAFE_CALL(glNormal3f(0.0f, 0.0f, 1.0f));
            GL_SAFE_CALL(glVertex3f(0.7f*axisScale, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.7f*axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.0f));

            GL_SAFE_CALL(glColor4f (0.8f, 0.8f, 0.8f, 0.8f));

            GL_SAFE_CALL(glNormal3f(0.49f, 0.49f, 0.49f));
            GL_SAFE_CALL(glVertex3f(0.7f*axisScale, 0.0f, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.7f*axisScale, 0.0f));
            GL_SAFE_CALL(glVertex3f(0.0f, 0.0f, 0.7f*axisScale));

            GLFunctions::end();
            break;
        }
        case GL_AXIS_NONE:
        case GL_AXIS_POSITION:
        default:
        {
            this->getGLWidget()->qglColor(QColorConstants::White);
            this->getGLWidget()->renderText(double(axisScale),0.0,0.0,QString("X") + postFix);

            this->getGLWidget()->qglColor(QColorConstants::White);
            this->getGLWidget()->renderText(0.0,double(axisScale),0.0,QString("Y") + postFix);

            this->getGLWidget()->qglColor(QColorConstants::White);
            this->getGLWidget()->renderText(0.0,0.0,double(axisScale),QString("Z") + postFix);

            GLFunctions::begin(GL_LINES);

            this->getGLWidget()->qglColor(gray);

            GL_SAFE_CALL(glVertex3f(  axisScale,  0.0f,  0.0f));
            GL_SAFE_CALL(glVertex3f( -axisScale,  0.0f,  0.0f));

            GL_SAFE_CALL(glVertex3f(  0.0f,  axisScale,  0.0f));
            GL_SAFE_CALL(glVertex3f(  0.0f, -axisScale,  0.0f));

            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f,  axisScale));
            GL_SAFE_CALL(glVertex3f(  0.0f,  0.0f, -axisScale));

            GLFunctions::end();
            break;
        }
    }
}
