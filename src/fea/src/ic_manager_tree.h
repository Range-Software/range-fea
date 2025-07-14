#ifndef IC_MANAGER_TREE_H
#define IC_MANAGER_TREE_H

#include <QTreeWidget>

#include <rml_initial_condition.h>

#include "session_entity_id.h"

class ICManagerTree : public QTreeWidget
{
    Q_OBJECT

    private:

        //! List of selected types.
        //! Used only for reselecting already selected items
        //! when model selection has changed.
        QList<RInitialConditionType> selectedTypes;
        //! If set to true populat function will return immediately.
        bool doNotPopulate;

    public:

        //! Constructor.
        explicit ICManagerTree(QObject *parent = nullptr);

    protected:

        //! Populate tree.
        void populate();

        //! Return map of applied initial conditions to number of occurences based on model ID list.
        QMap<RInitialConditionType,uint> getIcMap(const QList<SessionEntityID> &entityIDs) const;

    signals:

        //! Initial condition was selected.
        void icSelected(RInitialConditionType type, bool applied);

    protected slots:

        //! Called on entity selection changed.
        void onModelSelectionChanged(uint);

        //! Called on problem mask changed.
        void onProblemChanged(uint);

        //! Called when data has been changed.
        void onItemChanged(QTreeWidgetItem *item, int column);

        //! Called when item selection has changed.
        void onItemSelectionChanged();

        //! Called when icSelected signal is emitted.
        void onIcSelected();

};

#endif /* IC_MANAGER_TREE_H */
