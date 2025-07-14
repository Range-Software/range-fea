#ifndef CONVERGENCE_GRAPH_WIDGET_H
#define CONVERGENCE_GRAPH_WIDGET_H

#include <QWidget>
#include <QComboBox>

#include "convergence_graph_object.h"

class ConvergenceGraphWidget : public QWidget
{
    Q_OBJECT

    protected:

        //! Data column combo box.
        QComboBox *dataColumnCombo;
        //! Graph object.
        ConvergenceGraphObject *graphObject;

    public:

        //! Constructor.
        explicit ConvergenceGraphWidget(const QString &convergenceFileName, QWidget *parent = nullptr);

    signals:

    public slots:

        //! Data column combo current index changed.
        void onCurrentIndexChanged(int index);
        //! Clear button was clicked.
        void onClearButtonClicked();

};

#endif // CONVERGENCE_GRAPH_WIDGET_H
