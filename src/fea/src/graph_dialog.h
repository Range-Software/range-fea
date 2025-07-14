#ifndef GRAPH_DIALOG_H
#define GRAPH_DIALOG_H

#include <QDialog>

#include "graph_object.h"
#include "graph_widget.h"

class GraphDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Graph widget.
        GraphWidget *graphWidget;

    public:

        //! Constructor.
        explicit GraphDialog(GraphObject *graphObject, QWidget *parent = nullptr);

        //! Return pointer to graph widget.
        GraphWidget * getGraphWidget();
        
};

#endif // GRAPH_DIALOG_H
