#include <rml_problem.h>

#include "problem_selector_tree.h"

//! Role of item data holding short note about its state.
static constexpr int problemNoteRole = Qt::UserRole + 1;

typedef enum _ProblemSelectorTreeColumn
{
    PROBLEM_TREE_COLUMN_NAME = 0,
    PROBLEM_TREE_COLUMN_DESC,
    PROBLEM_TREE_N_COLUMNS
} ProblemSelectorTreeColumn;

ProblemSelectorTree::ProblemSelectorTree(QWidget *parent)
    : QTreeWidget(parent)
{
    this->setRootIsDecorated(false);
    this->setSelectionMode(QAbstractItemView::NoSelection);

    this->setColumnCount(PROBLEM_TREE_N_COLUMNS);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(PROBLEM_TREE_COLUMN_NAME,tr("Name"));
    headerItem->setText(PROBLEM_TREE_COLUMN_DESC,tr("Description"));
    this->setHeaderItem(headerItem);

    this->resizeColumnToContents(PROBLEM_TREE_COLUMN_NAME);

    std::vector<RProblemType> prbList(ProblemSelectorTree::sortTypesByDependency(RProblem::getTypes(R_PROBLEM_ALL)));

    for (uint i=0;i<prbList.size();i++)
    {
        QTreeWidgetItem *itemModel = new QTreeWidgetItem(this);
        itemModel->setText(PROBLEM_TREE_COLUMN_NAME, RProblem::getName(prbList[i]));
        itemModel->setData(PROBLEM_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(Qt::Unchecked));
        itemModel->setData(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole,QVariant(prbList[i]));
        itemModel->setText(PROBLEM_TREE_COLUMN_DESC, RProblem::getDesc(prbList[i]));

        QFont font = itemModel->font(PROBLEM_TREE_COLUMN_NAME);
        font.setBold(true);
        itemModel->setFont(PROBLEM_TREE_COLUMN_NAME,font);
    }
    for (int i=0;i<PROBLEM_TREE_N_COLUMNS;i++)
    {
        this->resizeColumnToContents(i);
    }

    this->updateItemStates();

    QObject::connect(this,&QTreeWidget::itemChanged,this,&ProblemSelectorTree::onTreeWidgetItemChanged);
}

std::vector<RProblemType> ProblemSelectorTree::sortTypesByDependency(const std::vector<RProblemType> &problemTypes)
{
    std::vector<RProblemType> sortedTypes;
    RProblemTypeMask listedMask = R_PROBLEM_NONE;

    for (uint i=0;i<problemTypes.size();i++)
    {
        if (RProblem::getRequiredProblemTypeMask(problemTypes[i]) == R_PROBLEM_NONE)
        {
            ProblemSelectorTree::appendTypeWithDependents(problemTypes,problemTypes[i],sortedTypes,listedMask);
        }
    }

    // Problem types with dependencies which could not be resolved.
    for (uint i=0;i<problemTypes.size();i++)
    {
        if (!(listedMask & problemTypes[i]))
        {
            sortedTypes.push_back(problemTypes[i]);
            listedMask |= problemTypes[i];
        }
    }

    return sortedTypes;
}

void ProblemSelectorTree::appendTypeWithDependents(const std::vector<RProblemType> &problemTypes, RProblemType problemType, std::vector<RProblemType> &sortedTypes, RProblemTypeMask &listedMask)
{
    sortedTypes.push_back(problemType);
    listedMask |= problemType;

    // Dependent problem types are listed directly after the problem type they depend on.
    for (uint i=0;i<problemTypes.size();i++)
    {
        if (listedMask & problemTypes[i])
        {
            continue;
        }
        RProblemTypeMask requiredMask = RProblem::getRequiredProblemTypeMask(problemTypes[i]);
        if ((requiredMask & problemType) && (requiredMask & ~listedMask) == R_PROBLEM_NONE)
        {
            ProblemSelectorTree::appendTypeWithDependents(problemTypes,problemTypes[i],sortedTypes,listedMask);
        }
    }
}

RProblemTypeMask ProblemSelectorTree::findProblemTypeMask() const
{
    RProblemTypeMask prbTypeMask = R_PROBLEM_NONE;
    int nItems = this->topLevelItemCount();
    for (int j=0;j<nItems;j++)
    {
        QTreeWidgetItem *item = this->topLevelItem(j);
        Qt::CheckState checkState = item->checkState(PROBLEM_TREE_COLUMN_NAME);
        if (checkState == Qt::Checked)
        {
            prbTypeMask |= RProblemType(item->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt());
        }
    }
    return prbTypeMask;
}

void ProblemSelectorTree::setProblemTypeMask(RProblemTypeMask problemTypeMask)
{
    const bool signalsWereBlocked = this->blockSignals(true);
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->topLevelItem(i);
        const bool checked = (item->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt() & problemTypeMask) != 0;
        item->setCheckState(PROBLEM_TREE_COLUMN_NAME,checked ? Qt::Checked : Qt::Unchecked);
    }
    this->blockSignals(signalsWereBlocked);

    this->updateItemStates();

    emit this->changed();
}

std::vector<RProblemType> ProblemSelectorTree::getProblemTypes() const
{
    std::vector<RProblemType> problemTypes;
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        problemTypes.push_back(RProblemType(this->topLevelItem(i)->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt()));
    }
    return problemTypes;
}

bool ProblemSelectorTree::isProblemTypeChecked(RProblemType problemType) const
{
    const QTreeWidgetItem *item = this->findItem(problemType);
    return item && item->checkState(PROBLEM_TREE_COLUMN_NAME) != Qt::Unchecked;
}

bool ProblemSelectorTree::isProblemTypeAvailable(RProblemType problemType) const
{
    const QTreeWidgetItem *item = this->findItem(problemType);
    return item && (item->flags() & Qt::ItemIsEnabled);
}

QString ProblemSelectorTree::findProblemTypeNote(RProblemType problemType) const
{
    const QTreeWidgetItem *item = this->findItem(problemType);
    return item ? item->data(PROBLEM_TREE_COLUMN_NAME,problemNoteRole).toString() : QString();
}

QString ProblemSelectorTree::findProblemTypeToolTip(RProblemType problemType) const
{
    const QTreeWidgetItem *item = this->findItem(problemType);
    return item ? item->toolTip(PROBLEM_TREE_COLUMN_NAME) : QString();
}

void ProblemSelectorTree::setProblemTypeChecked(RProblemType problemType, bool checked)
{
    QTreeWidgetItem *item = this->findItem(problemType);
    if (item && (!checked || (item->flags() & Qt::ItemIsEnabled)))
    {
        item->setCheckState(PROBLEM_TREE_COLUMN_NAME,checked ? Qt::Checked : Qt::Unchecked);
    }
}

QTreeWidgetItem *ProblemSelectorTree::findItem(RProblemType problemType) const
{
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->topLevelItem(i);
        if (RProblemType(item->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt()) == problemType)
        {
            return item;
        }
    }
    return nullptr;
}

void ProblemSelectorTree::updateItemStates()
{
    const RProblemTypeMask checkedTypeMask = this->findProblemTypeMask();
    const std::vector<RProblemType> checkedTypes = RProblem::getTypes(checkedTypeMask);

    const bool signalsWereBlocked = this->blockSignals(true);

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->topLevelItem(i);
        const RProblemType problemType = RProblemType(item->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt());
        const bool checked = (checkedTypeMask & problemType) != 0;

        // Selecting a problem type also selects those it requires.
        const RProblemTypeMask selectedTogetherMask = problemType | RProblem::getRequiredProblemTypeMask(problemType);

        QStringList requiredBy;
        QStringList incompatibleWith;
        for (RProblemType checkedType : checkedTypes)
        {
            if (RProblem::getRequiredProblemTypeMask(checkedType) & problemType)
            {
                requiredBy.append(RProblem::getName(checkedType));
            }
            if ((RProblem::getExcludedProblemTypeMask(selectedTogetherMask) & checkedType) ||
                (RProblem::getExcludedProblemTypeMask(checkedType) & selectedTogetherMask))
            {
                incompatibleWith.append(RProblem::getName(checkedType));
            }
        }

        // A checked item stays enabled, so that a conflict can always be resolved.
        const bool available = checked || incompatibleWith.isEmpty();

        QString note;
        QString toolTip = RProblem::getDesc(problemType);
        if (!incompatibleWith.isEmpty())
        {
            note = tr("Incompatible");
            toolTip += "\n\n" + tr("Incompatible with") + ": " + incompatibleWith.join(", ");
        }
        else if (!requiredBy.isEmpty())
        {
            note = tr("Required");
            toolTip += "\n\n" + tr("Required by") + ": " + requiredBy.join(", ");
        }

        item->setFlags(available ? (item->flags() | Qt::ItemIsEnabled) : (item->flags() & ~Qt::ItemIsEnabled));
        item->setData(PROBLEM_TREE_COLUMN_NAME,problemNoteRole,note);
        item->setToolTip(PROBLEM_TREE_COLUMN_NAME,toolTip);
        item->setToolTip(PROBLEM_TREE_COLUMN_DESC,toolTip);
    }

    this->blockSignals(signalsWereBlocked);
}

void ProblemSelectorTree::checkCheckedTreeWidgetItem(QTreeWidgetItem *item)
{
    const RProblemTypeMask requiredProblemTypeMask = RProblem::getRequiredProblemTypeMask(item->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt());

    const bool signalsWereBlocked = this->blockSignals(true);
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *topItem = this->topLevelItem(i);
        if (topItem->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt() & requiredProblemTypeMask)
        {
            topItem->setCheckState(PROBLEM_TREE_COLUMN_NAME,Qt::Checked);
        }
    }
    this->blockSignals(signalsWereBlocked);
}

void ProblemSelectorTree::uncheckTypesWithoutRequiredTypes()
{
    const bool signalsWereBlocked = this->blockSignals(true);

    bool found = true;
    while (found)
    {
        found = false;
        const RProblemTypeMask checkedTypeMask = this->findProblemTypeMask();

        for (int i=0;i<this->topLevelItemCount();i++)
        {
            QTreeWidgetItem *topItem = this->topLevelItem(i);
            const RProblemTypeMask problemType = topItem->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt();

            if ((checkedTypeMask & problemType) && (RProblem::getRequiredProblemTypeMask(problemType) & ~checkedTypeMask))
            {
                topItem->setCheckState(PROBLEM_TREE_COLUMN_NAME,Qt::Unchecked);
                found = true;
            }
        }
    }

    this->blockSignals(signalsWereBlocked);
}

void ProblemSelectorTree::onTreeWidgetItemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(PROBLEM_TREE_COLUMN_NAME) == Qt::Unchecked)
    {
        this->uncheckTypesWithoutRequiredTypes();
    }
    else
    {
        this->checkCheckedTreeWidgetItem(item);
    }

    this->updateItemStates();

    emit this->changed();
}
