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
