#include "gl_text_renderer_item.h"

void GLTextRendererItem::_init(const GLTextRendererItem *pGlTextRendererItem)
{
    if (pGlTextRendererItem)
    {
        this->color = pGlTextRendererItem->color;
        this->font = pGlTextRendererItem->font;
        this->position = pGlTextRendererItem->position;
        this->text = pGlTextRendererItem->text;
    }
}

GLTextRendererItem::GLTextRendererItem(const QColor &color, const QFont &font, const QPointF &position, const QString &text)
    : color(color)
    , font(font)
    , position(position)
    , text(text)
{
    this->_init();
}

GLTextRendererItem::GLTextRendererItem(const GLTextRendererItem &glTextRendererItem)
{
    this->_init(&glTextRendererItem);
}

GLTextRendererItem::~GLTextRendererItem()
{

}

GLTextRendererItem &GLTextRendererItem::operator =(const GLTextRendererItem &glTextRendererItem)
{
    this->_init(&glTextRendererItem);
    return (*this);
}
