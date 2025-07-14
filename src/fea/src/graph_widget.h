#ifndef GRAPH_WIDGET_H
#define GRAPH_WIDGET_H

#include <QWidget>

#include "graph_object.h"

class GraphWidget : public QWidget
{

    Q_OBJECT

    protected:

        //! Graph object.
        GraphObject *graphObject;
        double xMin;
        double xMax;
        double yMin;
        double yMax;
        double xMargin;
        double yMargin;
        double pointWidth;
        double lineWidth;
        double barWidth;
        double spacing;
        int titleFontSize;
        int labelFontSize;
        double labelSpace;
        double xAxisSpace;
        double yAxisSpace;
        bool showPointer;
        bool snapPointerX;
        bool snapPointerY;
        bool editLimits;
        QPoint mousePosition;
        //! Paint graph limits.
        bool paintGraphLimits;
        bool paintX0;
        bool paintY0;

    public:

        //! Constructor.
        explicit GraphWidget(GraphObject *graphObject, bool editLimits = false, QWidget *parent = nullptr);

        //! Set show pointer.
        void setShowPointer(bool showPointer, bool snapPointerX, bool snapPointerY);

        //! Set paint graph limits.
        void setPaintGraphLimits(bool paintGraphLimits);

        //! Set paint X = 0 line.
        void setPaintX0(bool paintX0);

        //! Set paint Y = 0 line.
        void setPaintY0(bool paintY0);

    protected:

        //! Paint event.
        void paintEvent(QPaintEvent *event);

        //! Process mouse pressed event.
        void mousePressEvent(QMouseEvent *mouseEvent);

        //! Mouse event.
        void mouseMoveEvent(QMouseEvent *mouseEvent);

        //! Mouse enter event.
        void enterEvent(QEnterEvent *);

        //! Mouse leave event.
        void leaveEvent(QEvent *);

        //! Paint preparations.
        void paintSetup();

        //! Paint points.
        void paintPoints(QPainter &painter, uint dataFieldID);

        //! Paint lines.
        void paintLines(QPainter &painter, uint dataFieldID);

        //! Paint bars.
        void paintBars(QPainter &painter, uint dataFieldID);

        //! Paint title.
        void paintTitle(QPainter &painter);

        //! Paint pointer.
        void paintPointer(QPainter &painter, uint dataFieldID);

        //! Paint limits.
        void paintLimits(QPainter &painter);

        //! Paint X line.
        void paintXLine(QPainter &painter, double value);

        //! Paint Y line.
        void paintYLine(QPainter &painter, double value);

        //! Paint x label.
        void paintXLabel(QPainter &painter);

        //! Paint y label.
        void paintYLabel(QPainter &painter);

        //! Paint x axis.
        void paintXAxis(QPainter &painter);

        //! Paint y axis.
        void paintYAxis(QPainter &painter);

        //! Paint position label.
        void paintPosition(QPainter &painter, uint dataFieldID);

        //! Update limits from mouse position.
        void updateLimits();

        //! Return drawing width.
        double getDrawingWidth() const;

        //! Return drawing height.
        double getDrawingHeight() const;

        //! Get x coordinate.
        double getX(double xValue, double xMin, double xMax) const;

        //! Get y coordinate.
        double getY(double yValue, double yMin, double yMax) const;

        //! Get value for x position.
        double getXValue(int x) const;

        //! Get value for y position.
        double getYValue(int y) const;

    protected slots:

        //! Graph data changed.
        void onGraphDataChanged();
        
};

#endif // GRAPH_WIDGET_H
