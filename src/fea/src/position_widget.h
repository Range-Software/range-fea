#ifndef POSITION_WIDGET_H
#define POSITION_WIDGET_H

#include <QGroupBox>
#include <QSlider>
#include <QPushButton>

#include <rbl_r3vector.h>

#include "value_line_edit.h"

class PositionWidget : public QGroupBox
{

    Q_OBJECT

    protected:

        //! Is horizontal layout.
        bool horizontalLayout;
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
        ValueLineEdit *xLineEdit;
        //! Y coordinate.
        ValueLineEdit *yLineEdit;
        //! Z coordinate.
        ValueLineEdit *zLineEdit;
        //! Pick button.
        QPushButton *pickButton;
        //! Cancel button.
        QPushButton *cancelButton;
        //! OK button.
        QPushButton *okButton;

    public:

        //! Constructor.
        explicit PositionWidget(const QString &title, const RR3Vector &position, bool horizontalLayout = false, QWidget *parent = nullptr);

        //! Show sliders.
        void showSliders();

        //! Hide sliders.
        void hideSliders();

        //! Show pick button.
        void showPickButton();

        //! Hide pick button.
        void hidePickButton();

        //! Show buttons.
        void showButtons();

        //! Hide buttons.
        void hideButtons();

        //! Set X range.
        void setXRange(double xMin, double xMax);

        //! Set Y range.
        void setYRange(double yMin, double yMax);

        //! Set Z range.
        void setZRange(double zMin, double zMax);

        //! Return position.
        RR3Vector getPosition() const;

        //! Set position.
        void setPosition(const RR3Vector &position);

    private:

        //! Set sliders position.
        void setSlidersPosition(const RR3Vector &position);

        //! Calculate slider positions.
        void findSliderPositions(const RR3Vector &position, int &x, int &y, int &z);

    signals:

        //! Position changed.
        void changed(const RR3Vector &position);

        //! Close signal.
        void closed();

    private slots:

        //! X slider value changed.
        void onXSliderValueChanged(int value);

        //! Y slider value changed.
        void onYSliderValueChanged(int value);

        //! Z slider value changed.
        void onZSliderValueChanged(int value);

        //! X line edit value changed.
        void onXLineEditValueChanged(double);

        //! Y line edit value changed.
        void onYLineEditValueChanged(double);

        //! Z line edit value changed.
        void onZLineEditValueChanged(double);

        //! On pick button clicked.
        void onPickButtonClicked();

        //! On ok button clicked.
        void onOkButtonClicked();

        //! On cancel button clicked.
        void onCancelButtonClicked();

        //! Session pick list has changed.
        void onPickListChanged();

};

#endif // POSITION_WIDGET_H
