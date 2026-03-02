#include "gl_functions.h"
#include "gl_node.h"
#include "gl_state_cache.h"

GLNode::GLNode(GLWidget *glWidget) : GLObject(glWidget)
{
    this->_init();
}

GLNode::GLNode(const GLNode &glNode)
    : GLObject(glNode)
    , RNode(glNode)
{
    this->_init(&glNode);
}

GLNode::~GLNode()
{
}

GLNode &GLNode::operator =(const GLNode &glNode)
{
    this->GLObject::operator =(glNode);
    this->RNode::operator =(glNode);
    this->_init(&glNode);
    return (*this);
}

void GLNode::_init(const GLNode *pGlNode)
{
    if (pGlNode)
    {

    }
}

void GLNode::initialize()
{
    this->lightingEnabled = GLStateCache::instance().getLighting();
    GL_SAFE_CALL(glGetFloatv(GL_POINT_SIZE, &this->pointSize));

    GLStateCache::instance().disableLighting();
    GL_SAFE_CALL(glPointSize(10.0f));
}

void GLNode::finalize()
{
    GLStateCache::instance().setLighting(this->lightingEnabled);
    GL_SAFE_CALL(glPointSize(this->pointSize));
}

void GLNode::draw()
{
    GLFunctions::begin(GL_POINTS);
    GLFunctions::vertex3d(this->getX(),this->getY(),this->getZ());
    GLFunctions::end();
}
