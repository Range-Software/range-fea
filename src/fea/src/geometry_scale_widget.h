#ifndef GEOMETRY_SCALE_WIDGET_H
#define GEOMETRY_SCALE_WIDGET_H

#include <QGroupBox>
#include <QWidget>

#include <rbl_r3vector.h>

#include "position_widget.h"
#include "value_line_edit.h"

class GeometryScaleWidget : public QWidget
{

    Q_OBJECT

    private:

        RR3Vector defaultScale;

    protected:

        QGroupBox *sameScaleGroupBox;
        QGroupBox *differentScaleGroupBox;
        ValueLineEdit *sameScaleLineEdit;
        ValueLineEdit *xScaleLineEdit;
        ValueLineEdit *yScaleLineEdit;
        ValueLineEdit *zScaleLineEdit;
        PositionWidget *scaleCenter;

    public:

        //! Constructor.
        explicit GeometryScaleWidget(const RR3Vector &center = RR3Vector(), const RR3Vector &scale = RR3Vector(1.0,1.0,1.0), QWidget *parent = nullptr);

        //! Destructor.
        ~GeometryScaleWidget();

    signals:

        //! Scale changed.
        void scaleChanged(const RR3Vector &center, const RR3Vector &scales);

    protected slots:

        //! Same scale was toggled.
        void onSameScaleToggled(bool checked);

        //! Different scale was toggled.
        void onDifferentScaleToggled(bool checked);

        //! Same scale reset clicked.
        void resetSameScale();

        //! Different scale reset clicked.
        void resetDifferentScale();

        //! Scale value changed.
        void onScaleValueChanged(double);

        //! Position changed.
        void onPositionChanged(const RR3Vector &);

};

#endif // GEOMETRY_SCALE_WIDGET_H
