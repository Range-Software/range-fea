#include "gl_entity.h"

void GLEntity::_init(const GLEntity *pGLEntity)
{
    if (pGLEntity)
    {
        this->entityID = pGLEntity->entityID;
    }
}

GLEntity::GLEntity(GLWidget *glWidget, const SessionEntityID &entityID)
    : GLObject(glWidget)
    , entityID(entityID)
{
    this->_init();
}

GLEntity::GLEntity(const GLEntity &gLEntity)
    : GLObject(gLEntity)
{
    this->_init(&gLEntity);
}

GLEntity::~GLEntity()
{
}

GLEntity &GLEntity::operator =(const GLEntity &gLEntity)
{
    this->GLObject::operator =(gLEntity);
    this->_init(&gLEntity);
    return (*this);
}

const SessionEntityID & GLEntity::getEntityID() const
{
    return this->entityID;
}

void GLEntity::initialize()
{
}

void GLEntity::finalize()
{
}

void GLEntity::draw()
{
}
