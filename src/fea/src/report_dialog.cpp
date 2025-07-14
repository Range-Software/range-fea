#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include <rgl_file_chooser_button.h>

#include "application.h"
#include "report_dialog.h"
#include "report_generator.h"

bool ReportDialog::htmlFileEnabled = true;
bool ReportDialog::markdownFileEnabled = true;
bool ReportDialog::pdfFileEnabled = true;
bool ReportDialog::odfFileEnabled = true;

ReportDialog::ReportDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
    , modelID(modelID)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Create report"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QLabel *titleLabel = new QLabel(tr("Generate report for model") + " <b>" + Application::instance()->getSession()->getModel(this->modelID).getName() + "</b>");
    mainLayout->addWidget(titleLabel);

    QGroupBox *fileGroupBox = new QGroupBox(tr("File"));
    mainLayout->addWidget(fileGroupBox);

    QVBoxLayout *fileLayout = new QVBoxLayout;
    fileGroupBox->setLayout(fileLayout);

    QString htmlFileName = Application::instance()->getSession()->getModel(this->modelID).buildDocFileName("html");
    QString markDownFileName = Application::instance()->getSession()->getModel(this->modelID).buildDocFileName("md");
    QString odfFileName = Application::instance()->getSession()->getModel(this->modelID).buildDocFileName("odf");
    QString pdfFileName = Application::instance()->getSession()->getModel(this->modelID).buildDocFileName("pdf");

    this->htmlGroupBox = new QGroupBox(tr("HyperText Markup Language") + " (HTML)");
    this->htmlGroupBox->setCheckable(true);
    this->htmlGroupBox->setChecked(ReportDialog::htmlFileEnabled);
    fileLayout->addWidget(this->htmlGroupBox);

    QVBoxLayout *htmlFileLayout = new QVBoxLayout;
    this->htmlGroupBox->setLayout(htmlFileLayout);

    this->htmlFileChooserButton = new RFileChooserButton(tr("Report file") + ":",
                                                         RFileChooserButton::SaveFile,
                                                         tr("Select file"),
                                                         htmlFileName,
                                                         "HTML " + tr("files") + " (*.html *.htm);;" + tr("Any files") + " (*)");
    htmlFileLayout->addWidget(this->htmlFileChooserButton);

    this->markdownGroupBox = new QGroupBox(tr("Markdown") + " (MD)");
    this->markdownGroupBox->setCheckable(true);
    this->markdownGroupBox->setChecked(ReportDialog::markdownFileEnabled);
    fileLayout->addWidget(this->markdownGroupBox);

    QVBoxLayout *markDownFileLayout = new QVBoxLayout;
    this->markdownGroupBox->setLayout(markDownFileLayout);

    this->markdownFileChooserButton = new RFileChooserButton(tr("Report file") + ":",
                                                             RFileChooserButton::SaveFile,
                                                             tr("Select file"),
                                                             markDownFileName,
                                                             "Markdown " + tr("files") + " (*.md);;" + tr("Any files") + " (*)");
    markDownFileLayout->addWidget(this->markdownFileChooserButton);

    this->odfGroupBox = new QGroupBox(tr("OpenDocument Format") + " (ODF)");
    this->odfGroupBox->setCheckable(true);
    this->odfGroupBox->setChecked(ReportDialog::odfFileEnabled);
    fileLayout->addWidget(this->odfGroupBox);

    QVBoxLayout *odfFileLayout = new QVBoxLayout;
    this->odfGroupBox->setLayout(odfFileLayout);

    this->odfFileChooserButton = new RFileChooserButton(tr("Report file") + ":",
                                                        RFileChooserButton::SaveFile,
                                                        tr("Select file"),
                                                        odfFileName,
                                                        "ODF " + tr("files") + " (*.odf);;" + tr("Any files") + " (*)");
    odfFileLayout->addWidget(this->odfFileChooserButton);

    this->pdfGroupBox = new QGroupBox(tr("Portable Document Format") +  "(PDF)");
    this->pdfGroupBox->setCheckable(true);
    this->pdfGroupBox->setChecked(ReportDialog::pdfFileEnabled);
    fileLayout->addWidget(this->pdfGroupBox);

    QVBoxLayout *pdfFileLayout = new QVBoxLayout;
    this->pdfGroupBox->setLayout(pdfFileLayout);

    this->pdfFileChooserButton = new RFileChooserButton(tr("Report file") + ":",
                                                        RFileChooserButton::SaveFile,
                                                        tr("Select file"),
                                                        pdfFileName,
                                                        "PDF " + tr("files") + " (*.pdf);;" + tr("Any files") + " (*)");
    pdfFileLayout->addWidget(this->pdfFileChooserButton);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int ReportDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        ReportDialog::htmlFileEnabled = this->htmlGroupBox->isChecked();
        ReportDialog::markdownFileEnabled = this->markdownGroupBox->isChecked();
        ReportDialog::odfFileEnabled = this->odfGroupBox->isChecked();
        ReportDialog::pdfFileEnabled = this->pdfGroupBox->isChecked();

        ReportGenerator *reportGenerator = new ReportGenerator(this->modelID);

        if (ReportDialog::htmlFileEnabled)
        {
            reportGenerator->addExportType(ReportGenerator::HTML,this->htmlFileChooserButton->getFileName());
        }
        if (ReportDialog::markdownFileEnabled)
        {
            reportGenerator->addExportType(ReportGenerator::Markdown,this->markdownFileChooserButton->getFileName());
        }
        if (ReportDialog::odfFileEnabled)
        {
            reportGenerator->addExportType(ReportGenerator::ODF,this->odfFileChooserButton->getFileName());
        }
        if (ReportDialog::pdfFileEnabled)
        {
            reportGenerator->addExportType(ReportGenerator::PDF,this->pdfFileChooserButton->getFileName());
        }

        RJobManager::getInstance().submit(reportGenerator);
    }

    return retVal;
}
