#ifndef SUB_WINDOW_STYLE_H
#define SUB_WINDOW_STYLE_H

#include <QColor>
#include <QProxyStyle>

//! Flat title bar and frame of a model sub window. Moving and resizing remain handled by QMdiSubWindow.
class SubWindowStyle : public QProxyStyle
{

    Q_OBJECT

    private:

        //! Color distinguishing the model.
        QColor modelColor;

    public:

        //! Constructor.
        explicit SubWindowStyle(const QColor &modelColor);

        //! Return pixel metric.
        int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

        //! Return style hint.
        int styleHint(StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const override;

        //! Draw primitive element.
        void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

        //! Draw complex control.
        void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const override;

        //! Return sub control rectangle.
        QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget = nullptr) const override;

        //! Return sub control at given position.
        SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget = nullptr) const override;
};

#endif // SUB_WINDOW_STYLE_H
