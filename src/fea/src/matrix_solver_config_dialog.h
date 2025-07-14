#ifndef MATRIX_SOLVER_CONFIG_DIALOG_H
#define MATRIX_SOLVER_CONFIG_DIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QGroupBox>

#include "value_line_edit.h"

class MatrixSolverConfigDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! CG SOLVER CONFIGURATION
        QGroupBox *groupCG;
        //! Number of iterations.
        QSpinBox *spinCGNIterations;
        //! Convergence value.
        ValueLineEdit *editCGCvgValue;
        //! Output frequency.
        QSpinBox *spinCGOutputFrequency;
        //! GMRES SOLVER CONFIGURATION
        QGroupBox *groupGMRES;
        //! Number of inner iterations.
        QSpinBox *spinGMRESNInnerIterations;
        //! Number of outer iterations.
        QSpinBox *spinGMRESNOuterIterations;
        //! Convergence value.
        ValueLineEdit *editGMRESCvgValue;
        //! Output frequency.
        QSpinBox *spinGMRESOutputFrequency;

    public:

        //! Constructor.
        explicit MatrixSolverConfigDialog(uint modelID,QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();
        
    signals:
        
    public slots:
        
};

#endif // MATRIX_SOLVER_CONFIG_DIALOG_H
