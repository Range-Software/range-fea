#ifndef RENDER_OVERLAY_WIDGET_H
#define RENDER_OVERLAY_WIDGET_H

#include <QWidget>

class GLWidget;

//! Transparent 2D overlay stacked on top of the render surface.
//!
//! The value ranges, message and info boxes and the projected 3D text labels
//! are plain QPainter drawing.  Keeping them in a separate widget means the
//! same code serves both rendering backends — QRhiWidget, unlike
//! QOpenGLWidget, cannot be painted into with QPainter.
class RenderOverlayWidget : public QWidget
{

    Q_OBJECT

    protected:

        //! Hosting view.
        GLWidget *view;

    public:

        //! Constructor.
        explicit RenderOverlayWidget(GLWidget *view, QWidget *parent = nullptr);

    protected:

        //! Paint the overlay.
        void paintEvent(QPaintEvent *paintEvent) override;

};

#endif // RENDER_OVERLAY_WIDGET_H
