#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_logger.h>

#include "problem_selector_dialog.h"

ProblemSelectorDialog::ProblemSelectorDialog(QWidget *parent) :
    QDialog(parent)
{
    R_LOG_TRACE_IN;
    this->setWindowTitle(QString("Problem type selector"));

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *messageLabel = new QLabel("Select problem types");
    mainLayout->addWidget(messageLabel);

    this->problemTree = new ProblemSelectorTree(this);
    this->problemTree->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->problemTree->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::MinimumExpanding);
    mainLayout->addWidget(this->problemTree);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setDisabled(true);
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->problemTree,&ProblemSelectorTree::changed,this,&ProblemSelectorDialog::onProblemTreeChanged);
    R_LOG_TRACE_OUT;
}

RProblemTypeMask ProblemSelectorDialog::exec()
{
    R_LOG_TRACE_IN;
    if (QDialog::exec() == QDialog::Accepted)
    {
        R_LOG_TRACE_RETURN(this->problemTree->findProblemTypeMask());
    }

    R_LOG_TRACE_RETURN(R_PROBLEM_NONE);
}

void ProblemSelectorDialog::onProblemTreeChanged()
{
    R_LOG_TRACE_IN;
    this->okButton->setEnabled(this->problemTree->checkProblemIsChecked());
    R_LOG_TRACE_OUT;
}
