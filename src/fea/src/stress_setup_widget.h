#ifndef STRESS_SETUP_WIDGET_H
#define STRESS_SETUP_WIDGET_H

#include <QWidget>
#include <QCheckBox>

#include <rml_stress_setup.h>

#include "value_line_edit.h"

class StressSetupWidget : public QWidget
{
    Q_OBJECT

    protected:

        //! Stress setup.
        RStressSetup stressSetup;
        //! Hilber-Hughes-Taylor alpha.
        ValueLineEdit *lineHhtAlpha;
        //! Lumped mass matrix.
        QCheckBox *checkLumpedMass;

    public:

        //! Constructor.
        explicit StressSetupWidget(const RStressSetup &stressSetup, QWidget *parent = nullptr);

    protected:

        //! Enable widgets which apply to the selected transient type.
        void updateEnabled();

    signals:

        //! Stress setup has changed.
        void changed(const RStressSetup &stressSetup);

    private slots:

        void onTransientTypeChanged(int index);

        void onHhtAlphaChanged(double hhtAlpha);

        void onLumpedMassChanged(Qt::CheckState state);

};

#endif // STRESS_SETUP_WIDGET_H
