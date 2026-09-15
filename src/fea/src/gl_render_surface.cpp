#include <QSurfaceFormat>

#include "gl_render_surface.h"
#include "gl_widget.h"

GLRenderSurface::GLRenderSurface(GLWidget *view, QWidget *parent)
    : QOpenGLWidget(parent)
    , view(view)
{
    this->setFormat(QSurfaceFormat::defaultFormat());
    // Mouse and key handling stays in GLWidget — let the events fall through.
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    this->setFocusPolicy(Qt::NoFocus);
}

QWidget *GLRenderSurface::asWidget()
{
    return this;
}

void GLRenderSurface::requestUpdate()
{
    this->update();
}

QImage GLRenderSurface::grabImage()
{
    return this->grabFramebuffer();
}

void GLRenderSurface::initializeGL()
{
    this->view->initializeRender();
}

void GLRenderSurface::resizeGL(int width, int height)
{
    this->view->resizeRender(width, height);
}

void GLRenderSurface::paintGL()
{
    this->view->paintRender();
}
