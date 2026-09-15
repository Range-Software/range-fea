#include <rbl_logger.h>

#include "gl_widget.h"
#include "render_backend.h"
#include "rhi_render_surface.h"

RhiRenderSurface::RhiRenderSurface(GLWidget *view, QWidget *parent)
    : QRhiWidget(parent)
    , view(view)
    , viewInitialized(false)
{
    this->setApi(RhiRenderSurface::resolveApi());
    this->setSampleCount(4);
    // Mouse and key handling stays in GLWidget — let the events fall through.
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    this->setFocusPolicy(Qt::NoFocus);
}

QRhiWidget::Api RhiRenderSurface::resolveApi()
{
    switch (RenderBackend::getRhiApi())
    {
        case RenderBackend::RhiApi::OpenGL: return QRhiWidget::Api::OpenGL;
        case RenderBackend::RhiApi::Vulkan: return QRhiWidget::Api::Vulkan;
        case RenderBackend::RhiApi::Metal:  return QRhiWidget::Api::Metal;
        case RenderBackend::RhiApi::D3D11:  return QRhiWidget::Api::Direct3D11;
        case RenderBackend::RhiApi::D3D12:  return QRhiWidget::Api::Direct3D12;
        case RenderBackend::RhiApi::Null:   return QRhiWidget::Api::Null;
        case RenderBackend::RhiApi::Auto:   break;
    }
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    return QRhiWidget::Api::Metal;
#elif defined(Q_OS_WIN)
    return QRhiWidget::Api::Direct3D11;
#else
    return QRhiWidget::Api::OpenGL;
#endif
}

QWidget *RhiRenderSurface::asWidget()
{
    return this;
}

void RhiRenderSurface::requestUpdate()
{
    this->update();
}

QImage RhiRenderSurface::grabImage()
{
    return this->grabFramebuffer();
}

RhiRenderer &RhiRenderSurface::getRenderer()
{
    return this->renderer;
}

void RhiRenderSurface::initialize(QRhiCommandBuffer *)
{
    QRhiRenderTarget *renderTarget = this->renderTarget();
    if (!renderTarget)
    {
        return;
    }

    RhiRenderer::setCurrent(&this->renderer);

    if (this->renderer.getRhi() && this->renderer.getRhi() != this->rhi())
    {
        // The device was replaced: every cached entity buffer belongs to the old
        // one and has to go before the renderer forgets about that device.
        this->view->releaseRenderResources();
    }

    if (!this->renderer.initialize(this->rhi(), renderTarget->renderPassDescriptor(), renderTarget->sampleCount()))
    {
        RLogger::error("RhiRenderSurface: failed to initialize the QRhi renderer\n");
        RhiRenderer::setCurrent(nullptr);
        return;
    }

    if (!this->viewInitialized)
    {
        RLogger::info("QRhi backend: %s\n", this->rhi()->backendName());
        RLogger::info("QRhi device : %s\n", this->rhi()->driverInfo().deviceName.constData());
        this->view->initializeRender();
        this->viewInitialized = true;
    }

    RhiRenderer::setCurrent(nullptr);
}

void RhiRenderSurface::render(QRhiCommandBuffer *cb)
{
    QRhiRenderTarget *renderTarget = this->renderTarget();
    if (!renderTarget || !this->renderer.isValid())
    {
        return;
    }

    const QSize pixelSize = renderTarget->pixelSize();
    if (pixelSize != this->lastSize)
    {
        this->lastSize = pixelSize;
        this->view->resizeRender(this->width(), this->height());
    }

    RhiRenderer::setCurrent(&this->renderer);

    this->renderer.beginFrame(pixelSize);

    // Record phase: the view emits draw items and uploads vertex data.
    this->view->paintRender();

    // Submit phase: one render pass holding every recorded draw item.
    this->renderer.flush(cb, renderTarget, this->view->getClearColor());

    RhiRenderer::setCurrent(nullptr);
}

void RhiRenderSurface::releaseResources()
{
    RhiRenderer::setCurrent(&this->renderer);
    this->view->releaseRenderResources();
    this->renderer.releaseResources();
    RhiRenderer::setCurrent(nullptr);
    this->viewInitialized = false;
    this->lastSize = QSize();
}
