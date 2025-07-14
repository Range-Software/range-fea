#ifndef GL_TEXT_RENDERER_H
#define GL_TEXT_RENDERER_H

#include <QList>
#include <QPainter>

#include "gl_text_renderer_item.h"

class GLTextRenderer
{

    protected:

        //! Stack of texts to render.
        QList<GLTextRendererItem> list;

    private:

        //! Internal initialization function.
        void _init(const GLTextRenderer *pGlTextRenderer = nullptr);

    public:

        //! Constructor.
        GLTextRenderer();

        //! Copy constructor.
        GLTextRenderer(const GLTextRenderer &glTextRenderer);

        //! Destructor.
        ~GLTextRenderer();

        //! Assignment operator.
        GLTextRenderer &operator =(const GLTextRenderer &glTextRenderer);

        //! Add item.
        void add(const GLTextRendererItem &item);

        //! Clear list.
        void clear();

        //! Render items.
        void render(QPainter &painter) const;

};

#endif // GL_TEXT_RENDERER_H
