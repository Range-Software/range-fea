#ifndef BC_TREE_H
#define BC_TREE_H

#include <QTreeWidget>

#include <rbl_local_direction.h>

#include <rml_boundary_condition.h>

class BCTree : public QTreeWidget
{
    Q_OBJECT

    protected:

        //! Boundary condition.
        RBoundaryCondition bc;

    public:

        //! Constructor.
        explicit BCTree(QWidget *parent = nullptr);

    protected:

        //! Populate model.
        void populate();

        //! Update selected entities with internal boundary condition.
        void updateSelectedEntities() const;

        //! Begin to draw rotation.
        void drawLocalRotationBegin();

        //! End to draw rotation.
        void drawLocalRotationEnd();

        //! Calculate position of selected entity.
        QList<RLocalDirection> findSelectedEntityLocalDirections() const;

    signals:

    public slots:

        //! Boundary condition was selected.
        void onBcSelected(RBoundaryConditionType bcType, bool applied);

    protected slots:

        //! Boundary condition value has changed.
        void onBcValueChanged(RVariableType variableType, double value);

        //! Edit values button was clicked.
        void onButtonValueClicked(int id);

        //! Direction changed.
        void onDirectionChanged(const RR3Vector &direction);

};

#endif /* BC_TREE_H */
