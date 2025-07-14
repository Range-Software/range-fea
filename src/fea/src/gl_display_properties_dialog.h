#ifndef GL_DISPLAY_PROPERTIES_DIALOG_H
#define GL_DISPLAY_PROPERTIES_DIALOG_H

#include <QDialog>
#include <QTreeWidget>

#include "gl_display_properties.h"

class GLWidget;

class GLDisplayPropertiesDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! OpenGL Widget.
        GLWidget *pGlWidget;

        //! OpenGL display properties.
        GLDisplayProperties backupDisplayProperties;

        //! Property tree.
        QTreeWidget *propertyTree;

    public:

        //! Constructor
        explicit GLDisplayPropertiesDialog(GLWidget *pGlWidget, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    private:

        //! Populate property tree.
        void populate();

    private slots:

        //! Show model edges clicked.
        void onShowModelEdgesClicked(bool checked);

        //! Show model dimensions clicked.
        void onShowModelDimensionsClicked(bool checked);

        //! Show model grid clicked.
        void onShowModelGridClicked(bool checked);

        //! Show model errors clicked.
        void onShowModelErrorsClicked(bool checked);

        //! Show global axis clicked.
        void onShowGlobalAxisClicked(bool checked);

        //! Show local axis clicked.
        void onShowLocalAxisClicked(bool checked);

        //! Background color has changed.
        void onBgColorChanged(const QColor &color);

        //! Background gradient checkbox has changed.
        void onBgGradientStateChanged(Qt::CheckState state);

        //! Light has changed.
        void onLightChanged(const RGLLight &light, uint id);

        //! Default button was clicked.
        void onDefaultClicked();

};

#endif /* GL_DISPLAY_PROPERTIES_DIALOG_H */
