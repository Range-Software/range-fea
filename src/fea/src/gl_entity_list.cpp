#include "gl_entity_list.h"
#include "gl_functions.h"
#include "gl_state_cache.h"

GLEntityList::GLEntityList()
{
}

GLEntityList::GLEntityList(const GLEntityList &)
{
    // GPU resources are not copied — new entity list starts invalid.
}

GLEntityList::~GLEntityList()
{
}

GLEntityList &GLEntityList::operator =(const GLEntityList &)
{
    // GPU resources are not copied — destination list starts invalid.
    for (int i = 0; i < GL_ENTITY_LIST_ITEM_N_LISTS; i++)
    {
        this->vbo[i].invalidate();
    }
    return (*this);
}

GLVertexBuffer &GLEntityList::getVBO(GLuint listPosition)
{
    return this->vbo[listPosition];
}

const GLVertexBuffer &GLEntityList::getVBO(GLuint listPosition) const
{
    return this->vbo[listPosition];
}

bool GLEntityList::getVBOValid(GLuint listPosition) const
{
    if (listPosition >= GL_ENTITY_LIST_ITEM_N_LISTS)
    {
        return false;
    }
    return this->vbo[listPosition].isValid();
}

void GLEntityList::setVBOInvalid(GLuint listPosition)
{
    if (listPosition < GL_ENTITY_LIST_ITEM_N_LISTS)
    {
        this->vbo[listPosition].invalidate();
    }
}

void GLEntityList::setListInvalid(GLuint listPosition)
{
    this->setVBOInvalid(listPosition);
}

// Compat stubs

bool GLEntityList::getListValid(GLuint listPosition) const
{
    return this->getVBOValid(listPosition);
}

void GLEntityList::newList(GLuint listPosition, GLenum)
{
    if (listPosition < GL_ENTITY_LIST_ITEM_N_LISTS)
    {
        GLFunctions::beginVBORecording(&this->vbo[listPosition]);
    }
}

void GLEntityList::endList(GLuint)
{
    GLFunctions::endVBORecording();
}

void GLEntityList::callList(GLuint listPosition) const
{
    if (listPosition < GL_ENTITY_LIST_ITEM_N_LISTS)
    {
        const GLVertexBuffer &vbo = this->vbo[listPosition];
        // Sync uUseTexture with what was recorded: textured VBOs use the 1D colour-map
        // sampler, non-textured VBOs use the per-vertex colour attribute.
        GLStateCache::instance().setTexture1D(vbo.getUsesTexture() ? GL_TRUE : GL_FALSE);
        vbo.render();
        // Always reset after render so subsequent draws default to non-textured.
        GLStateCache::instance().setTexture1D(GL_FALSE);
    }
}
