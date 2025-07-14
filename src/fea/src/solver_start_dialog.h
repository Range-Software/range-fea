#ifndef SOLVER_START_DIALOG_H
#define SOLVER_START_DIALOG_H

#include <QDialog>
#include <QCheckBox>

#include "solver_task.h"

class SolverStartDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model IDs.
        uint modelID;
        //! List of warnings.
        QStringList warnings;
        //! List of errors.
        QStringList errors;
        //! Restart solver.
        QCheckBox *restartSolverCheck;
        //! Pointer to solvr task object.
        SolverTask *pSolverTask;

    public:

        //! Constructor.
        explicit SolverStartDialog(uint modelID, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

};

#endif // SOLVER_START_DIALOG_H
