#ifndef GL_TEXT_RENDERER_ITEM_H
#define GL_TEXT_RENDERER_ITEM_H

#include <QColor>
#include <QFont>
#include <QPointF>

class GLTextRendererItem
{

    protected:

        //! Text color.
        QColor color;
        //! Font.
        QFont font;
        //! Position.
        QPointF position;
        //! Text.
        QString text;

    private:

        //! Internal initialization function.
        void _init(const GLTextRendererItem *pGlTextRendererItem = nullptr);

    public:

        //! Constructor.
        GLTextRendererItem(const QColor &color, const QFont &font, const QPointF &position, const QString &text);

        //! Copy constructor.
        GLTextRendererItem(const GLTextRendererItem &glTextRendererItem);

        //! Destructor.
        ~GLTextRendererItem();

        //! Assignment operator.
        GLTextRendererItem &operator =(const GLTextRendererItem &glTextRendererItem);

        friend class GLTextRenderer;

};

#endif // GL_TEXT_RENDERER_ITEM_H
