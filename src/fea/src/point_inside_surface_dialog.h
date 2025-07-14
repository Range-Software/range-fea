#ifndef POINT_INSIDE_SURFACE_DIALOG_H
#define POINT_INSIDE_SURFACE_DIALOG_H

#include <QDialog>

#include "session_entity_id.h"
#include "position_widget.h"

class PointInsideSurfaceDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! Selected entity IDs.
        const QList<SessionEntityID> &entityIDs;
        //! Point position widget.
        PositionWidget *position;

    public:

        //! Constructor.
        explicit PointInsideSurfaceDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

};

#endif // POINT_INSIDE_SURFACE_DIALOG_H
