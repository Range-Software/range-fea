#include <rbl_error.h>
#include <rbl_logger.h>

#include "gl_functions.h"
#include "gl_list.h"

GLList::GLList(GLuint nLists)
    : listBase(0)
    , nLists(0)
{
    this->setNLists(nLists);
    this->_init();
}

GLList::GLList(const GLList &glList)
{
    this->_init(&glList);
}

GLList::~GLList()
{
    GL_SAFE_CALL(glDeleteLists(this->getBase(),GLsizei(this->getNLists())));
}

GLList &GLList::operator =(const GLList &glList)
{
    this->_init(&glList);
    return (*this);
}

void GLList::_init(const GLList *pGlList)
{
    if (pGlList)
    {
        this->listBase = pGlList->listBase;
        this->nLists = pGlList->nLists;
        this->listValid = pGlList->listValid;
    }
}

GLuint GLList::getBase() const
{
    return this->listBase;
}

GLuint GLList::getNLists() const
{
    return GLuint(this->nLists);
}

void GLList::setNLists(GLuint nLists)
{
    if (this->getBase() > 0)
    {
        GL_SAFE_CALL(glDeleteLists(this->getBase(),GLsizei(this->getNLists())));
    }
    GL_SAFE_CALL(this->listBase = glGenLists(GLsizei(nLists)));
    if (!glIsList(this->listBase))
    {
        RLogger::error("Failed to generate openGL lists\n");
    }
    this->listValid.resize(nLists,false);
    std::fill(this->listValid.begin(),this->listValid.end(),false);
    this->nLists = nLists;
}

bool GLList::getListValid(GLuint listPosition) const
{
    R_ERROR_ASSERT(listPosition < this->getNLists());
    return this->listValid[listPosition];
}

void GLList::setListInvalid(GLuint listPosition)
{
    R_ERROR_ASSERT(listPosition < this->getNLists());
    this->listValid[listPosition] = false;
}

bool GLList::getValid() const
{
    if (this->getBase() == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void GLList::newList(GLuint listPosition, GLenum mode)
{
    R_ERROR_ASSERT(listPosition < this->getNLists());
    this->listValid[listPosition] = false;
    GL_SAFE_CALL(glNewList(this->getBase() + listPosition, mode));
}

void GLList::endList(GLuint listPosition)
{
    GL_SAFE_CALL(glEndList());
    this->listValid[listPosition] = true;
}

void GLList::callList(GLuint listPosition) const
{
    R_ERROR_ASSERT(listPosition < this->getNLists());
    if (this->getListValid(listPosition))
    {
        R_ERROR_ASSERT(glIsList(this->getBase() + listPosition));
        GL_SAFE_CALL(glCallList(this->getBase() + listPosition));
    }
}

void GLList::callLists() const
{
    for (GLuint i=0;i<this->getNLists();i++)
    {
        this->callList(i);
    }
}
