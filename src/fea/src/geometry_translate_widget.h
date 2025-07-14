#ifndef GEOMETRY_TRANSLATE_WIDGET_H
#define GEOMETRY_TRANSLATE_WIDGET_H

#include <QWidget>

#include <rbl_r3vector.h>

#include "value_line_edit.h"

class GeometryTranslateWidget : public QWidget
{

    Q_OBJECT

    private:

        static const RR3Vector defaultTranslation;

    protected:

        ValueLineEdit *xDistanceLineEdit;
        ValueLineEdit *yDistanceLineEdit;
        ValueLineEdit *zDistanceLineEdit;

    public:

        //! Constructor.
        explicit GeometryTranslateWidget(const RR3Vector &translation = RR3Vector(), QWidget *parent = nullptr);

    signals:

        //! Translation changed.
        void translationChanged(const RR3Vector &distances);

    protected slots:

        //! Origin clicked.
        void setOrigin();

        //! Center clicked.
        void setCenter();

        //! Reset distances clicked.
        void resetDistances();

        //! Distance value changed.
        void onDistanceValueChanged(double);

};

#endif // GEOMETRY_TRANSLATE_WIDGET_H
