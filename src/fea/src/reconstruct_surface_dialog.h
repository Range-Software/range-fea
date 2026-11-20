#ifndef RECONSTRUCT_SURFACE_DIALOG_H
#define RECONSTRUCT_SURFACE_DIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>

#include "session_entity_id.h"
#include "value_line_edit.h"

class ReconstructSurfaceDialog : public QDialog
{

    Q_OBJECT

    public:

        //! Dialog values.
        struct Values
        {
            double angleTolerance;
            double distanceTolerance;
            double minEdgeLength;
            double sliverAngle;
            bool delaunay;
            bool refine;
            double qualityAngle;
            double maxEdgeLength;
        };

    protected:

        //! Model ID.
        uint modelID;
        //! Selected entity IDs.
        const QList<SessionEntityID> &entityIDs;
        //! Angle tolerance.
        QDoubleSpinBox *angleToleranceSpin;
        //! Distance tolerance.
        ValueLineEdit *distanceToleranceEdit;
        //! Minimum edge length.
        ValueLineEdit *minEdgeLengthEdit;
        //! Sliver angle.
        QDoubleSpinBox *sliverAngleSpin;
        //! Delaunay triangulation.
        QCheckBox *delaunayCheck;
        //! Quality mesh (node insertion).
        QCheckBox *refineCheck;
        //! Quality (minimum) angle.
        QDoubleSpinBox *qualityAngleSpin;
        //! Maximum edge length.
        ValueLineEdit *maxEdgeLengthEdit;

    public:

        //! Constructor.
        explicit ReconstructSurfaceDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    protected:

        //! Find default values for selected surfaces.
        Values findDefaultValues() const;

        //! Set values to widgets.
        void setValues(const Values &values);

        //! Get values from widgets.
        Values getValues() const;

    protected slots:

        //! Reset values to defaults.
        void resetToDefaults();

        //! Quality mesh check box toggled.
        void onRefineToggled(bool checked);

};

#endif // RECONSTRUCT_SURFACE_DIALOG_H
