#ifndef SUB_WINDOW_H
#define SUB_WINDOW_H

#include <QMdiSubWindow>
#include <QToolBar>

#include "gl_widget.h"

class SubWindowStyle;

class SubWindow : public QMdiSubWindow
{

    Q_OBJECT

    private:

        //! Create sub window.
        void createSubWindow();

        //! Highlight window if its model is selected in model tree.
        void updateHighlight();

    protected:

        //! Model ID.
        uint modelID;
        //! OpenGL widget.
        GLWidget *glWidget;
        //! Toolbar.
        QToolBar *toolBar;
        //! Style of the window.
        SubWindowStyle *subWindowStyle;

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

        //! Model selection has changed.
        void onModelSelectionChanged(uint modelID);

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

        //! Highlight selection check toggled.
        void onHighlightSelectedChanged(Qt::CheckState state);

        //! Back side color check toggled.
        void onTwoSideFaceChanged(Qt::CheckState state);

        //! Edit preferences.
        void onPreferences();

        //! Toolbar icon size has changed.
        void onToolbarIconSizeChanged(int toolbarIconSize);

};

#endif /* SUB_WINDOW_H */
