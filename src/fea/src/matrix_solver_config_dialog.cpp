#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialogButtonBox>

#include "application.h"
#include "matrix_solver_config_dialog.h"

MatrixSolverConfigDialog::MatrixSolverConfigDialog(uint modelID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Matrix solver configuration"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    // CG SOLVER
    RMatrixSolverConf &solverConfCG = Application::instance()->getSession()->getModel(this->modelID).getMatrixSolverConf(RMatrixSolverConf::CG);

    this->groupCG = new QGroupBox(RMatrixSolverConf::getName(RMatrixSolverConf::CG));
    mainLayout->addWidget(this->groupCG);

    QFormLayout *cgLayout = new QFormLayout;
    this->groupCG->setLayout(cgLayout);

    this->spinCGNIterations = new QSpinBox;
    this->spinCGNIterations->setRange(1,INT_MAX);
    this->spinCGNIterations->setValue(solverConfCG.getNOuterIterations());
    cgLayout->addRow(tr("Number of iterations") + ":",this->spinCGNIterations);

    this->editCGCvgValue = new ValueLineEdit(0.0,1.0);
    this->editCGCvgValue->setValue(solverConfCG.getSolverCvgValue());
    cgLayout->addRow(tr("Convergence value") + ":",this->editCGCvgValue);

    this->spinCGOutputFrequency = new QSpinBox;
    this->spinCGOutputFrequency->setRange(0,INT_MAX);
    this->spinCGOutputFrequency->setValue(solverConfCG.getOutputFrequency());
    cgLayout->addRow(tr("Solver output frequency") + ":",this->spinCGOutputFrequency);

    // GMRES SOLVER
    RMatrixSolverConf &solverConfGMRES = Application::instance()->getSession()->getModel(this->modelID).getMatrixSolverConf(RMatrixSolverConf::GMRES);

    this->groupGMRES = new QGroupBox(RMatrixSolverConf::getName(RMatrixSolverConf::GMRES));
    mainLayout->addWidget(this->groupGMRES);

    QFormLayout *gmresLayout = new QFormLayout;
    this->groupGMRES->setLayout(gmresLayout);

    this->spinGMRESNInnerIterations = new QSpinBox;
    this->spinGMRESNInnerIterations->setRange(1,INT_MAX);
    this->spinGMRESNInnerIterations->setValue(solverConfGMRES.getNInnerIterations());
    gmresLayout->addRow(tr("Number of inner iterations") + ":",this->spinGMRESNInnerIterations);

    this->spinGMRESNOuterIterations = new QSpinBox;
    this->spinGMRESNOuterIterations->setRange(1,INT_MAX);
    this->spinGMRESNOuterIterations->setValue(solverConfGMRES.getNOuterIterations());
    gmresLayout->addRow(tr("Number of outer iterations") + ":",this->spinGMRESNOuterIterations);

    this->editGMRESCvgValue = new ValueLineEdit(0.0,1.0);
    this->editGMRESCvgValue->setValue(solverConfGMRES.getSolverCvgValue());
    gmresLayout->addRow(tr("Convergence value") + ":",this->editGMRESCvgValue);

    this->spinGMRESOutputFrequency = new QSpinBox;
    this->spinGMRESOutputFrequency->setRange(0,INT_MAX);
    this->spinGMRESOutputFrequency->setValue(solverConfGMRES.getOutputFrequency());
    gmresLayout->addRow(tr("Solver output frequency") + ":",this->spinGMRESOutputFrequency);

    // Button layout

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int MatrixSolverConfigDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        RMatrixSolverConf &solverConfCG = Application::instance()->getSession()->getModel(this->modelID).getMatrixSolverConf(RMatrixSolverConf::CG);

        solverConfCG.setNOuterIterations(this->spinCGNIterations->value());
        solverConfCG.setSolverCvgValue(this->editCGCvgValue->getValue());
        solverConfCG.setOutputFrequency(this->spinCGOutputFrequency->value());

        RMatrixSolverConf &solverConfGMRES = Application::instance()->getSession()->getModel(this->modelID).getMatrixSolverConf(RMatrixSolverConf::GMRES);

        solverConfGMRES.setNInnerIterations(this->spinGMRESNInnerIterations->value());
        solverConfGMRES.setNOuterIterations(this->spinGMRESNOuterIterations->value());
        solverConfGMRES.setSolverCvgValue(this->editGMRESCvgValue->getValue());
        solverConfGMRES.setOutputFrequency(this->spinGMRESOutputFrequency->value());
    }

    return retVal;
}
