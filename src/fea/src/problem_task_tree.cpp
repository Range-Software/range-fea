#include <QGridLayout>
#include <QVBoxLayout>

#include "problem_selector_dialog.h"
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

    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_NAME);
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_VALUE);

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

    this->deleteButton = new QPushButton("Remove");
    this->deleteButton->setDisabled(true);
    buttonsLayout->addWidget(this->deleteButton);

    QObject::connect(this->deleteButton,&QPushButton::clicked,this,&ProblemTaskTree::onDeleteButtonClicked);

    QPushButton *addProblem = new QPushButton(tr("Add problem type"));
    mainLayout->addWidget(addProblem,1,0,1,2);

    QObject::connect(addProblem,&QPushButton::clicked,this,&ProblemTaskTree::onAddProblemButtonClicked);
}

RProblemTaskItem ProblemTaskTree::getProblemTaskTree() const
{
    RProblemTaskItem taskTree;

    ProblemTaskTree::addWidgetItemToTree(taskTree,this->treeWidget->topLevelItem(0));

    return taskTree.getChild(0);
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

    if (taskItem.getProblemType() == R_PROBLEM_NONE)
    {
        item->setText(ProblemTaskTree::C_NAME,"# of iterations:");
        item->setText(ProblemTaskTree::C_VALUE,QLocale().toString(taskItem.getNIterations()));

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

    RProblemTaskItem newItem(problemType);
    newItem.setNIterations(nIterations);

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
    if (column != ProblemTaskTree::C_VALUE)
    {
        return;
    }

    bool ok;

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
}

void ProblemTaskTree::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    this->treeWidget->blockSignals(true);

    bool isEditable = false;

    if (column == ProblemTaskTree::C_VALUE)
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
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_NAME);
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_VALUE);

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
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_NAME);
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_VALUE);

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

    newItem->setData(ProblemTaskTree::C_NAME,Qt::UserRole,QVariant(R_PROBLEM_NONE));
    newItem->setData(ProblemTaskTree::C_VALUE,Qt::UserRole,QVariant(1));

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
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_NAME);
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_VALUE);

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
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_NAME);
    this->treeWidget->resizeColumnToContents(ProblemTaskTree::C_VALUE);

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

    emit this->changed();
}

void ProblemTaskTree::onAddProblemButtonClicked()
{
    ProblemSelectorDialog problemSelectorDialog(this);
    RProblemTypeMask problemTypeMask = problemSelectorDialog.exec();

    QTreeWidgetItem *item = this->treeWidget->topLevelItem(this->treeWidget->topLevelItemCount()-1);

    std::vector<RProblemType> problemTypes = RProblem::getTypes(problemTypeMask);
    RProblem::sortTypesByDependency(problemTypes);
    for (uint i=0;i<problemTypes.size();i++)
    {
        RProblemTaskItem taskItem(problemTypes[i]);
        this->addTaskItemToWidget(item,taskItem);
    }
}
