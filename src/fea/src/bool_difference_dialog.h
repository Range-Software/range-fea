#ifndef BOOL_DIFFERENCE_DIALOG_H
#define BOOL_DIFFERENCE_DIALOG_H

#include <QDialog>
#include <QListWidget>

#include "session_entity_id.h"

class BoolDifferenceDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! List of entity IDs where last one is cutting surface.
        QList<SessionEntityID> entityIDs;
        //! Entity list.
        QListWidget *listWidget;

    public:

        //! Constructor.
        explicit BoolDifferenceDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

};

#endif // BOOL_DIFFERENCE_DIALOG_H
