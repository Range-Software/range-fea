#include <QVBoxLayout>
#include <QIcon>
#include <QTabWidget>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rml_problem.h>

#include "convergence_graph_dialog.h"
#include "convergence_graph_widget.h"

ConvergenceGraphDialog::ConvergenceGraphDialog(const QStringList &convergenceFileNames, QWidget *parent) :
    QDialog(parent)
{
    QIcon closeIcon(":/icons/action/pixmaps/range-close.svg");

    this->setWindowTitle(tr("Solver convergence"));
    this->resize(600,400);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QTabWidget *tabWidget = new QTabWidget;
    mainLayout->addWidget(tabWidget);

    for (int i=0;i<convergenceFileNames.size();i++)
    {
        ConvergenceGraphWidget *convergenceGraphWidget = new ConvergenceGraphWidget(convergenceFileNames.at(i),this);
        tabWidget->addTab(convergenceGraphWidget,ConvergenceGraphDialog::getNameFromID(convergenceFileNames.at(i),QString::number(i+1)));
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *closeButton = new QPushButton(closeIcon, tr("Close"));
    buttonBox->addButton(closeButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

QString ConvergenceGraphDialog::getNameFromID(const QString &fileName, const QString &fallbackName)
{
    std::vector<RProblemType> problemTypes = RProblem::getTypes(R_PROBLEM_ALL);
    for (uint i=0;i<problemTypes.size();i++)
    {
        if (fileName.contains("-" + RProblem::getId(problemTypes[i]) + "."))
        {
            return RProblem::getName(problemTypes[i]);
        }
    }

    for (RMatrixSolverType type=RMatrixSolverConf::None;type<RMatrixSolverConf::NTypes;type++)
    {
        if (fileName.contains(RMatrixSolverConf::getId(type)))
        {
            return RMatrixSolverConf::getName(type);
        }
    }

    return fallbackName;
}
