#include <QImage>

#include <rbl_logger.h>

#include "gl_texture.h"

GLTexture::GLTexture()
    : texture(nullptr)
    , loaded(false)
{
}

GLTexture::~GLTexture()
{
}

void GLTexture::load(const QString &file)
{
    QImage image;
    if (!image.load(file))
    {
        RLogger::warning("Failed to load texture file \'%s\'\n",file.toUtf8().constData());
        return;
    }

    this->texture = new QOpenGLTexture(QOpenGLTexture::Target1D);
    this->texture->setData(image.flipped());

    this->texture->setMinMagFilters(QOpenGLTexture::LinearMipMapNearest,QOpenGLTexture::Linear);
    this->texture->setWrapMode(QOpenGLTexture::Repeat);

    this->texture->bind();

    this->loaded = true;
}

void GLTexture::unload()
{
    if (this->loaded)
    {
        this->texture->release();
        delete this->texture;
    }
    this->loaded = false;
}
