#ifndef BOOL_UNION_DIALOG_H
#define BOOL_UNION_DIALOG_H

#include <QDialog>

#include "session_entity_id.h"

class BoolUnionDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! Selected entity IDs.
        const QList<SessionEntityID> &entityIDs;

    public:

        //! Constructor.
        explicit BoolUnionDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

};

#endif // BOOL_UNION_DIALOG_H
