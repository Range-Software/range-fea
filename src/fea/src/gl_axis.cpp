#include "gl_functions.h"
#include "gl_axis.h"
#include "gl_state_cache.h"
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

void GLAxis::finalize()
{
    // Restore previous environment
    GLStateCache::instance().setDepthTest(this->depthTestEnabled);
    GLStateCache::instance().setLineSmooth(this->lineSmoothEnabled);
    GLStateCache::instance().setNormalize(this->normalizeEnabled);
    GLStateCache::instance().setLighting(this->lightingEnabled);
    GLStateCache::instance().setCullFace(this->cullFaceEnabled);
    GLStateCache::instance().setLineWidth(this->lineWidth);
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
            stipple = GLStateCache::instance().getLineStipple();


            GLStateCache::instance().setLineStipple(GL_TRUE, 6, 0xAAAA);

            GLFunctions::begin(GL_LINES);

            this->getGLWidget()->qglColor(red);
            GLFunctions::vertex3f(  0.0f,  0.0f,  0.0f);
            GLFunctions::vertex3f( axisScale,  0.0f,  0.0f);

            this->getGLWidget()->qglColor(green);
            GLFunctions::vertex3f(  0.0f,  0.0f,  0.0f);
            GLFunctions::vertex3f(  0.0f, axisScale,  0.0f);

            this->getGLWidget()->qglColor(blue.lighter());
            GLFunctions::vertex3f(  0.0f,  0.0f,  0.0f);
            GLFunctions::vertex3f(  0.0f,  0.0f, axisScale);

            GLFunctions::end();

            this->getGLWidget()->qglColor(gray);

            GLFunctions::begin(GL_LINES);

            GLFunctions::vertex3f(  0.0f,  0.0f,  0.0f);
            GLFunctions::vertex3f( -axisScale,  0.0f,  0.0f);

            GLFunctions::vertex3f(  0.0f,  0.0f,  0.0f);
            GLFunctions::vertex3f(  0.0f, -axisScale,  0.0f);

            GLFunctions::vertex3f(  0.0f,  0.0f,  0.0f);
            GLFunctions::vertex3f(  0.0f,  0.0f, -axisScale);

            GLFunctions::end();


            if (!stipple)
            {
                GLStateCache::instance().setLineStipple(GL_FALSE);
            }
            break;
        }
        case GL_AXIS_LOCAL:
        {
            GLFunctions::begin(GL_LINES);

            // Lable the X axis.
            this->getGLWidget()->qglColor(red);

            GLFunctions::vertex3f(0.0f, 0.0f, 0.0f);
            GLFunctions::vertex3f(axisScale, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, 1.1f*axisScale, 0.7f*axisScale);
            GLFunctions::vertex3f(0.0f, 0.9f*axisScale, 0.9f*axisScale);
            GLFunctions::vertex3f(0.0f, 1.1f*axisScale, 0.9f*axisScale);
            GLFunctions::vertex3f(0.0f, 0.9f*axisScale, 0.7f*axisScale);

            // Lable the Y axis.
            this->getGLWidget()->qglColor(green);

            GLFunctions::vertex3f(0.0f, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, axisScale, 0.0f);
            GLFunctions::vertex3f(0.7f*axisScale, 0.0f, axisScale);
            GLFunctions::vertex3f(0.8f*axisScale, 0.0f, axisScale);
            GLFunctions::vertex3f(0.8f*axisScale, 0.0f, axisScale);
            GLFunctions::vertex3f(0.9f*axisScale, 0.0f, 1.1f*axisScale);
            GLFunctions::vertex3f(0.8f*axisScale, 0.0f, axisScale);
            GLFunctions::vertex3f(0.9f*axisScale, 0.0f, 0.9f*axisScale);

            // And the Z.
            this->getGLWidget()->qglColor(blue);
            GLFunctions::vertex3f(0.0f, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.0f, axisScale);
            GLFunctions::vertex3f(0.9f*axisScale, 0.9f*axisScale, 0.0f);
            GLFunctions::vertex3f(1.1f*axisScale, 0.9f*axisScale, 0.0f);
            GLFunctions::vertex3f(1.1f*axisScale, 0.9f*axisScale, 0.0f);
            GLFunctions::vertex3f(0.9f*axisScale, 0.7f*axisScale, 0.0f);
            GLFunctions::vertex3f(0.9f*axisScale, 0.7f*axisScale, 0.0f);
            GLFunctions::vertex3f(1.1f*axisScale, 0.7f*axisScale, 0.0f);

            GLFunctions::end();

            GLFunctions::begin(GL_TRIANGLES);

            this->getGLWidget()->qglColor(red);

            GLFunctions::normal3f(1.0f, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.7f*axisScale, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.0f, 0.7f*axisScale);

            this->getGLWidget()->qglColor(green);

            GLFunctions::normal3f(0.0f, 1.0f, 0.0f);
            GLFunctions::vertex3f(0.7f*axisScale, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.0f, 0.7f*axisScale);

            this->getGLWidget()->qglColor(blue);

            GLFunctions::normal3f(0.0f, 0.0f, 1.0f);
            GLFunctions::vertex3f(0.7f*axisScale, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.7f*axisScale, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.0f, 0.0f);

            this->getGLWidget()->qglColor(QColor::fromRgbF(0.8f, 0.8f, 0.8f, 0.8f));

            GLFunctions::normal3f(0.49f, 0.49f, 0.49f);
            GLFunctions::vertex3f(0.7f*axisScale, 0.0f, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.7f*axisScale, 0.0f);
            GLFunctions::vertex3f(0.0f, 0.0f, 0.7f*axisScale);

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

            GLFunctions::vertex3f(  axisScale,  0.0f,  0.0f);
            GLFunctions::vertex3f( -axisScale,  0.0f,  0.0f);

            GLFunctions::vertex3f(  0.0f,  axisScale,  0.0f);
            GLFunctions::vertex3f(  0.0f, -axisScale,  0.0f);

            GLFunctions::vertex3f(  0.0f,  0.0f,  axisScale);
            GLFunctions::vertex3f(  0.0f,  0.0f, -axisScale);

            GLFunctions::end();
            break;
        }
    }
}
