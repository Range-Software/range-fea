#include <algorithm>
#include <limits>

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

#include "application.h"
#include "problem_selector_tiles.h"
#include "problem_selector_tree.h"
#include "problem_task_dialog.h"
#include "solver_setup_checker.h"
#include "model.h"
#include "application_settings.h"
#include "session.h"
#include "main_window.h"
#include <rgl_icon.h>
#include <rgl_message_box.h>

ProblemTaskDialog::ProblemTaskDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
    , modelID(modelID)
    , syncingSelection(false)
    , layoutState(-1)
{
    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);
    const RProblemTaskItem &rTaskTree = rModel.getProblemTaskTree();

    this->initialTypeMask = rTaskTree.getProblemTypeMask();
    this->selectedTypeMask = this->initialTypeMask;
    this->customFlow = !rTaskTree.hasDefaultLayout();

    this->setWindowTitle(tr("Select problem"));

    QIcon cancelIcon = RIcon::fromFile(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon = RIcon::fromFile(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    // Header.
    QLabel *messageLabel = new QLabel(tr("What do you want to solve for model") + " <b>" + rModel.getName() + "</b>?");

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

    // Selection.
    this->problemTree = new ProblemSelectorTree(this);
    this->problemTree->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->problemTree->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::MinimumExpanding);
    this->problemTree->setProblemTypeMask(this->initialTypeMask);
    mainLayout->addWidget(this->problemTree);

    this->problemTiles = new ProblemSelectorTiles(this->problemTree,this);
    this->problemTiles->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::MinimumExpanding);
    mainLayout->addWidget(this->problemTiles);

    // Automatic task flow.
    this->flowPanel = new QWidget;
    mainLayout->addWidget(this->flowPanel);

    this->flowLayout = new QFormLayout;
    this->flowLayout->setContentsMargins(0,0,0,0);
    this->flowPanel->setLayout(this->flowLayout);

    this->solveOrderLabel = new QLabel;
    this->solveOrderLabel->setWordWrap(true);
    this->flowLayout->addRow(tr("Solve order") + ":",this->solveOrderLabel);

    this->iterationsLabel = new QLabel;

    this->iterationsSpin = new QSpinBox;
    this->iterationsSpin->setRange(1,std::numeric_limits<int>::max());
    this->iterationsSpin->setGroupSeparatorShown(true);
    this->iterationsSpin->setToolTip(tr("The solver stops earlier as soon as the problems have converged."));
    this->iterationsSpin->setValue(int(std::min(rTaskTree.getNIterations(),uint(std::numeric_limits<int>::max()))));

    this->convergenceEdit = new QLineEdit(QLocale().toString(rTaskTree.getCvgValue(),'g',6));
    QDoubleValidator *convergenceValidator = new QDoubleValidator(0.0,std::numeric_limits<double>::max(),20,this->convergenceEdit);
    convergenceValidator->setNotation(QDoubleValidator::ScientificNotation);
    this->convergenceEdit->setValidator(convergenceValidator);
    this->convergenceEdit->setToolTip(tr("The iterations end as soon as every task has converged below this value.")
                                      + " " + tr("Zero runs all of them."));

    this->iterationsRow = new QWidget;
    QHBoxLayout *iterationsLayout = new QHBoxLayout;
    iterationsLayout->setContentsMargins(0,0,0,0);
    this->iterationsRow->setLayout(iterationsLayout);
    iterationsLayout->addWidget(this->iterationsSpin);
    iterationsLayout->addSpacing(12);
    iterationsLayout->addWidget(new QLabel(tr("Convergence") + ":"));
    iterationsLayout->addWidget(this->convergenceEdit);
    iterationsLayout->addStretch(1);
    this->flowLayout->addRow(this->iterationsLabel,this->iterationsRow);

    // Customized task flow.
    this->customPanel = new QWidget;
    mainLayout->addWidget(this->customPanel);

    QVBoxLayout *customLayout = new QVBoxLayout;
    customLayout->setContentsMargins(0,0,0,0);
    this->customPanel->setLayout(customLayout);

    QLabel *customHintLabel = new QLabel(tr("Tasks run from top to bottom. Use > to place a task into a group which repeats, and double-click a value to edit it."));
    customHintLabel->setWordWrap(true);
    customLayout->addWidget(customHintLabel);

    this->taskTree = new ProblemTaskTree(rTaskTree,this);
    customLayout->addWidget(this->taskTree);

    // Problems of the task flow.
    this->issuesLabel = new QLabel;
    this->issuesLabel->setWordWrap(true);
    this->issuesLabel->setTextFormat(Qt::RichText);
    mainLayout->addWidget(this->issuesLabel);

    // Buttons.
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonsLayout);

    this->customizeButton = new QPushButton;
    this->customizeButton->setAutoDefault(false);
    buttonsLayout->addWidget(this->customizeButton);
    buttonsLayout->addStretch(1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonsLayout->addWidget(buttonBox);

    this->cancelButton = new QPushButton(cancelIcon,this->initialTypeMask == R_PROBLEM_NONE ? tr("Skip for now") : tr("Cancel"));
    buttonBox->addButton(this->cancelButton,QDialogButtonBox::RejectRole);

    this->okButton = new QPushButton(okIcon,tr("OK"));
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->problemTree,&ProblemSelectorTree::changed,this,&ProblemTaskDialog::onSelectionChanged);
    QObject::connect(this->problemTiles,&ProblemSelectorTiles::confirmed,this,&ProblemTaskDialog::onTileConfirmed);
    QObject::connect(this->taskTree,&ProblemTaskTree::changed,this,&ProblemTaskDialog::onTaskTreeChanged);
    QObject::connect(this->customizeButton,&QPushButton::clicked,this,&ProblemTaskDialog::onCustomizeButtonClicked);
    QObject::connect(this->iterationsSpin,&QSpinBox::valueChanged,this,&ProblemTaskDialog::updateState);
    QObject::connect(this->convergenceEdit,&QLineEdit::textChanged,this,&ProblemTaskDialog::updateState);

    const bool tileView = Application::instance()->getApplicationSettings()->getProblemSelectorTileView();
    this->setTileView(tileView);
    (tileView ? this->tileViewButton : this->listViewButton)->setChecked(true);

    QObject::connect(this->tileViewButton,&QToolButton::toggled,this,[this](bool checked)
    {
        Application::instance()->getApplicationSettings()->setProblemSelectorTileView(checked);
        this->setTileView(checked);
        this->adjustSize();
    });

    this->updateState();
}

int ProblemTaskDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        Model &rModel = Application::instance()->getSession()->getModel(this->modelID);
        RProblemTaskItem problemTaskTree = this->findProblemTaskTree();

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

        const RProblemTypeMask problemTypeMask = problemTaskTree.getProblemTypeMask();
        if (problemTypeMask != R_PROBLEM_NONE && problemTypeMask != this->initialTypeMask)
        {
            Application::instance()->getMainWindow()->showProblemSetupTab(problemTypeMask);
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

bool ProblemTaskDialog::findTransient(RProblemTypeMask problemTypeMask) const
{
    return Application::instance()->getSession()->getModel(this->modelID).getTimeSolver().getEnabled() && RProblem::getTimeSolverEnabled(problemTypeMask);
}

RProblemTaskItem ProblemTaskDialog::findProblemTaskTree() const
{
    if (this->customFlow)
    {
        return this->taskTree->getProblemTaskTree();
    }

    RProblemTaskItem problemTaskTree = RProblemTaskItem::generateDefaultTree(this->selectedTypeMask,this->findTransient(this->selectedTypeMask));

    if (RProblemTaskItem::getNeedsIterations(this->selectedTypeMask))
    {
        problemTaskTree.setNIterations(uint(this->iterationsSpin->value()));

        bool ok = false;
        const double cvgValue = QLocale().toDouble(this->convergenceEdit->text(),&ok);
        if (ok)
        {
            problemTaskTree.setCvgValue(cvgValue);
        }
    }

    return problemTaskTree;
}

void ProblemTaskDialog::setTileView(bool tileView)
{
    this->problemTree->setVisible(!tileView);
    this->problemTiles->setVisible(tileView);
}

void ProblemTaskDialog::updateState()
{
    const RProblemTaskItem problemTaskTree = this->findProblemTaskTree();
    const bool needsIterations = RProblemTaskItem::getNeedsIterations(this->selectedTypeMask);

    const bool showSolveOrder = !this->customFlow && problemTaskTree.getNChildren() > 1;
    const bool showIterations = !this->customFlow && needsIterations;

    if (showSolveOrder)
    {
        QStringList names;
        for (uint i=0;i<problemTaskTree.getNChildren();i++)
        {
            names.append(RProblem::getName(problemTaskTree.getChild(i).getProblemType()));
        }
        this->solveOrderLabel->setText(names.join(QStringLiteral(" → ")));
    }

    this->iterationsLabel->setText((this->findTransient(this->selectedTypeMask) ? tr("Maximum iterations per time step") : tr("Maximum iterations")) + ":");

    this->flowLayout->setRowVisible(this->solveOrderLabel,showSolveOrder);
    this->flowLayout->setRowVisible(this->iterationsRow,showIterations);
    this->flowPanel->setVisible(showSolveOrder || showIterations);
    this->customPanel->setVisible(this->customFlow);

    this->customizeButton->setText(this->customFlow ? tr("Use automatic order") : tr("Customize") + QStringLiteral("…"));
    this->customizeButton->setToolTip(this->customFlow ? tr("Discard the custom task flow and let the order and iterations be set automatically.")
                                                       : tr("Set the order of tasks, groups which repeat and the number of iterations by hand."));
    this->customizeButton->setEnabled(this->selectedTypeMask != R_PROBLEM_NONE);

    QStringList warnings;
    QStringList errors;
    if (this->selectedTypeMask != R_PROBLEM_NONE)
    {
        SolverSetupChecker::checkProblemTaskTree(problemTaskTree,warnings,errors);
    }

    QString issues;
    for (const QString &error : std::as_const(errors))
    {
        issues += QStringLiteral("<div style=\"color:#d32f2f\">") + error + QStringLiteral("</div>");
    }
    for (const QString &warning : std::as_const(warnings))
    {
        issues += QStringLiteral("<div style=\"color:#b26a00\">") + warning + QStringLiteral("</div>");
    }
    this->issuesLabel->setText(issues);
    this->issuesLabel->setVisible(!issues.isEmpty());

    bool valid = errors.isEmpty();
    valid = valid && (this->selectedTypeMask != R_PROBLEM_NONE || this->initialTypeMask != R_PROBLEM_NONE);
    valid = valid && (!showIterations || this->convergenceEdit->hasAcceptableInput());
    this->okButton->setEnabled(valid);

    const int newLayoutState = (this->customFlow ? 1 : 0) | (showSolveOrder ? 2 : 0) | (showIterations ? 4 : 0);
    if (this->layoutState != -1 && this->layoutState != newLayoutState)
    {
        this->layout()->activate();
        this->adjustSize();
    }
    this->layoutState = newLayoutState;
}

void ProblemTaskDialog::onSelectionChanged()
{
    if (this->syncingSelection)
    {
        return;
    }

    const RProblemTypeMask oldTypeMask = this->selectedTypeMask;
    const RProblemTypeMask newTypeMask = this->problemTree->findProblemTypeMask();

    this->selectedTypeMask = newTypeMask;

    if (this->customFlow)
    {
        this->taskTree->removeProblemTypes(oldTypeMask & ~newTypeMask);
        this->taskTree->addProblemTypes(newTypeMask & ~oldTypeMask);
    }
    else if (uint(this->iterationsSpin->value()) == RProblemTaskItem::getDefaultNIterations(oldTypeMask,this->findTransient(oldTypeMask)))
    {
        // A value entered by the user is kept.
        QSignalBlocker blocker(this->iterationsSpin);
        this->iterationsSpin->setValue(int(RProblemTaskItem::getDefaultNIterations(newTypeMask,this->findTransient(newTypeMask))));
    }

    this->updateState();
}

void ProblemTaskDialog::onTaskTreeChanged()
{
    this->selectedTypeMask = this->taskTree->getProblemTaskTree().getProblemTypeMask();

    this->syncingSelection = true;
    this->problemTree->setProblemTypeMask(this->selectedTypeMask);
    this->syncingSelection = false;

    this->updateState();
}

void ProblemTaskDialog::onCustomizeButtonClicked()
{
    if (this->customFlow)
    {
        const RProblemTaskItem problemTaskTree = RProblemTaskItem::generateDefaultTree(this->selectedTypeMask,this->findTransient(this->selectedTypeMask));

        QSignalBlocker spinBlocker(this->iterationsSpin);
        QSignalBlocker editBlocker(this->convergenceEdit);
        this->iterationsSpin->setValue(int(problemTaskTree.getNIterations()));
        this->convergenceEdit->setText(QLocale().toString(problemTaskTree.getCvgValue(),'g',6));

        this->customFlow = false;
    }
    else
    {
        this->taskTree->setProblemTaskTree(this->findProblemTaskTree());
        this->customFlow = true;
    }

    this->updateState();
}

void ProblemTaskDialog::onTileConfirmed()
{
    if (this->okButton->isEnabled())
    {
        this->accept();
    }
}
