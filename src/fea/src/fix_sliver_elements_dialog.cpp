#include <QVBoxLayout>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include "fix_sliver_elements_dialog.h"
#include "application.h"
#include "model_action.h"

FixSliverElementsDialog::FixSliverElementsDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
    , modelID(modelID)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Fix sliver elements"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *label = new QLabel(tr("Fix sliver elements for model") + " <b>" + Application::instance()->getSession()->getModel(this->modelID).getName() + "</b>");
    mainLayout->addWidget(label);

    QFormLayout *toleranceLayout = new QFormLayout;
    mainLayout->addLayout(toleranceLayout);

    this->edgeRatioEdit = new ValueLineEdit(1.0,1e10);
    this->edgeRatioEdit->setValue(Model::SliverElementEdgeRatio);
    toleranceLayout->addRow(tr("Edge length ratio") + ":",this->edgeRatioEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int FixSliverElementsDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        ModelActionInput modelActionInput(this->modelID);
        modelActionInput.setFixSliverElements(this->edgeRatioEdit->getValue());

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    return retVal;
}
