#ifndef EC_MANAGER_TREE_H
#define EC_MANAGER_TREE_H

#include <QTreeWidget>

#include <rml_environment_condition.h>

#include "session_entity_id.h"

class ECManagerTree : public QTreeWidget
{
    Q_OBJECT

    private:

        //! List of selected types.
        //! Used only for reselecting already selected items
        //! when model selection has changed.
        QList<REnvironmentConditionType> selectedTypes;
        //! If set to true populat function will return immediately.
        bool doNotPopulate;

    public:

        //! Constructor.
        explicit ECManagerTree(QObject *parent = nullptr);

    protected:

        //! Populate tree.
        void populate();

        //! Return map of applied environment conditions to number of occurences based on model ID list.
        QMap<REnvironmentConditionType,uint> getEcMap(const QList<SessionEntityID> &entityIDs) const;

    signals:

        //! Environment condition was selected.
        void ecSelected(REnvironmentConditionType type, bool applied);

    protected slots:

        //! Called on entity selection changed.
        void onModelSelectionChanged(uint);

        //! Called on problem mask changed.
        void onProblemChanged(uint);

        //! Called when data has been changed.
        void onItemChanged(QTreeWidgetItem *item, int column);

        //! Called when item selection has changed.
        void onItemSelectionChanged();

        //! Called when ecSelected signal is emitted.
        void onEcSelected();

};

#endif /* EC_MANAGER_TREE_H */
