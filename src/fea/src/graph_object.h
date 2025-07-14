#ifndef GRAPH_OBJECT_H
#define GRAPH_OBJECT_H

#include <QObject>
#include <QTimer>
#include <QString>

#include "graph_data.h"

class GraphObject : public QObject
{

    Q_OBJECT

    protected:

        //! Graph data.
        GraphData graphData;
        //! Source file read interval in miliseconds.
        //! (0 = off).
        uint sourceReadInterval;
        //! Timer.
        QTimer *sourceReadTimer;

    public:

        //! Constructor.
        explicit GraphObject(QObject *parent = nullptr);

        //! Return const reference to graph data object.
        const GraphData & getData() const;

        //! Return reference to graph data object.
        GraphData & getData();

        //! Set new graph data.
        void setData(const GraphData &graphData);

        //! Emit dataChanged signal.
        void setDataChanged();

        //! Return source read interval.
        uint getSourceReadInterval() const;

        //! Set source read interval.
        void setSourceReadInterval(uint sourceReadInterval);

    protected:

        //! Read data from source.
        virtual void readSource();

    signals:

        //! Graph data has changed.
        void dataChanged();

    public slots:

        //! Read data from source.
        void loadData();

};

#endif // GRAPH_OBJECT_H
