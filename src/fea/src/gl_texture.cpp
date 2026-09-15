#include <QImage>

#include <rbl_logger.h>

#include "gl_texture.h"
#include "render_backend.h"
#include "rhi_renderer.h"

GLTexture::GLTexture()
    : texture(nullptr)
    , loaded(false)
{
}

GLTexture::~GLTexture()
{
    this->unload();
}

void GLTexture::load(const QString &file)
{
    QImage image;
    if (!image.load(file))
    {
        RLogger::warning("Failed to load texture file \'%s\'\n",file.toUtf8().constData());
        return;
    }

    if (RenderBackend::isRhi())
    {
        RhiRenderer *renderer = RhiRenderer::current();
        if (!renderer)
        {
            return;
        }
        // The QRhi renderer owns and caches the colour map textures, keyed by file name.
        renderer->setColorMap(file, image);
        this->loaded = true;
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
        if (RenderBackend::isRhi())
        {
            if (RhiRenderer *renderer = RhiRenderer::current())
            {
                renderer->clearColorMap();
            }
        }
        else if (this->texture)
        {
            this->texture->release();
            delete this->texture;
            this->texture = nullptr;
        }
    }
    this->loaded = false;
}
