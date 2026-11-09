#ifndef VALUE_SCALE_H
#define VALUE_SCALE_H

#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QSpinBox>

#include "value_line_edit.h"

//! Single row for entering a value, with optional units and logarithmic slider.
//! Slider for the mantissa and spin box for the power of ten can be shown under the row.
class ValueScale : public QWidget
{

    Q_OBJECT

    protected:

        static const double mantisScale;
        static const uint mantisSize;
        static const uint exponentSize;

        //! Mantis.
        double mantis;
        //! Exponent.
        int exponent;

        //! Title label.
        QLabel *titleLabel;
        //! Scale edit.
        ValueLineEdit *scaleEdit;
        //! Units label.
        QLabel *unitsLabel;
        //! Logarithmic slider.
        QSlider *logSlider;
        //! Widget with fine adjustment.
        QWidget *fineAdjustWidget;
        //! Mantis slider.
        QSlider *mantisSlider;
        //! Mantis edit.
        ValueLineEdit *mantisEdit;
        //! Exponent spin box.
        QSpinBox *exponentSpinBox;

        //! Setting scale allowed.
        bool setScaleAllowed;
        //! Logarithmic slider is the source of the current change.
        bool logSliderChanging;

    public:

        //! Constructor. Empty title shows no label.
        explicit ValueScale(const QString &title, QWidget *parent = nullptr);

        //! Return scale;
        double getValue() const;

        //! Set value.
        void setValue(double value);

        //! Set text shown after the value. Empty text shows no label.
        void setUnits(const QString &units);

        //! Show logarithmic slider after the value.
        void setLogSliderVisible(bool visible);

        //! Show or hide fine adjustment under the value.
        void setFineAdjustVisible(bool visible);

        //! Return width needed by the title.
        int findTitleWidth() const;

        //! Set width of the title, so that values of several rows are aligned.
        void setTitleWidth(int width);

    private:

        //! Move logarithmic slider to given value.
        void syncLogSlider(double value);

    signals:

        //! Value has changed.
        void valueChanged(double value);

    private slots:

        //! Scale edit changed.
        void onScaleEditChanged(double value);

        //! Logarithmic slider changed.
        void onLogSliderChanged(int position);

        //! Mantis slider changed.
        void onMantisSliderChanged(int value);

        //! Mantis edit changed.
        void onMantisEditChanged(double value);

        //! Exponent spin box changed.
        void onExponentSpinBoxChanged(int value);

};

#endif // VALUE_SCALE_H
