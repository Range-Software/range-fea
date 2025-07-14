#include <QMessageBox>

#include "application.h"
#include "problem_selector_tree.h"

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
    headerItem->setText(PROBLEM_TREE_COLUMN_NAME,QString("Name"));
    headerItem->setText(PROBLEM_TREE_COLUMN_DESC,QString("Description"));
    this->setHeaderItem(headerItem);

    this->resizeColumnToContents(PROBLEM_TREE_COLUMN_NAME);

    std::vector<RProblemType> prbList(RProblem::getTypes(R_PROBLEM_ALL));

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

    QObject::connect(this,&QTreeWidget::itemChanged,this,&ProblemSelectorTree::onTreeWidgetItemChanged);
}

RProblemTypeMask ProblemSelectorTree::findProblemTypeMask()
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

bool ProblemSelectorTree::checkProblemIsChecked() const
{
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *topItem = this->topLevelItem(i);
        Qt::CheckState checkState = topItem->checkState(PROBLEM_TREE_COLUMN_NAME);
        if (checkState == Qt::Checked || checkState == Qt::PartiallyChecked)
        {
            return true;
        }
    }
    return false;
}

void ProblemSelectorTree::checkCheckedTreeWidgetItem(QTreeWidgetItem *item)
{
    RProblemType problemType = RProblemType(item->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt());

    RProblemTypeMask excludedProblemTypeMask = RProblem::getExcludedProblemTypeMask(problemType);
    RProblemTypeMask requiredProblemTypeMask = RProblem::getRequiredProblemTypeMask(problemType);

    QList<RProblemType> excludedProblemTypes;

    int nItems = this->topLevelItemCount();

    for (int i=0;i<nItems;i++)
    {
        QTreeWidgetItem *topItem = this->topLevelItem(i);
        Qt::CheckState checkState = topItem->checkState(PROBLEM_TREE_COLUMN_NAME);
        if (checkState == Qt::Checked || checkState == Qt::PartiallyChecked)
        {
            RProblemType selectedProblemType = RProblemType(topItem->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt());
            if (selectedProblemType & excludedProblemTypeMask)
            {
                excludedProblemTypes.append(selectedProblemType);
            }
        }
        else
        {
            RProblemType unselectedProblemType = RProblemType(topItem->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt());
            if (unselectedProblemType & requiredProblemTypeMask)
            {
                this->blockSignals(true);
                topItem->setCheckState(PROBLEM_TREE_COLUMN_NAME,Qt::Checked);
                this->blockSignals(false);
            }
        }
    }

    if (excludedProblemTypes.size() > 0)
    {
        QString message;

        message += "Problem type <b>" + RProblem::getName(problemType) + "</b> is incompatible with following selected problem types:";
        for (int i=0;i<excludedProblemTypes.size();i++)
        {
            message += "<br/>- <b>" + RProblem::getName(excludedProblemTypes[i]) + "</b>";
        }

        QMessageBox::warning(this,tr("Incompatible problems selected"),message);
        this->blockSignals(true);
        item->setCheckState(PROBLEM_TREE_COLUMN_NAME,Qt::Unchecked);
        this->blockSignals(false);

        return;
    }
}

void ProblemSelectorTree::checkUncheckedTreeWidgetItem(QTreeWidgetItem *item)
{
    RProblemType problemType = RProblemType(item->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt());

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *topItem = this->topLevelItem(i);
        Qt::CheckState checkState = topItem->checkState(PROBLEM_TREE_COLUMN_NAME);
        if (checkState == Qt::Checked || checkState == Qt::PartiallyChecked)
        {
            RProblemType selectedProblemType = RProblemType(topItem->data(PROBLEM_TREE_COLUMN_NAME,Qt::UserRole).toInt());
            RProblemTypeMask requiredProblemTypeMask = RProblem::getRequiredProblemTypeMask(selectedProblemType);

            if (problemType & requiredProblemTypeMask)
            {
                topItem->setCheckState(PROBLEM_TREE_COLUMN_NAME,Qt::Unchecked);
            }
        }
    }
}

void ProblemSelectorTree::onTreeWidgetItemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(PROBLEM_TREE_COLUMN_NAME) == Qt::Unchecked)
    {
        this->checkUncheckedTreeWidgetItem(item);
    }
    else
    {
        this->checkCheckedTreeWidgetItem(item);
    }

    emit this->changed();
}
