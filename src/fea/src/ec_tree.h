#ifndef EC_TREE_H
#define EC_TREE_H

#include <QTreeWidget>

#include <rml_environment_condition.h>

class ECTree : public QTreeWidget
{
    Q_OBJECT

    protected:

        //! Environment condition.
        REnvironmentCondition ec;

    public:

        //! Constructor.
        explicit ECTree(QWidget *parent = nullptr);

    protected:

        //! Populate model.
        void populate();

        //! Update selected entities with internal environment condition.
        void updateSelectedEntities() const;

    signals:

    public slots:

        //! Environment condition was selected.
        void onEcSelected(REnvironmentConditionType ecType, bool applied);

    protected slots:

        //! Environment condition value has changed.
        void onEcValueChanged(RVariableType variableType, double value);

};

#endif /* EC_TREE_H */
