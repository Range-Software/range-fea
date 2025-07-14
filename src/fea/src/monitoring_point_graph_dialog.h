#ifndef MONITORING_POINT_GRAPH_DIALOG_H
#define MONITORING_POINT_GRAPH_DIALOG_H

#include <QDialog>
#include <QMap>
#include <QComboBox>
#include <QTreeWidget>

#include <rml_monitoring_point.h>

class MonitoringPointGraphDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Monitoring points agregated structure.
        QMap< RMonitoringPoint,QVector<RRVector> > data;
        //! Monitoring points combo box.
        QComboBox *monitoringPointsCombo;
        //! Monitoring point values tree widget.
        QTreeWidget *valuesTree;

    public:

        //! Constructor.
        explicit MonitoringPointGraphDialog(const QString &monitoringFileName, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    protected:

        //! Read data from file.
        void readDataFromFile(const QString &fileName);

        //! Populate values tree widget.
        void populateValuesTree();

        //! Dislpay graph dialog.
        void displayInGraph();

    protected slots:

        //! Monitoring point combo current index changed.
        void onCurrentIndexChanged(int);

        //! Display values in graph button clicked.
        void onDisplayInGraphClicked();

};

#endif // MONITORING_POINT_GRAPH_DIALOG_H
