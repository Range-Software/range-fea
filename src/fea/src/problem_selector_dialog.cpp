#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QButtonGroup>
#include <QDialogButtonBox>

#include <rbl_logger.h>

#include "application.h"
#include "problem_selector_dialog.h"
#include "problem_selector_tiles.h"
#include <rgl_icon.h>

ProblemSelectorDialog::ProblemSelectorDialog(QWidget *parent) :
    QDialog(parent)
{
    R_LOG_TRACE_IN;
    this->setWindowTitle(tr("Problem type selector"));

    QIcon cancelIcon = RIcon::fromFile(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon = RIcon::fromFile(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *messageLabel = new QLabel(tr("Select problem types"));

    this->listViewButton = new QToolButton;
    this->listViewButton->setIcon(RIcon::fromFile(":/icons/action/pixmaps/range-view_list.svg"));
    this->listViewButton->setToolTip(tr("List view"));
    this->listViewButton->setCheckable(true);
    this->listViewButton->setAutoRaise(true);

    this->tileViewButton = new QToolButton;
    this->tileViewButton->setIcon(RIcon::fromFile(":/icons/action/pixmaps/range-view_tiles.svg"));
    this->tileViewButton->setToolTip(tr("Tile view"));
    this->tileViewButton->setCheckable(true);
    this->tileViewButton->setAutoRaise(true);

    QButtonGroup *viewButtonGroup = new QButtonGroup(this);
    viewButtonGroup->addButton(this->listViewButton);
    viewButtonGroup->addButton(this->tileViewButton);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addWidget(messageLabel);
    headerLayout->addStretch(1);
    headerLayout->addWidget(this->listViewButton);
    headerLayout->addWidget(this->tileViewButton);
    mainLayout->addLayout(headerLayout);

    this->problemTree = new ProblemSelectorTree(this);
    this->problemTree->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->problemTree->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::MinimumExpanding);
    mainLayout->addWidget(this->problemTree);

    this->problemTiles = new ProblemSelectorTiles(this->problemTree,this);
    this->problemTiles->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::MinimumExpanding);
    mainLayout->addWidget(this->problemTiles);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    this->okButton = new QPushButton(okIcon, tr("OK"));
    this->okButton->setDisabled(true);
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->problemTree,&ProblemSelectorTree::changed,this,&ProblemSelectorDialog::onProblemTreeChanged);

    const bool tileView = Application::instance()->getApplicationSettings()->getProblemSelectorTileView();
    this->setTileView(tileView);
    (tileView ? this->tileViewButton : this->listViewButton)->setChecked(true);

    QObject::connect(this->tileViewButton,&QToolButton::toggled,this,[this](bool checked)
    {
        Application::instance()->getApplicationSettings()->setProblemSelectorTileView(checked);
        this->setTileView(checked);
        this->adjustSize();
    });
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

void ProblemSelectorDialog::setTileView(bool tileView)
{
    R_LOG_TRACE_IN;
    this->problemTree->setVisible(!tileView);
    this->problemTiles->setVisible(tileView);
    R_LOG_TRACE_OUT;
}
