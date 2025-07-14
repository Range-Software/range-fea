#ifndef IC_TREE_H
#define IC_TREE_H

#include <QTreeWidget>

#include <rml_initial_condition.h>

class ICTree : public QTreeWidget
{
    Q_OBJECT

    protected:

        //! Initial condition.
        RInitialCondition ic;

    public:

        //! Constructor.
        explicit ICTree(QWidget *parent = nullptr);

    protected:

        //! Populate model.
        void populate();

        //! Update selected entities with internal initial condition.
        void updateSelectedEntities() const;

    signals:

    public slots:

        //! Initial condition was selected.
        void onIcSelected(RInitialConditionType icType, bool applied);

    protected slots:

        //! Initial condition value has changed.
        void onIcValueChanged(RVariableType variableType, double value);

};

#endif /* IC_TREE_H */
