#ifndef BOOL_INTERSECTION_DIALOG_H
#define BOOL_INTERSECTION_DIALOG_H

#include <QDialog>

#include "session_entity_id.h"

class BoolIntersectionDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! Selected entity IDs.
        const QList<SessionEntityID> &entityIDs;

    public:

        //! Constructor.
        explicit BoolIntersectionDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    signals:

    public slots:

};

#endif // BOOL_INTERSECTION_DIALOG_H
