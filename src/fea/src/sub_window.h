#ifndef SUB_WINDOW_H
#define SUB_WINDOW_H

#include <QMdiSubWindow>
#include <QToolBar>

#include "gl_widget.h"

class SubWindow : public QMdiSubWindow
{

    Q_OBJECT

    private:

        //! Create sub window.
        void createSubWindow();

    protected:

        //! Model ID.
        uint modelID;
        //! OpenGL widget.
        GLWidget *glWidget;
        //! Toolbar.
        QToolBar *toolBar;

    public:

        //! Constructor.
        explicit SubWindow(uint modelID, QWidget *parent = nullptr);

        //! Return model ID.
        uint getModelID() const;

        //! Set new model ID.
        void setModelID(uint modelID);

        //! Return pointer to GLWidget.
        GLWidget *getGlWidget() const;

    protected:

        //! Window close event.
        void closeEvent(QCloseEvent *closeEvent);

    signals:

        //! Closed signal.
        void closed(uint modelID);

    protected slots:

        //! Reset to origin.
        void onResetO();

        //! Reset to X plane.
        void onResetX();

        //! Reset to Y plane.
        void onResetY();

        //! Reset to Z plane.
        void onResetZ();

        //! Take screenshot.
        void onScreenshot();

        //! Take screenshot.
        void onSessionScreenshot(uint modelID, const QString &fileName);

        //! Show model edges.
        void onShowModelEdgesToggled(bool checked);

        //! Show errors.
        void onShowErrorsToggled(bool checked);

        //! Back side color check toggled.
        void onBackSideColorChanged(Qt::CheckState state);

        //! Edit preferences.
        void onPreferences();

        //! Toolbar icon size has changed.
        void onToolbarIconSizeChanged(int toolbarIconSize);

};

#endif /* SUB_WINDOW_H */
