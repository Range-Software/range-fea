#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "application.h"
#include "problem_task_dialog.h"

ProblemTaskDialog::ProblemTaskDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
    , modelID(modelID)
{
    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    this->setWindowTitle(QString("Problem task flow"));
    this->resize(600,300);

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

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

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setDisabled(true);
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->taskTree,&ProblemTaskTree::changed,this,&ProblemTaskDialog::onProblemTaskTreeChanged);
}

int ProblemTaskDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        Application::instance()->getSession()->getModel(this->modelID).setProblemTaskTree(this->taskTree->getProblemTaskTree());

        RLogger::info("Problem task flow have changed for model \'%s\'\n", Application::instance()->getSession()->getModel(this->modelID).getName().toUtf8().constData());

        Application::instance()->getSession()->setProblemSelectionChanged(this->modelID);
    }
    return retVal;
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

void ProblemTaskDialog::onProblemTaskTreeChanged()
{
    RProblemTaskItem problemTaskTree = this->taskTree->getProblemTaskTree();
    this->okButton->setEnabled(problemTaskTree.getNChildren() > 0);
}
