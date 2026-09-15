#ifndef RENDER_SURFACE_H
#define RENDER_SURFACE_H

class QWidget;
class QImage;

//! Abstract render surface hosted by GLWidget.
//!
//! GLWidget owns all of the view logic and knows nothing about the backend that
//! actually puts pixels on the screen.  The surface is a child widget filling
//! the whole GLWidget; it drives the three render callbacks and hands back a
//! grabbed frame for screenshots.
class RenderSurface
{

    public:

        //! Destructor.
        virtual ~RenderSurface() {}

        //! Return the surface as a QWidget.
        virtual QWidget *asWidget() = 0;

        //! Schedule a repaint of the surface.
        virtual void requestUpdate() = 0;

        //! Grab the last rendered frame.
        virtual QImage grabImage() = 0;

};

#endif // RENDER_SURFACE_H
