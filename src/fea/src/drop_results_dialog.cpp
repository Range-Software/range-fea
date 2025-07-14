#include <QVBoxLayout>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QDialogButtonBox>

#include "drop_results_dialog.h"
#include "application.h"

static bool clearResults = true;
static bool deleteResults = false;

DropResultsDialog::DropResultsDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(QString("Drop results"));
    this->resize(600,300);

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    this->clearResultsCheckBox = new QCheckBox(tr("Clear results from current model"));
    this->clearResultsCheckBox->setChecked(clearResults);
    mainLayout->addWidget(this->clearResultsCheckBox);

    this->deleteResultsCheckBox = new QCheckBox(tr("Delete all records (model files) related to this model"));
    this->deleteResultsCheckBox->setChecked(deleteResults);
    mainLayout->addWidget(this->deleteResultsCheckBox);

    QListWidget *resultsRecordsList = new QListWidget;
    resultsRecordsList->setEnabled(deleteResults);
    mainLayout->addWidget(resultsRecordsList);

    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    uint nRecords = 0;
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        QList<QString> recordFiles = Application::instance()->getSession()->getModel(selectedModelIDs[i]).getRecordFiles(true);

        for (int j=0;j<recordFiles.size();j++)
        {
            QListWidgetItem *item = new QListWidgetItem(resultsRecordsList);
            item->setText(recordFiles[j]);
            nRecords++;
        }
    }
    if (nRecords == 0)
    {
        this->deleteResultsCheckBox->setDisabled(true);
        resultsRecordsList->setDisabled(true);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->deleteResultsCheckBox,&QCheckBox::clicked,resultsRecordsList,&QListWidget::setEnabled);
}

int DropResultsDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        clearResults = this->clearResultsCheckBox->isChecked();
        deleteResults = this->deleteResultsCheckBox->isChecked();

        if (clearResults)
        {
            QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

            for (int i=0;i<selectedModelIDs.size();i++)
            {
                Application::instance()->getSession()->storeCurentModelVersion(selectedModelIDs[i],tr("Drop results"));
                Application::instance()->getSession()->getModel(selectedModelIDs[i]).clearResults();
                Application::instance()->getSession()->setModelChanged(selectedModelIDs[i]);
            }
        }
        if (deleteResults)
        {
            QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

            for (int i=0;i<selectedModelIDs.size();i++)
            {
                QList<QString> recordFiles = Application::instance()->getSession()->getModel(selectedModelIDs[i]).getRecordFiles(true);

                for (int j=0;j<recordFiles.size();j++)
                {
                    QFile file(recordFiles[j]);
                    RLogger::info("Removing file \'%s\'\n",recordFiles[j].toUtf8().constData());

                    if (!file.remove())
                    {
                        RLogger::warning("Failed to remove file \'%s\'\n",recordFiles[j].toUtf8().constData());
                    }
                }
            }
        }
    }
    return retVal;
}
