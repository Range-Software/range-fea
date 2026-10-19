#ifndef PROBLEM_TASK_DIALOG_H
#define PROBLEM_TASK_DIALOG_H

#include <QDialog>

#include "problem_task_tree.h"

class ProblemTaskDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! Problem task tree widget.
        ProblemTaskTree *taskTree;
        //! OK button.
        QPushButton *okButton;
        //! Cancel button.
        QPushButton *cancelButton;

    public:

        //! Constructor.
        explicit ProblemTaskDialog(uint modelID, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    private:

        //! Ask user whether results of removed problems should be cleared and clear them.
        //! Return true if any results were cleared.
        bool clearResults(RProblemTypeMask removedTypeMask, RProblemTypeMask remainingTypeMask);

    protected slots:

        //! Handle close event.
        void closeEvent(QCloseEvent *e);

        //! Handle key press event.
        void keyPressEvent(QKeyEvent *e);

};

#endif // PROBLEM_TASK_DIALOG_H
