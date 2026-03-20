#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include "model.h"
#include "new_model_dialog.h"
#include "model_io.h"

NewModelDialog::NewModelDialog(QWidget *parent) :
    QDialog(parent)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Create a new model"));
    this->resize(300,-1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    this->editName = new QLineEdit;
    this->editName->setPlaceholderText(tr("New model name"));
    formLayout->addRow(tr("Name") + ":",this->editName);

    this->editDesc = new QLineEdit;
    this->editDesc->setPlaceholderText(tr("New model description"));
    formLayout->addRow(tr("Description") + ":",this->editDesc);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    okButton->setEnabled(!this->editName->text().isEmpty());
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(this->editName,&QLineEdit::textChanged,okButton,[okButton](const QString &text){
        okButton->setEnabled(!text.trimmed().isEmpty());
    });

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int NewModelDialog::exec()
{
    int retVal = this->QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        Model *pNewModel = new Model;
        pNewModel->setName(this->editName->text());
        pNewModel->setDescription(this->editDesc->text());
        
        RJobManager::getInstance().submit(new ModelIO(MODEL_IO_ADD, QString(), pNewModel));
    }
    return retVal;
}
