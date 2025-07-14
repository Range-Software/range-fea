#ifndef MODEL_TREE_SIMPLE_H
#define MODEL_TREE_SIMPLE_H

#include <QTreeWidget>

#include "session_entity_id.h"

class ModelTreeSimple : public QTreeWidget
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit ModelTreeSimple(uint modelID, REntityGroupTypeMask modelEntitiesMask, QWidget *parent = nullptr);

        //! Select entity.
        void selectEntity(uint modelID, REntityGroupType entityGroupType, uint entityID);

        //! Get list of selected entity IDs.
        QList<SessionEntityID> getSelected() const;

    private:

        //! Add model to the tree.
        void addModel(uint modelID, REntityGroupTypeMask modelEntitiesMask);

    signals:

    protected slots:

        //! Model selection has canged.
        void onSelectionChanged();

};

#endif // MODEL_TREE_SIMPLE_H
