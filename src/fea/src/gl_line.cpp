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
    GL_SAFE_CALL(glGetBooleanv(GL_NORMALIZE, &this->normalizeEnabled));
    this->lightingEnabled = GLStateCache::instance().getLighting();
    GL_SAFE_CALL(glGetFloatv(GL_LINE_WIDTH, &this->lineWidth));
    // Initialize environment
    GL_SAFE_CALL(glDisable(GL_NORMALIZE));
    GLStateCache::instance().disableLighting();
    GL_SAFE_CALL(glLineWidth(this->width));
}

void GLLine::finalize()
{
    GL_SAFE_CALL(this->normalizeEnabled ? glEnable(GL_NORMALIZE) :glDisable(GL_NORMALIZE));
    GLStateCache::instance().setLighting(this->lightingEnabled);
    GL_SAFE_CALL(glLineWidth(this->lineWidth));
}

void GLLine::draw()
{
    GLFunctions::begin(GL_LINES);
    GLFunctions::vertex3d(this->start[0],this->start[1],this->start[2]);
    GLFunctions::vertex3d(this->end[0],this->end[1],this->end[2]);
    GLFunctions::end();
}
