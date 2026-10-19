#include <algorithm>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "application.h"
#include "problem_task_dialog.h"
#include <rgl_icon.h>
#include <rgl_message_box.h>

ProblemTaskDialog::ProblemTaskDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
    , modelID(modelID)
{
    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    this->setWindowTitle(tr("Problem task flow"));
    this->resize(600,300);

    QIcon cancelIcon = RIcon::fromFile(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon = RIcon::fromFile(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *messageLabel = new QLabel;
    messageLabel->setText(tr("Problem task flow for model") + ": <b>" + rModel.getName() + "</b>");
    mainLayout->addWidget(messageLabel);

    this->taskTree = new ProblemTaskTree(rModel.getProblemTaskTree(),this);
    mainLayout->addWidget(this->taskTree);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    this->cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    this->cancelButton->setDisabled(rModel.getProblemTaskTree().getProblemTypeMask() == R_PROBLEM_NONE);
    buttonBox->addButton(this->cancelButton,QDialogButtonBox::RejectRole);

    // An empty task flow is a valid choice - it removes all problems.
    this->okButton = new QPushButton(okIcon, tr("OK"));
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int ProblemTaskDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        Model &rModel = Application::instance()->getSession()->getModel(this->modelID);
        RProblemTaskItem problemTaskTree = this->taskTree->getProblemTaskTree();

        RProblemTypeMask removedTypeMask = rModel.getProblemTaskTree().getProblemTypeMask() & ~problemTaskTree.getProblemTypeMask();
        bool resultsCleared = (removedTypeMask != R_PROBLEM_NONE) && this->clearResults(removedTypeMask,problemTaskTree.getProblemTypeMask());

        if (problemTaskTree.getNChildren() == 0)
        {
            // Without any problem the model keeps no problem configuration -
            // time solver, matrix solvers, monitoring points and problem setup
            // return to their defaults.
            static_cast<RProblem&>(rModel) = RProblem();
        }

        // Also removes all conditions not belonging to any selected problem.
        rModel.setProblemTaskTree(problemTaskTree);

        RLogger::info("Problem task flow have changed for model \'%s\'\n", Application::instance()->getSession()->getModel(this->modelID).getName().toUtf8().constData());

        Application::instance()->getSession()->setProblemSelectionChanged(this->modelID);
        if (resultsCleared)
        {
            Application::instance()->getSession()->setResultsChanged(this->modelID);
        }
    }
    return retVal;
}

bool ProblemTaskDialog::clearResults(RProblemTypeMask removedTypeMask, RProblemTypeMask remainingTypeMask)
{
    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    // Results still computed by a remaining problem are kept - e.g. temperature
    // when heat transfer in fluids stays in the flow.
    std::vector<RVariableType> remainingVariableTypes = RProblem::getComputedVariableTypes(remainingTypeMask);

    std::vector<RVariableType> variableTypes;
    for (RVariableType variableType : RProblem::getComputedVariableTypes(removedTypeMask))
    {
        if (rModel.findVariable(variableType) != RConstants::eod &&
            std::find(remainingVariableTypes.begin(),remainingVariableTypes.end(),variableType) == remainingVariableTypes.end())
        {
            variableTypes.push_back(variableType);
        }
    }

    if (variableTypes.empty())
    {
        return false;
    }

    QStringList problemNames;
    for (RProblemType problemType : RProblem::getTypes(removedTypeMask))
    {
        problemNames.append(RProblem::getName(problemType));
    }

    QStringList variableNames;
    for (RVariableType variableType : variableTypes)
    {
        variableNames.append(RVariable::getName(variableType));
    }

    QString question = tr("Following problems were removed from the task flow") + ": <b>" + problemNames.join(", ") + "</b>.<br/><br/>"
                     + tr("Clear their results from the model?") + "<br/>"
                     + variableNames.join(", ");

    if (RMessageBox::question(this,tr("Clear results"),question) != RMessageBox::Yes)
    {
        return false;
    }

    RLogger::info("Clearing results of removed problems from model \'%s\'\n",rModel.getName().toUtf8().constData());
    for (RVariableType variableType : variableTypes)
    {
        uint variablePosition = RConstants::eod;
        while ((variablePosition = rModel.findVariable(variableType)) != RConstants::eod)
        {
            rModel.removeVariable(variablePosition);
        }
    }

    return true;
}

void ProblemTaskDialog::closeEvent(QCloseEvent *e)
{
    if (Application::instance()->getSession()->getModel(this->modelID).getProblemTaskTree().getProblemTypeMask() == R_PROBLEM_NONE)
    {
        e->ignore();
    }
    else
    {
        QDialog::closeEvent(e);
    }
}

void ProblemTaskDialog::keyPressEvent(QKeyEvent *e)
{
    // Ignore ESC key.
    if (e->key() == Qt::Key_Escape && (Application::instance()->getSession()->getModel(this->modelID).getProblemTaskTree().getProblemTypeMask() == R_PROBLEM_NONE))
    {
        e->ignore();
    }
    else
    {
        QDialog::keyPressEvent(e);
    }
}
