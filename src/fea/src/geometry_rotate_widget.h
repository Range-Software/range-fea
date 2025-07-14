#ifndef GEOMETRY_ROTATE_WIDGET_H
#define GEOMETRY_ROTATE_WIDGET_H

#include <QWidget>

#include <rbl_r3vector.h>

#include "position_widget.h"
#include "value_line_edit.h"

class GeometryRotateWidget : public QWidget
{

    Q_OBJECT

    private:

        static const RR3Vector defaultAngles;

    protected:

        PositionWidget *rotationCenter;
        ValueLineEdit *xAngleLineEdit;
        ValueLineEdit *yAngleLineEdit;
        ValueLineEdit *zAngleLineEdit;

    public:

        //! Constructor.
        explicit GeometryRotateWidget(const RR3Vector &center = RR3Vector(), const RR3Vector &rotation = RR3Vector(), QWidget *parent = nullptr);

        //! Destructor.
        ~GeometryRotateWidget();

    signals:

        //! Rotation changed.
        void rotationChanged(const RR3Vector &center, const RR3Vector &angles);

    protected slots:

        //! Reset angles clicked.
        void resetAngles();

        //! Angle value changed.
        void onAngleValueChanged(double);

        //! Position changed.
        void onPositionChanged(const RR3Vector &);

};

#endif // GEOMETRY_ROTATE_WIDGET_H
