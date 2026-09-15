#ifndef RHI_RENDER_SURFACE_H
#define RHI_RENDER_SURFACE_H

#include <QRhiWidget>

#include "render_surface.h"
#include "rhi_renderer.h"

class GLWidget;

//! QRhi render surface.
//! Drives the two phase RhiRenderer: the hosting GLWidget records draw items
//! during render(), which are then submitted in a single render pass.
class RhiRenderSurface : public QRhiWidget, public RenderSurface
{

    Q_OBJECT

    protected:

        //! Hosting view.
        GLWidget *view;
        //! Renderer owning the QRhi resources of this surface.
        RhiRenderer renderer;
        //! Whether the hosting view has been told about the first frame.
        bool viewInitialized;
        //! Last size the view was told about.
        QSize lastSize;

    public:

        //! Constructor.
        explicit RhiRenderSurface(GLWidget *view, QWidget *parent = nullptr);

        //! Return the surface as a QWidget.
        QWidget *asWidget() override;

        //! Schedule a repaint of the surface.
        void requestUpdate() override;

        //! Grab the last rendered frame.
        QImage grabImage() override;

        //! Return the renderer owning the QRhi resources.
        RhiRenderer &getRenderer();

        //! Map a RenderBackend::RhiApi value to the QRhiWidget API enum.
        static QRhiWidget::Api resolveApi();

    protected:

        //! Create or recreate the device level resources.
        void initialize(QRhiCommandBuffer *cb) override;

        //! Record and submit one frame.
        void render(QRhiCommandBuffer *cb) override;

        //! Destroy the device level resources.
        void releaseResources() override;

};

#endif // RHI_RENDER_SURFACE_H
