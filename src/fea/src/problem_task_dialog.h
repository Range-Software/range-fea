#ifndef PROBLEM_TASK_DIALOG_H
#define PROBLEM_TASK_DIALOG_H

#include <QDialog>

#include "problem_task_tree.h"

class QFormLayout;
class QLabel;
class QLineEdit;
class QSpinBox;
class QToolButton;
class ProblemSelectorTiles;
class ProblemSelectorTree;

//! Select what should be solved for a model.
//! The task flow is built automatically from selected problem types, unless the user customizes it.
class ProblemTaskDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! Problem types of the model when the dialog was opened.
        RProblemTypeMask initialTypeMask;
        //! Currently selected problem types.
        RProblemTypeMask selectedTypeMask;
        //! True if task flow is customized by the user.
        bool customFlow;
        //! Selection is being updated from the customized task flow.
        bool syncingSelection;
        //! Last layout state, used to resize the dialog when its content changes.
        int layoutState;
        //! Problem selector list.
        ProblemSelectorTree *problemTree;
        //! Problem selector tiles.
        ProblemSelectorTiles *problemTiles;
        //! List view button.
        QToolButton *listViewButton;
        //! Tile view button.
        QToolButton *tileViewButton;
        //! Layout of automatic task flow details.
        QFormLayout *flowLayout;
        //! Panel with automatic task flow details.
        QWidget *flowPanel;
        //! Solve order.
        QLabel *solveOrderLabel;
        //! Iterations row of automatic task flow details.
        QWidget *iterationsRow;
        //! Label of number of iterations.
        QLabel *iterationsLabel;
        //! Number of iterations.
        QSpinBox *iterationsSpin;
        //! Convergence value.
        QLineEdit *convergenceEdit;
        //! Panel with customized task flow.
        QWidget *customPanel;
        //! Problem task tree widget.
        ProblemTaskTree *taskTree;
        //! Errors and warnings of the task flow.
        QLabel *issuesLabel;
        //! Customize button.
        QPushButton *customizeButton;
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

        //! Return true if given problem types would be solved in time.
        bool findTransient(RProblemTypeMask problemTypeMask) const;

        //! Return task flow currently defined in the dialog.
        RProblemTaskItem findProblemTaskTree() const;

        //! Switch between list and tile view.
        void setTileView(bool tileView);

        //! Update content of the dialog from its current state.
        void updateState();

    protected slots:

        //! Selection of problem types has changed.
        void onSelectionChanged();

        //! Customized task flow has changed.
        void onTaskTreeChanged();

        //! Customize button clicked.
        void onCustomizeButtonClicked();

        //! Tile was double-clicked.
        void onTileConfirmed();

};

#endif // PROBLEM_TASK_DIALOG_H
