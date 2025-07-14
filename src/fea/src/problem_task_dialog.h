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

    protected slots:

        //! Handle close event.
        void closeEvent(QCloseEvent *e);

        //! Handle key press event.
        void keyPressEvent(QKeyEvent *e);

        //! handle ProblemtaskTree::changed() signal.
        void onProblemTaskTreeChanged();

};

#endif // PROBLEM_TASK_DIALOG_H
