#ifndef GL_RENDER_SURFACE_H
#define GL_RENDER_SURFACE_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QOpenGLWidget>

#include "render_surface.h"

class GLWidget;

//! Legacy OpenGL render surface.
//! Forwards the QOpenGLWidget render callbacks to the hosting GLWidget.
class GLRenderSurface : public QOpenGLWidget, public RenderSurface
{

    Q_OBJECT

    protected:

        //! Hosting view.
        GLWidget *view;

    public:

        //! Constructor.
        explicit GLRenderSurface(GLWidget *view, QWidget *parent = nullptr);

        //! Return the surface as a QWidget.
        QWidget *asWidget() override;

        //! Schedule a repaint of the surface.
        void requestUpdate() override;

        //! Grab the last rendered frame.
        QImage grabImage() override;

    protected:

        //! Initialize scene.
        void initializeGL() override;

        //! Resize scene.
        void resizeGL(int width, int height) override;

        //! Paint scene.
        void paintGL() override;

};

#endif // GL_RENDER_SURFACE_H
