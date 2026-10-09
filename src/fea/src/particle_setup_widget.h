#ifndef PARTICLE_SETUP_WIDGET_H
#define PARTICLE_SETUP_WIDGET_H

#include <QWidget>

#include <rml_particle_setup.h>

class ParticleSetupWidget : public QWidget
{
    Q_OBJECT

    protected:

        //! Particle setup.
        RParticleSetup particleSetup;

    public:

        //! Constructor.
        explicit ParticleSetupWidget(const RParticleSetup &particleSetup, QWidget *parent = nullptr);

    signals:

        //! Particle setup has changed.
        void changed(const RParticleSetup &particleSetup);

    private slots:

        void onMaximumSaturationChanged(double maximumSaturation);

        void onDiffusionCoefficientChanged(double diffusionCoefficient);

};

#endif // PARTICLE_SETUP_WIDGET_H
