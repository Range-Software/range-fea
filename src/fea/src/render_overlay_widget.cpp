#include <QPainter>

#include "gl_widget.h"
#include "render_overlay_widget.h"

RenderOverlayWidget::RenderOverlayWidget(GLWidget *view, QWidget *parent)
    : QWidget(parent)
    , view(view)
{
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    this->setAttribute(Qt::WA_NoSystemBackground, true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setFocusPolicy(Qt::NoFocus);
}

void RenderOverlayWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    this->view->paintOverlay(painter);
    painter.end();
}
