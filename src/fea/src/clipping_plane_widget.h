#ifndef CLIPPING_PLANE_WIDGET_H
#define CLIPPING_PLANE_WIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>

class ClippingPlaneWidget : public QWidget
{

    Q_OBJECT

    protected:

        QCheckBox *enabledCheck;
        QLabel *distanceLabel;
        QSlider *distanceSlider;

    public:

        //! Constructor.
        explicit ClippingPlaneWidget(QWidget *parent = nullptr);

    signals:

        //! Distance changed signal.
        void changed(bool enabled, double fraction);

    protected slots:

        //! Check-box state has changed.
        void onStateChanged(Qt::CheckState state);

        //! Slider value has changed.
        void onValueChanged(int value);

};

#endif // CLIPPING_PLANE_WIDGET_H
