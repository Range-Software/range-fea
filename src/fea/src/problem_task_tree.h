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

    protected:

        //! Add item to tree widget.
        void addTaskItemToWidget(QTreeWidgetItem *parent, const RProblemTaskItem &taskItem);

        //! Remove item.
        void removeItem(QTreeWidgetItem *item);

        //! Add widget item to task tree.
        static void addWidgetItemToTree(RProblemTaskItem &taskItem, const QTreeWidgetItem *item);

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

        //! Add problem type button clicked.
        void onAddProblemButtonClicked();

    signals:

        //! Tree changed signal.
        void changed();

};

#endif // PROBLEM_TASK_TREE_H
