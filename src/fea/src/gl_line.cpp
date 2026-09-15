#include "gl_functions.h"
#include "gl_line.h"
#include "gl_state_cache.h"


void GLLine::_init(const GLLine *pGlLine)
{
    if (pGlLine)
    {
        this->start = pGlLine->start;
        this->end = pGlLine->end;
    }
}

GLLine::GLLine(GLWidget *glWidget, const RR3Vector &start, const RR3Vector &end, GLfloat width)
    : GLObject(glWidget)
    , start(start)
    , end(end)
    , width(width)
{
    this->_init();
}

GLLine::GLLine(const GLLine &glLine)
    : GLObject(glLine)
{
    this->_init(&glLine);
}

GLLine::~GLLine()
{

}

GLLine &GLLine::operator =(const GLLine &glLine)
{
    this->GLObject::operator =(glLine);
    this->_init(&glLine);
    return (*this);
}

void GLLine::initialize()
{
    // Save current settings
    this->normalizeEnabled = GLStateCache::instance().getNormalize();
    this->lightingEnabled = GLStateCache::instance().getLighting();
    this->lineWidth = GLStateCache::instance().getLineWidth();
    // Initialize environment
    GLStateCache::instance().setNormalize(GL_FALSE);
    GLStateCache::instance().disableLighting();
    GLStateCache::instance().setLineWidth(this->width);
}

void GLLine::finalize()
{
    GLStateCache::instance().setNormalize(this->normalizeEnabled);
    GLStateCache::instance().setLighting(this->lightingEnabled);
    GLStateCache::instance().setLineWidth(this->lineWidth);
}

void GLLine::draw()
{
    GLFunctions::begin(GL_LINES);
    GLFunctions::vertex3d(this->start[0],this->start[1],this->start[2]);
    GLFunctions::vertex3d(this->end[0],this->end[1],this->end[2]);
    GLFunctions::end();
}
