#ifndef PROBLEM_TASK_TREE_H
#define PROBLEM_TASK_TREE_H

#include <QTreeWidget>
#include <QPushButton>

#include <rml_model.h>

class ProblemTaskTree : public QWidget
{

    Q_OBJECT

    protected:

        enum ColumnType
        {
            C_NAME = 0,
            C_VALUE,
            C_CVG_NAME,
            C_CVG_VALUE,
            N_COLUMNS
        };

        //! Tree widget.
        QTreeWidget *treeWidget;
        //! Up button.
        QPushButton *upButton;
        //! Down button.
        QPushButton *downButton;
        //! Indent button.
        QPushButton *indentButton;
        //! Unindent button.
        QPushButton *unindentButton;
        //! Delete button.
        QPushButton *deleteButton;

    public:

        //! Constructor.
        explicit ProblemTaskTree(const RProblemTaskItem &rTaskTree, QWidget *parent = nullptr);

        //! Return problem task tree.
        RProblemTaskItem getProblemTaskTree() const;

        //! Replace content with given problem task tree.
        void setProblemTaskTree(const RProblemTaskItem &rTaskTree);

        //! Add given problem types not yet present, each before the tasks which use its results.
        void addProblemTypes(RProblemTypeMask problemTypeMask);

        //! Remove all tasks of given problem types.
        void removeProblemTypes(RProblemTypeMask problemTypeMask);

    protected:

        //! Return problem types of given item and of all items below it.
        static RProblemTypeMask findItemTypeMask(const QTreeWidgetItem *item);

        //! Remove tasks whose required problem types are no longer present.
        void removeTasksWithoutRequiredTypes();

        //! Add item to tree widget.
        void addTaskItemToWidget(QTreeWidgetItem *parent, const RProblemTaskItem &taskItem);

        //! Remove item.
        void removeItem(QTreeWidgetItem *item);

        //! Add widget item to task tree.
        static void addWidgetItemToTree(RProblemTaskItem &taskItem, const QTreeWidgetItem *item);

        //! Resize every column to its contents.
        void resizeColumns();

    protected slots:

        //! Handle selection changed signal.
        void onItemSelectionChanged();

        //! Handle changed signal.
        void onItemChanged(QTreeWidgetItem *item, int column);

        //! Handle double click signal.
        void onItemDoubleClicked(QTreeWidgetItem *item, int column);

        //! Up button clicked.
        void onUpButtonClicked();

        //! Down button clicked.
        void onDownButtonClicked();

        //! Indent button clicked.
        void onIndentButtonClicked();

        //! Unindent button clicked.
        void onUnindentButtonClicked();

        //! Delete button clicked.
        void onDeleteButtonClicked();

    signals:

        //! Tree changed signal.
        void changed();

};

#endif // PROBLEM_TASK_TREE_H
