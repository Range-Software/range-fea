#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialogButtonBox>

#include "rename_model_dialog.h"
#include "application.h"

RenameModelDialog::RenameModelDialog(uint modelId, QWidget *parent)
	: QDialog(parent)
    , modelId(modelId)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Rename model"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    this->editName = new QLineEdit(Application::instance()->getSession()->getModel(this->modelId).getName());
    this->editName->setPlaceholderText(tr("Model name"));
    formLayout->addRow(tr("Name") + ":",this->editName);

    this->editDesc = new QLineEdit(Application::instance()->getSession()->getModel(this->modelId).getDescription());
    this->editDesc->setPlaceholderText(tr("Model description"));
    formLayout->addRow(tr("Description") + ":",this->editDesc);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int RenameModelDialog::exec()
{
    int retVal = this->QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        Application::instance()->getSession()->getModel(this->modelId).setName(this->getName());
        Application::instance()->getSession()->getModel(this->modelId).setDescription(this->getDescription());
        Application::instance()->getSession()->setModelChanged(this->modelId);
    }
    return retVal;
}

QString RenameModelDialog::getName() const
{
    return this->editName->text().isEmpty() ? tr("New model") : this->editName->text();
}

QString RenameModelDialog::getDescription() const
{
    return this->editDesc->text().isEmpty() ? tr("New model") : this->editDesc->text();
}
