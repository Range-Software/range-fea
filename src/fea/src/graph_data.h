#ifndef GRAPH_DATA_H
#define GRAPH_DATA_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>

#include <rbl_rvector.h>

typedef enum _GraphType {
    GRAPH_NONE   = 1 << 0,
    GRAPH_LINES  = 1 << 1,
    GRAPH_POINTS = 1 << 2,
    GRAPH_BARS   = 1 << 3
} GraphType;

typedef int GraphTypeMask;

class GraphData : public QMap<double,RRVector>
{

    protected:

        //! Graph type mask.
        GraphTypeMask typeMask;
        //! Graph title.
        QString title;
        //! Graph x axis label.
        QString xlabel;
        //! Graph y axis label.
        QString ylabel;
        //! Graph lower limit.
        double lLimit;
        //! Graph upper limit.
        double uLimit;

    private:

        //! Internal initialization function.
        void _init(const GraphData *pGraphData = nullptr);

    public:

        //! Constructor.
        GraphData();

        //! Copy constructor.
        GraphData(const GraphData &graphData);

        //! Assignment operator.
        GraphData & operator =(const GraphData &graphData);

        //! Return graph type mask.
        GraphTypeMask getTypeMask() const;

        //! Set graph type mask.
        void setTypeMask(GraphTypeMask typeMask);

        //! Return title.
        const QString & getTitle() const;

        //! Set title.
        void setTitle(const QString &title);

        //! Return x label.
        const QString & getXLabel() const;

        //! Set x label.
        void setXLabel(const QString &xlabel);

        //! Return y label.
        const QString & getYLabel() const;

        //! Set y label.
        void setYLabel(const QString &ylabel);

        //! Return lower limit.
        double getLLimit() const;

        //! Return upper limit.
        double getULimit() const;

        //! Set limits.
        void setLimits(double lLimit, double uLimit);

        //! Find min x value.
        double findXMin() const;

        //! Find max x value.
        double findXMax() const;

        //! Find min y value.
        double findYMin() const;

        //! Find max y value.
        double findYMax() const;

        //! Round key to neares value.
        double roundKey(double x) const;

        //! Return y value for a given x.
        //! If such key is not fund an interpolated value is returned.
        RRVector findValue(double x) const;

        //! Get nuber of data fields.
        uint getNDataFields() const;

};

#endif // GRAPH_DATA_H
