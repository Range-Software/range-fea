#include <QImage>

#include <rbl_logger.h>

#include "color_scale.h"
#include "gl_state_cache.h"
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

    GLStateCache::instance().setColorMapBands(ColorScale::getBandCount(file),ColorScale::getBandOutline(file));

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
    this->texture->setData(image.flipped(),QOpenGLTexture::DontGenerateMipMaps);

    // Averaged mip map levels would mix colors of the color map, values at its ends must not wrap around.
    this->texture->setMinMagFilters(QOpenGLTexture::Linear,QOpenGLTexture::Linear);
    this->texture->setWrapMode(QOpenGLTexture::ClampToEdge);

    this->texture->bind();

    this->loaded = true;
}

void GLTexture::unload()
{
    if (this->loaded)
    {
        GLStateCache::instance().setColorMapBands(0,false);

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
