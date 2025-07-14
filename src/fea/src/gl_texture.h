#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include <QOpenGLTexture>

#include <QString>

class GLTexture
{
    private:

        //! Texture.
        QOpenGLTexture *texture;
        //! Texture loaded flag.
        bool loaded;

    public:

        //! Constructor.
        GLTexture();

        //! Destructor.
        ~GLTexture();

        //! Load texture
        void load(const QString &file);

        //! Unload texture
        void unload();
};

#endif // GL_TEXTURE_H
