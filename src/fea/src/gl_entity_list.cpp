#include "gl_entity_list.h"

GLEntityList::GLEntityList()
    : GLList(GL_ENTITY_LIST_ITEM_N_LISTS)
    , useVBO(false)  // VBO disabled: VAO + legacy client-state mixing is incompatible
{
    this->_init();
}

GLEntityList::GLEntityList(const GLEntityList &glEntityList)
    : GLList(glEntityList)
    , useVBO(false)
{
    this->_init(&glEntityList);
}

GLEntityList::~GLEntityList()
{
}

GLList &GLEntityList::operator =(const GLEntityList &glEntityList)
{
    this->GLList::operator =(glEntityList);
    this->_init(&glEntityList);
    return (*this);
}

void GLEntityList::_init(const GLEntityList *pGlEntityList)
{
    if (pGlEntityList)
    {
        this->useVBO = pGlEntityList->useVBO;
    }
}

bool GLEntityList::getUseVBO() const
{
    return this->useVBO;
}

void GLEntityList::setUseVBO(bool useVBO)
{
    this->useVBO = useVBO;
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
    // Invalidate both display list and VBO
    GLList::setListInvalid(listPosition);
    this->setVBOInvalid(listPosition);
}
