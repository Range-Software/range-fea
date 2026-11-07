#ifndef PROBLEM_SELECTOR_TREE_H
#define PROBLEM_SELECTOR_TREE_H

#include <vector>

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

        //! Return listed problem types in order in which they are listed.
        std::vector<RProblemType> getProblemTypes() const;

        //! Return true if given problem type is checked.
        bool isProblemTypeChecked(RProblemType problemType) const;

        //! Check or uncheck given problem type. Rules for required and excluded problem types are applied.
        void setProblemTypeChecked(RProblemType problemType, bool checked);

    protected:

        //! Sort problem types so that each is listed directly after the problem types it depends on.
        static std::vector<RProblemType> sortTypesByDependency(const std::vector<RProblemType> &problemTypes);

        //! Append problem type followed by its dependent problem types.
        static void appendTypeWithDependents(const std::vector<RProblemType> &problemTypes, RProblemType problemType, std::vector<RProblemType> &sortedTypes, RProblemTypeMask &listedMask);

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
