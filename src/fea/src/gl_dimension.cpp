#include "gl_arrow.h"
#include "gl_dimension.h"
#include "gl_widget.h"

const double GLDimension::arrowScale = 0.1;

void GLDimension::_init(const GLDimension *pGlDimension)
{
    if (pGlDimension)
    {
        this->scale = pGlDimension->scale;
        this->xMin = pGlDimension->xMin;
        this->xMax = pGlDimension->xMax;
        this->yMin = pGlDimension->yMin;
        this->yMax = pGlDimension->yMax;
        this->zMin = pGlDimension->zMin;
        this->zMax = pGlDimension->zMax;
    }
}

GLDimension::GLDimension(GLWidget *glWidget,
                         double scale,
                         double xMin,
                         double xMax,
                         double yMin,
                         double yMax,
                         double zMin,
                         double zMax)
    : GLObject(glWidget)
    , scale(scale)
    , xMin(xMin)
    , xMax(xMax)
    , yMin(yMin)
    , yMax(yMax)
    , zMin(zMin)
    , zMax(zMax)
{
    this->_init();
}

GLDimension::GLDimension(const GLDimension &glDimension)
    : GLObject(glDimension)
{
    this->_init(&glDimension);
}

GLDimension &GLDimension::operator=(const GLDimension &glDimension)
{
    this->_init(&glDimension);
    return (*this);
}

void GLDimension::initialize()
{

}

void GLDimension::finalize()
{

}

void GLDimension::draw()
{
    double dx = this->xMax-this->xMin;
    double dy = this->yMax-this->yMin;
    double dz = this->zMax-this->zMin;

    double dmax = std::max(std::max(std::fabs(dx),std::fabs(dy)),std::fabs(dz));

    double dxScale = GLDimension::arrowScale * this->scale * dmax / std::fabs(dx);
    double dyScale = GLDimension::arrowScale * this->scale * dmax / std::fabs(dy);
    double dzScale = GLDimension::arrowScale * this->scale * dmax / std::fabs(dz);

    GLArrow(this->getGLWidget(),RR3Vector(this->xMin,this->yMin,this->zMin),RR3Vector( dx,0.0,0.0),true,true,dxScale).paint();
    GLArrow(this->getGLWidget(),RR3Vector(this->xMax,this->yMin,this->zMin),RR3Vector(-dx,0.0,0.0),true,true,dxScale).paint();

    this->getGLWidget()->renderText((this->xMin+this->xMax)/2.0,this->yMin,this->zMin,"∆x = " + QLocale().toString(dx) + " " + RVariable::getUnits(R_VARIABLE_LENGTH),QFont("Courier",20));

    GLArrow(this->getGLWidget(),RR3Vector(this->xMin,this->yMin,this->zMin),RR3Vector(0.0, dy,0.0),true,true,dyScale).paint();
    GLArrow(this->getGLWidget(),RR3Vector(this->xMin,this->yMax,this->zMin),RR3Vector(0.0,-dy,0.0),true,true,dyScale).paint();

    this->getGLWidget()->renderText(this->xMin,(this->yMin+this->yMax)/2.0,this->zMin,"∆y = " + QLocale().toString(dy) + " " + RVariable::getUnits(R_VARIABLE_LENGTH),QFont("Courier",20));

    GLArrow(this->getGLWidget(),RR3Vector(this->xMin,this->yMin,this->zMin),RR3Vector(0.0,0.0, dz),true,true,dzScale).paint();
    GLArrow(this->getGLWidget(),RR3Vector(this->xMin,this->yMin,this->zMax),RR3Vector(0.0,0.0,-dz),true,true,dzScale).paint();

    this->getGLWidget()->renderText(this->xMin,this->yMin,(this->zMin+this->zMax)/2.0,"∆z = " + QLocale().toString(dz) + " " + RVariable::getUnits(R_VARIABLE_LENGTH),QFont("Courier",20));
}
