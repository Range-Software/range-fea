#include "gl_entity_list.h"
#include "gl_functions.h"

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
        this->vbo[listPosition].render();
    }
}
