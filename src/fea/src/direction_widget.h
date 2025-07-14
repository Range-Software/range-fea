#ifndef DIRECTION_WIDGET_H
#define DIRECTION_WIDGET_H

#include <QGroupBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>

#include <rbl_r3vector.h>

class DirectionWidget : public QGroupBox
{

    Q_OBJECT

    protected:

        //! Show button box.
        //! If set to true changed signal will be emitted only if OK button is clicked.
        bool showButtonBox;
        //! X slider.
        QSlider *xSlider;
        //! Y slider.
        QSlider *ySlider;
        //! Z slider.
        QSlider *zSlider;
        //! X coordinate.
        QDoubleSpinBox *xSpin;
        //! Y coordinate.
        QDoubleSpinBox *ySpin;
        //! Z coordinate.
        QDoubleSpinBox *zSpin;
        //! Cancel button.
        QPushButton *cancelButton;
        //! OK button.
        QPushButton *okButton;

    public:

        //! Constructor.
        explicit DirectionWidget(const QString &title, const RR3Vector &direction, QWidget *parent = nullptr);

        //! Show buttons.
        void showButtons();

        //! Hide buttons.
        void hideButtons();

        //! Return direction.
        RR3Vector getDirection() const;

        //! Set direction.
        void setDirection(const RR3Vector &direction);

    private:

        //! Set plane normal.
        void normalize(RR3Vector &direction, int keepPosition);

        //! Calculate slider positions.
        void findSliderPositions(const RR3Vector &direction, int &x, int &y, int &z);

    signals:

        //! Direction changed.
        void changed(const RR3Vector &direction);

        //! Close signal.
        void closed();

    private slots:

        //! X slider value changed.
        void onXSliderValueChanged(int value);

        //! Y slider value changed.
        void onYSliderValueChanged(int value);

        //! Z slider value changed.
        void onZSliderValueChanged(int value);

        //! X spin box value changed.
        void onXSpinValueChanged(double);

        //! Y spin box value changed.
        void onYSpinValueChanged(double);

        //! Z spin box value changed.
        void onZSpinValueChanged(double);

        //! On ok button clicked.
        void onOkButtonClicked();

        //! On cancel button clicked.
        void onCancelButtonClicked();

};

#endif // DIRECTION_WIDGET_H
