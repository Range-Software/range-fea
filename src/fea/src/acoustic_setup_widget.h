#ifndef ACOUSTIC_SETUP_WIDGET_H
#define ACOUSTIC_SETUP_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSpinBox>

#include <rml_acoustic_setup.h>

#include "value_line_edit.h"

class AcousticSetupWidget : public QWidget
{
    Q_OBJECT

    protected:

        //! Acoustic setup.
        RAcousticSetup acousticSetup;
        //! First frequency of the sweep.
        ValueLineEdit *lineFrequencyStart;
        //! Frequency increment of the sweep.
        ValueLineEdit *lineFrequencyStep;
        //! Number of frequencies in the sweep.
        QSpinBox *spinNFrequencies;
        //! Label showing the resulting frequency range.
        QLabel *labelFrequencyRange;

    public:

        //! Constructor.
        explicit AcousticSetupWidget(const RAcousticSetup &acousticSetup, QWidget *parent = nullptr);

    signals:

        //! Acoustic setup has changed.
        void changed(const RAcousticSetup &acousticSetup);

    protected:

        //! Enable / disable the harmonic sweep controls.
        void updateHarmonicControls();

    private slots:

        void onAnalysisTypeChanged(int index);

        void onFrequencyStartChanged(double frequencyStart);

        void onFrequencyStepChanged(double frequencyStep);

        void onNFrequenciesChanged(int nFrequencies);

        void onReferencePressureChanged(double referencePressure);

};

#endif // ACOUSTIC_SETUP_WIDGET_H
