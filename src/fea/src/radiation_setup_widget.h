#ifndef RADIATION_SETUP_WIDGET_H
#define RADIATION_SETUP_WIDGET_H

#include <QWidget>

#include <rml_radiation_setup.h>

#include <rgl_file_chooser_button.h>

class RadiationSetupWidget : public QWidget
{
    Q_OBJECT

    protected:

        //! Radiation setup.
        RRadiationSetup radiationSetup;
        //! Default view-factor filename.
        QString defaultViewFactorFileName;
        //! View-factor file chooser button.
        RFileChooserButton *viewFactorFileButton;

    public:

        //! Constructor.
        explicit RadiationSetupWidget(const RRadiationSetup &radiationSetup, const QString &defaultViewFactorFileName, QWidget *parent = nullptr);

    signals:

        //! Radiation setup has changed.
        void changed(const RRadiationSetup &radiationSetup);

    protected slots:

        //! Radiation resolution changed.
        void onResolutionChanged(int index);

        //! Radiation custom view-factor file toggled.
        void onCustomViewFactorFileToggled(bool checked);

        //! Radiation view-factor file changed.
        void onViewFactorFileChanged(const QString &fileName);

        //! Regenerate patches.
        void regeneratePatches();

        //! Recalculate view-factors.
        void recalculateViewFactors();

        //! Clear view-factor mtrix.
        void clearViewFactorMatrix();

};

#endif // RADIATION_SETUP_WIDGET_H
