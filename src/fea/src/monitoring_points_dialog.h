#ifndef MONITORING_POINTS_DIALOG_H
#define MONITORING_POINTS_DIALOG_H

#include <QDialog>
#include <QTableWidget>

#include <rml_monitoring_point.h>

class MonitoringPointsDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! Table widget containing monitoring points.
        QTableWidget *tableWidget;
        //! Remove button.
        QPushButton *removeButton;
        //! Use picked coordinates button.
        QPushButton *usePickedButton;

    public:

        //! Construct.
        explicit MonitoringPointsDialog(uint modelID, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    protected:

        //! Add new monitoring point.
        void addMonitoringPoint(const RMonitoringPoint &monitoringPoint);

        //! Return list of selected rows.
        QList<int> findSelectedRows() const;

        //! Find picked coordinates.
        RR3Vector findPickedPosition() const;

    protected slots:

        //! Item selection has changed.
        void onItemSelectionChanged();

        //! Add button was clicked.
        void onAddClicked();

        //! Remove button was clicked.
        void onRemoveClicked();

        //! Use picked button was clicked.
        void onUsePickedClicked();

};

#endif // MONITORING_POINTS_DIALOG_H
