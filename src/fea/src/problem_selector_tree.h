#ifndef PROBLEM_SELECTOR_TREE_H
#define PROBLEM_SELECTOR_TREE_H

#include <rml_problem_type.h>

#include <QTreeWidget>

class ProblemSelectorTree : public QTreeWidget
{

    Q_OBJECT

    protected:

        //! Problem tree widget.
        QTreeWidget *problemTree;

    public:

        //! Constructor.
        explicit ProblemSelectorTree(QWidget *parent = nullptr);

        //! Return problem mask for model id.
        RProblemTypeMask findProblemTypeMask();

        //! Check if some problem type is selected.
        bool checkProblemIsChecked() const;

    protected:

        //! Check checked item.
        void checkCheckedTreeWidgetItem(QTreeWidgetItem *item);

        //! Check unchecked item.
        void checkUncheckedTreeWidgetItem(QTreeWidgetItem *item);

    protected slots:

        //! Tree widget item has changed.
        void onTreeWidgetItemChanged(QTreeWidgetItem *item);

    signals:

        //! Widget changes signal.
        void changed();

};

#endif // PROBLEM_SELECTOR_TREE_H
