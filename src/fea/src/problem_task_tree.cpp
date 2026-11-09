#include <QGridLayout>
#include <QVBoxLayout>
#include <QTreeWidgetItemIterator>

#include "problem_task_tree.h"

ProblemTaskTree::ProblemTaskTree(const RProblemTaskItem &rTaskTree, QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *mainLayout = new QGridLayout;
    this->setLayout(mainLayout);

    this->treeWidget = new QTreeWidget;
    mainLayout->addWidget(this->treeWidget,0,0,1,1);

    this->treeWidget->setRootIsDecorated(true);
    this->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    this->treeWidget->setColumnCount(ProblemTaskTree::N_COLUMNS);
    this->treeWidget->setHeaderHidden(true);

    this->addTaskItemToWidget(0,rTaskTree);

    this->treeWidget->expandAll();

    this->resizeColumns();

    QObject::connect(this->treeWidget,&QTreeWidget::itemSelectionChanged,
                     this,&ProblemTaskTree::onItemSelectionChanged);

    QObject::connect(this->treeWidget,&QTreeWidget::itemChanged,
                     this,&ProblemTaskTree::onItemChanged);

    QObject::connect(this->treeWidget,&QTreeWidget::itemDoubleClicked,
                     this,&ProblemTaskTree::onItemDoubleClicked);

    QVBoxLayout *buttonsLayout = new QVBoxLayout;
    mainLayout->addLayout(buttonsLayout,0,1,1,1);

    this->upButton = new QPushButton(tr("Up"));
    this->upButton->setDisabled(true);
    buttonsLayout->addWidget(this->upButton);

    QObject::connect(this->upButton,&QPushButton::clicked,this,&ProblemTaskTree::onUpButtonClicked);

    this->downButton = new QPushButton(tr("Down"));
    this->downButton->setDisabled(true);
    buttonsLayout->addWidget(this->downButton);

    QObject::connect(this->downButton,&QPushButton::clicked,this,&ProblemTaskTree::onDownButtonClicked);

    this->indentButton = new QPushButton(">");
    this->indentButton->setDisabled(true);
    buttonsLayout->addWidget(this->indentButton);

    QObject::connect(this->indentButton,&QPushButton::clicked,this,&ProblemTaskTree::onIndentButtonClicked);

    this->unindentButton = new QPushButton("<");
    this->unindentButton->setDisabled(true);
    buttonsLayout->addWidget(this->unindentButton);

    QObject::connect(this->unindentButton,&QPushButton::clicked,this,&ProblemTaskTree::onUnindentButtonClicked);

    this->deleteButton = new QPushButton(tr("Remove"));
    this->deleteButton->setDisabled(true);
    buttonsLayout->addWidget(this->deleteButton);

    QObject::connect(this->deleteButton,&QPushButton::clicked,this,&ProblemTaskTree::onDeleteButtonClicked);
}

RProblemTaskItem ProblemTaskTree::getProblemTaskTree() const
{
    RProblemTaskItem taskTree;

    ProblemTaskTree::addWidgetItemToTree(taskTree,this->treeWidget->topLevelItem(0));

    return taskTree.getChild(0);
}

void ProblemTaskTree::setProblemTaskTree(const RProblemTaskItem &rTaskTree)
{
    const bool signalsWereBlocked = this->treeWidget->blockSignals(true);

    this->treeWidget->clear();
    this->addTaskItemToWidget(nullptr,rTaskTree);
    this->treeWidget->expandAll();

    this->treeWidget->blockSignals(signalsWereBlocked);

    this->resizeColumns();
    this->onItemSelectionChanged();
}

void ProblemTaskTree::addProblemTypes(RProblemTypeMask problemTypeMask)
{
    const bool signalsWereBlocked = this->treeWidget->blockSignals(true);

    QTreeWidgetItem *rootItem = this->treeWidget->topLevelItem(0);

    std::vector<RProblemType> problemTypes = RProblem::getTypes(problemTypeMask);
    RProblem::sortTypesByDependency(problemTypes);

    for (RProblemType problemType : problemTypes)
    {
        if (ProblemTaskTree::findItemTypeMask(rootItem) & problemType)
        {
            continue;
        }

        // Goes in front of the first task which uses its results.
        int position = rootItem->childCount();
        for (int i=0;i<rootItem->childCount() && position == rootItem->childCount();i++)
        {
            const RProblemTypeMask childTypeMask = ProblemTaskTree::findItemTypeMask(rootItem->child(i));
            if ((RProblem::getRequiredProblemTypeMask(childTypeMask) | RProblem::getSolveAfterProblemTypeMask(childTypeMask)) & problemType)
            {
                position = i;
            }
        }

        this->addTaskItemToWidget(rootItem,RProblemTaskItem(problemType));

        QTreeWidgetItem *item = rootItem->takeChild(rootItem->childCount()-1);
        rootItem->insertChild(position,item);
        item->setFirstColumnSpanned(true);
    }

    this->treeWidget->blockSignals(signalsWereBlocked);

    this->resizeColumns();
}

void ProblemTaskTree::removeProblemTypes(RProblemTypeMask problemTypeMask)
{
    bool found = true;
    while (found)
    {
        found = false;
        for (QTreeWidgetItemIterator it(this->treeWidget); *it; ++it)
        {
            if (RProblemType((*it)->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt()) & problemTypeMask)
            {
                this->removeItem(*it);
                found = true;
                break;
            }
        }
    }

    this->resizeColumns();
    this->onItemSelectionChanged();
}

RProblemTypeMask ProblemTaskTree::findItemTypeMask(const QTreeWidgetItem *item)
{
    RProblemTypeMask typeMask = RProblemTypeMask(item->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt());

    for (int i=0;i<item->childCount();i++)
    {
        typeMask |= ProblemTaskTree::findItemTypeMask(item->child(i));
    }

    return typeMask;
}

void ProblemTaskTree::removeTasksWithoutRequiredTypes()
{
    bool found = true;
    while (found)
    {
        found = false;
        const RProblemTypeMask typeMask = ProblemTaskTree::findItemTypeMask(this->treeWidget->topLevelItem(0));

        for (QTreeWidgetItemIterator it(this->treeWidget); *it; ++it)
        {
            if (RProblem::getRequiredProblemTypeMask((*it)->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt()) & ~typeMask)
            {
                this->removeItem(*it);
                found = true;
                break;
            }
        }
    }
}

void ProblemTaskTree::addTaskItemToWidget(QTreeWidgetItem *parent, const RProblemTaskItem &taskItem)
{
    QTreeWidgetItem *item;
    if (parent)
    {
        item = new QTreeWidgetItem(parent);
    }
    else
    {
        item = new QTreeWidgetItem(this->treeWidget);
    }

    item->setData(ProblemTaskTree::C_NAME,Qt::UserRole,QVariant(taskItem.getProblemType()));
    item->setData(ProblemTaskTree::C_VALUE,Qt::UserRole,QVariant(taskItem.getNIterations()));
    item->setData(ProblemTaskTree::C_CVG_VALUE,Qt::UserRole,QVariant(taskItem.getCvgValue()));

    if (taskItem.getProblemType() == R_PROBLEM_NONE)
    {
        item->setText(ProblemTaskTree::C_NAME,tr("# of iterations") + ":");
        item->setText(ProblemTaskTree::C_VALUE,QLocale().toString(taskItem.getNIterations()));
        item->setText(ProblemTaskTree::C_CVG_NAME,tr("Convergence") + ":");
        item->setText(ProblemTaskTree::C_CVG_VALUE,QLocale().toString(taskItem.getCvgValue(),'g',6));
        item->setToolTip(ProblemTaskTree::C_CVG_VALUE,
                         tr("The iterations end as soon as every task of the group has converged below this value.")
                         + " " + tr("Zero runs all of them."));

        for (uint i=0;i<taskItem.getNChildren();i++)
        {
            this->addTaskItemToWidget(item,taskItem.getChild(i));
        }
    }
    else
    {
        item->setText(ProblemTaskTree::C_NAME,RProblem::getName(taskItem.getProblemType()));
        item->setFirstColumnSpanned(true);
    }
}

void ProblemTaskTree::resizeColumns()
{
    for (int i=0;i<ProblemTaskTree::N_COLUMNS;i++)
    {
        this->treeWidget->resizeColumnToContents(i);
    }
}

void ProblemTaskTree::removeItem(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parentItem = item->parent();

    delete parentItem->takeChild(parentItem->indexOfChild(item));

    if (parentItem->childCount() == 0 && parentItem->parent())
    {
        this->removeItem(parentItem);
    }
}

void ProblemTaskTree::addWidgetItemToTree(RProblemTaskItem &taskItem, const QTreeWidgetItem *item)
{
    RProblemType problemType = RProblemType(item->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt());
    uint nIterations = item->data(ProblemTaskTree::C_VALUE,Qt::UserRole).toUInt();
    double cvgValue = item->data(ProblemTaskTree::C_CVG_VALUE,Qt::UserRole).toDouble();

    RProblemTaskItem newItem(problemType);
    newItem.setNIterations(nIterations);
    newItem.setCvgValue(cvgValue);

    if (problemType == R_PROBLEM_NONE)
    {
        for (int i=0;i<item->childCount();i++)
        {
            ProblemTaskTree::addWidgetItemToTree(newItem,item->child(i));
        }
    }

    taskItem.addChild(newItem);
}

void ProblemTaskTree::onItemSelectionChanged()
{
    QList<QTreeWidgetItem*> selectedItems = this->treeWidget->selectedItems();

    if (selectedItems.size() != 1)
    {
        this->upButton->setDisabled(true);
        this->downButton->setDisabled(true);
        this->indentButton->setDisabled(true);
        this->unindentButton->setDisabled(true);
        this->deleteButton->setDisabled(true);
        return;
    }

    bool canGoUp = true;
    bool canGoDown = true;
    bool canIndent = true;
    bool canUnindent = true;
    bool canDelete = true;


    if (selectedItems[0]->parent())
    {
        if (!selectedItems[0]->parent()->parent())
        {
            canUnindent = false;
            if (selectedItems[0]->parent()->indexOfChild(selectedItems[0]) == 0)
            {
                canGoUp = false;
            }
            if (selectedItems[0]->parent()->indexOfChild(selectedItems[0]) == selectedItems[0]->parent()->childCount() - 1)
            {
                canGoDown = false;
            }
        }
        canDelete = true;
    }
    else
    {
        canGoUp = false;
        canGoDown = false;
        canIndent = false;
        canUnindent = false;
        canDelete = false;
    }

    this->upButton->setEnabled(canGoUp);
    this->downButton->setEnabled(canGoDown);
    this->indentButton->setEnabled(canIndent);
    this->unindentButton->setEnabled(canUnindent);
    this->deleteButton->setEnabled(canDelete);
}

void ProblemTaskTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    bool ok;

    if (column == ProblemTaskTree::C_VALUE)
    {
        uint nIterations = item->text(column).toUInt(&ok);

        if (ok && nIterations > 0)
        {
            item->setData(ProblemTaskTree::C_VALUE,Qt::UserRole,nIterations);
            emit this->changed();
        }
        else
        {
            nIterations = item->data(ProblemTaskTree::C_VALUE,Qt::UserRole).toUInt();
            item->setText(ProblemTaskTree::C_VALUE,QLocale().toString(nIterations));
        }
        return;
    }

    if (column == ProblemTaskTree::C_CVG_VALUE)
    {
        double cvgValue = item->text(column).toDouble(&ok);

        if (ok && cvgValue >= 0.0)
        {
            item->setData(ProblemTaskTree::C_CVG_VALUE,Qt::UserRole,cvgValue);
            emit this->changed();
        }
        else
        {
            cvgValue = item->data(ProblemTaskTree::C_CVG_VALUE,Qt::UserRole).toDouble();
            item->setText(ProblemTaskTree::C_CVG_VALUE,QLocale().toString(cvgValue,'g',6));
        }
        return;
    }
}

void ProblemTaskTree::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    this->treeWidget->blockSignals(true);

    bool isEditable = false;

    if (column == ProblemTaskTree::C_VALUE || column == ProblemTaskTree::C_CVG_VALUE)
    {
        isEditable = (RProblemType(item->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt()) == R_PROBLEM_NONE);
    }

    if (isEditable)
    {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    else
    {
        if (item->flags() & Qt::ItemIsEditable)
        {
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        }
    }
    this->treeWidget->blockSignals(false);
}

void ProblemTaskTree::onUpButtonClicked()
{
    QList<QTreeWidgetItem*> selectedItems = this->treeWidget->selectedItems();

    if (selectedItems.size() != 1)
    {
        return;
    }

    QTreeWidgetItem *item = selectedItems[0];
    QTreeWidgetItem *parent = item->parent();
    int index = parent->indexOfChild(item);
    if (index == 0)
    {
        QTreeWidgetItem *topParent = parent->parent();
        if (!topParent)
        {
            return;
        }
        int parentIndex = topParent->indexOfChild(parent);
        item = parent->takeChild(index);
        if (parent->childCount() == 0)
        {
            delete topParent->takeChild(parentIndex);
        }
        topParent->insertChild(parentIndex,item);
    }
    else
    {
        item = parent->takeChild(index);
        if (parent->child(index-1)->childCount() > 0)
        {
            parent->child(index-1)->addChild(item);
        }
        else
        {
            parent->insertChild(index-1,item);
        }
    }

    if (RProblemType(item->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt()) != R_PROBLEM_NONE)
    {
        item->setFirstColumnSpanned(true);
    }
    if (item->childCount() > 0)
    {
        item->setExpanded(true);
    }

    this->treeWidget->setCurrentItem(item);
    this->resizeColumns();

    emit this->changed();
}

void ProblemTaskTree::onDownButtonClicked()
{
    QList<QTreeWidgetItem*> selectedItems = this->treeWidget->selectedItems();

    if (selectedItems.size() != 1)
    {
        return;
    }

    QTreeWidgetItem *item = selectedItems[0];
    QTreeWidgetItem *parent = item->parent();
    int index = parent->indexOfChild(item);
    if (index == parent->childCount()-1)
    {
        QTreeWidgetItem *topParent = parent->parent();
        if (!topParent)
        {
            return;
        }
        int parentIndex = topParent->indexOfChild(parent);
        item = parent->takeChild(index);
        topParent->insertChild(parentIndex+1,item);
        if (parent->childCount() == 0)
        {
            delete topParent->takeChild(parentIndex);
        }
    }
    else
    {
        item = parent->takeChild(index);
        if (parent->child(index)->childCount() > 0)
        {
            parent->child(index)->insertChild(0,item);
        }
        else
        {
            parent->insertChild(index+1,item);
        }
    }

    if (RProblemType(item->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt()) != R_PROBLEM_NONE)
    {
        item->setFirstColumnSpanned(true);
    }
    if (item->childCount() > 0)
    {
        item->setExpanded(true);
    }

    this->treeWidget->setCurrentItem(item);
    this->resizeColumns();

    emit this->changed();
}

void ProblemTaskTree::onIndentButtonClicked()
{
    QList<QTreeWidgetItem*> selectedItems = this->treeWidget->selectedItems();

    if (selectedItems.size() != 1)
    {
        return;
    }

    QTreeWidgetItem *item = selectedItems[0];
    QTreeWidgetItem *parent = item->parent();
    int index = parent->indexOfChild(item);
    item = parent->takeChild(index);
    QTreeWidgetItem *newItem = new QTreeWidgetItem;

    newItem->setText(ProblemTaskTree::C_NAME,"# of iterations:");
    newItem->setText(ProblemTaskTree::C_VALUE,QLocale().toString(1));
    newItem->setText(ProblemTaskTree::C_CVG_NAME,tr("Convergence") + ":");
    newItem->setText(ProblemTaskTree::C_CVG_VALUE,QLocale().toString(RProblemTaskItem::defaultCvgValue,'g',6));

    newItem->setData(ProblemTaskTree::C_NAME,Qt::UserRole,QVariant(R_PROBLEM_NONE));
    newItem->setData(ProblemTaskTree::C_VALUE,Qt::UserRole,QVariant(1));
    newItem->setData(ProblemTaskTree::C_CVG_VALUE,Qt::UserRole,QVariant(RProblemTaskItem::defaultCvgValue));

    parent->insertChild(index,newItem);
    newItem->addChild(item);

    newItem->setExpanded(true);

    if (RProblemType(item->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt()) != R_PROBLEM_NONE)
    {
        item->setFirstColumnSpanned(true);
    }
    if (item->childCount() > 0)
    {
        item->setExpanded(true);
    }

    this->treeWidget->setCurrentItem(item);
    this->resizeColumns();

    emit this->changed();
}

void ProblemTaskTree::onUnindentButtonClicked()
{
    QList<QTreeWidgetItem*> selectedItems = this->treeWidget->selectedItems();

    if (selectedItems.size() != 1)
    {
        return;
    }

    QTreeWidgetItem *item = selectedItems[0];
    QTreeWidgetItem *parent = item->parent();
    QTreeWidgetItem *topParent = parent->parent();
    int index = parent->indexOfChild(item);
    int parentIndex = topParent->indexOfChild(parent);
    item = parent->takeChild(index);
    if (parent->childCount() == 0)
    {
        delete topParent->takeChild(parentIndex);
    }
    topParent->insertChild(parentIndex,item);

    if (RProblemType(item->data(ProblemTaskTree::C_NAME,Qt::UserRole).toInt()) != R_PROBLEM_NONE)
    {
        item->setFirstColumnSpanned(true);
    }
    if (item->childCount() > 0)
    {
        item->setExpanded(true);
    }

    this->treeWidget->setCurrentItem(item);
    this->resizeColumns();

    emit this->changed();
}

void ProblemTaskTree::onDeleteButtonClicked()
{
    QList<QTreeWidgetItem*> selectedItems = this->treeWidget->selectedItems();

    if (selectedItems.size() != 1)
    {
        return;
    }

    this->removeItem(selectedItems[0]);
    this->removeTasksWithoutRequiredTypes();

    emit this->changed();
}
