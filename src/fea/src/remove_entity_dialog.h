#ifndef REMOVE_ENTITY_DIALOG_H
#define REMOVE_ENTITY_DIALOG_H

#include <QDialog>

#include <rml_entity_group.h>

#include "session_entity_id.h"

class RemoveEntityDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Entity type.
        REntityGroupTypeMask entityTypeMask;

    public:

        //! Constructor.
        explicit RemoveEntityDialog(REntityGroupTypeMask entityTypeMask, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    private:

        //! Return list of entities for given model ID.
        QList<SessionEntityID> findEntities(uint mid) const;

};

#endif // REMOVE_ENTITY_DIALOG_H
