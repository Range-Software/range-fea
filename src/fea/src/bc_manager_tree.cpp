#include <QList>
#include <QMessageBox>

#include "bc_manager_tree.h"
#include "application.h"

typedef enum _BoundaryConditionTreeColumn
{
    BC_TREE_COLUMN_NAME = 0,
    BC_TREE_N_COLUMNS
} BoundaryConditionTreeColumn;

BCManagerTree::BCManagerTree(QWidget *parent) :
    QTreeWidget(parent),
    doNotPopulate(false)
{
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setSortingEnabled(true);
    this->sortByColumn(BC_TREE_COLUMN_NAME,Qt::AscendingOrder);

    this->setColumnCount(BC_TREE_N_COLUMNS);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(BC_TREE_COLUMN_NAME,QString("Name"));
    this->setHeaderItem(headerItem);
    this->setHeaderHidden(true);
    this->setRootIsDecorated(false);

    QObject::connect(Application::instance()->getSession(),
                     &Session::problemSelectionChanged,
                     this,
                     &BCManagerTree::onProblemChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelSelectionChanged,
                     this,
                     &BCManagerTree::onModelSelectionChanged);
    QObject::connect(this,&BCManagerTree::itemChanged,
                     this,&BCManagerTree::onItemChanged);
    QObject::connect(this,&BCManagerTree::itemSelectionChanged,
                     this,&BCManagerTree::onItemSelectionChanged);
    QObject::connect(this,&BCManagerTree::bcSelected,
                     this,&BCManagerTree::onBcSelected);
}

void BCManagerTree::populate()
{
    if (this->doNotPopulate)
    {
        return;
    }

    RProblemTypeMask bcPrbTypeMask;
    RProblemTypeMask modelPrbTypeMask = R_PROBLEM_NONE;
    REntityGroupTypeMask elementGroupTypeMask = R_ENTITY_GROUP_NONE;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<selectedEntityIDs.size();i++)
    {
        elementGroupTypeMask |= selectedEntityIDs[i].getType();
        modelPrbTypeMask |= Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getProblemTaskTree().getProblemTypeMask();
    }

    QList<QTreeWidgetItem*> selectedItemsList = this->selectedItems();
    // Following is a hack to not to clear selected types list if item is being deselected.
    if (selectedItemsList.size() > 0)
    {
        this->selectedTypes.clear();
    }
    for (int i=0;i<selectedItemsList.size();i++)
    {
        this->selectedTypes.append(RBoundaryConditionType(selectedItemsList[i]->data(BC_TREE_COLUMN_NAME,Qt::UserRole).toInt()));
    }

    this->blockSignals(true);

    this->clear();

    QMap<RBoundaryConditionType,uint> bcMap = this->getBcMap(selectedEntityIDs);

    for (RBoundaryConditionType bcType = (RBoundaryConditionType)((int)R_BOUNDARY_CONDITION_NONE + 1);
                                bcType < R_BOUNDARY_CONDITION_N_TYPES;
                                bcType ++)
    {
        bcPrbTypeMask = RBoundaryCondition::getProblemTypeMask(bcType);
        if (!(bcPrbTypeMask & modelPrbTypeMask))
        {
            continue;
        }
        bool applies = false;
        if (elementGroupTypeMask & R_ENTITY_GROUP_POINT)
        {
            if (RBoundaryCondition::getApplyOnPoint(bcType))
            {
                applies = true;
            }
        }
        if (elementGroupTypeMask & R_ENTITY_GROUP_LINE)
        {
            if (RBoundaryCondition::getApplyOnLine(bcType))
            {
                applies = true;
            }
        }
        if (elementGroupTypeMask & R_ENTITY_GROUP_SURFACE)
        {
            if (RBoundaryCondition::getApplyOnSurface(bcType))
            {
                applies = true;
            }
        }
        if (elementGroupTypeMask & R_ENTITY_GROUP_VOLUME)
        {
            if (RBoundaryCondition::getApplyOnVolume(bcType))
            {
                applies = true;
            }
        }
        if (!applies)
        {
            continue;
        }

        Qt::CheckState state = Qt::Unchecked;
        if (bcMap.contains(bcType))
        {
            if (bcMap.value(bcType,1) == (uint)selectedEntityIDs.size())
            {
                state = Qt::Checked;
            }
            else
            {
                state = Qt::PartiallyChecked;
            }
        }
        QTreeWidgetItem *itemModel = new QTreeWidgetItem(this);
        if (RBoundaryCondition::getExplicit(bcType))
        {
            itemModel->setForeground(BC_TREE_COLUMN_NAME,this->palette().brush(QPalette::Active,QPalette::Text));
            QFont font(itemModel->font(BC_TREE_COLUMN_NAME));
            font.setBold(true);
            itemModel->setFont(BC_TREE_COLUMN_NAME,font);
        }
        itemModel->setText(BC_TREE_COLUMN_NAME, RBoundaryCondition::getName(bcType));
        itemModel->setData(BC_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(state));
        itemModel->setData(BC_TREE_COLUMN_NAME,Qt::UserRole,QVariant(bcType));
    }

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        this->topLevelItem(i)->setSelected(false);
        for (int j=0;j<this->selectedTypes.size();j++)
        {
            if (this->selectedTypes[j] == RBoundaryConditionType(this->topLevelItem(i)->data(BC_TREE_COLUMN_NAME,Qt::UserRole).toInt()))
            {
                this->topLevelItem(i)->setSelected(true);
                break;
            }
        }
    }

    this->blockSignals(false);

    emit this->itemSelectionChanged();
}
void BCManagerTree::onModelSelectionChanged(uint)
{
    this->populate();
}

void BCManagerTree::onProblemChanged(uint)
{
    this->populate();
}

QMap<RBoundaryConditionType,uint> BCManagerTree::getBcMap(const QList<SessionEntityID> &entityIDs) const
{
    QMap<RBoundaryConditionType,uint> bcMap;
    RBoundaryConditionType bcType;
    uint bcCount;

    for (int i=0;i<entityIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(entityIDs[i].getMid());

        if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT)
        {
            for (uint j=0;j<pModel->getPoint(entityIDs[i].getEid()).getNBoundaryConditions();j++)
            {
                bcType = pModel->getPoint(entityIDs[i].getEid()).getBoundaryCondition(j).getType();
                bcCount = bcMap.value(bcType,0) + 1;
                bcMap.insert(bcType,bcCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE)
        {
            for (uint j=0;j<pModel->getLine(entityIDs[i].getEid()).getNBoundaryConditions();j++)
            {
                bcType = pModel->getLine(entityIDs[i].getEid()).getBoundaryCondition(j).getType();
                bcCount = bcMap.value(bcType,0) + 1;
                bcMap.insert(bcType,bcCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE)
        {
            for (uint j=0;j<pModel->getSurface(entityIDs[i].getEid()).getNBoundaryConditions();j++)
            {
                bcType = pModel->getSurface(entityIDs[i].getEid()).getBoundaryCondition(j).getType();
                bcCount = bcMap.value(bcType,0) + 1;
                bcMap.insert(bcType,bcCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_VOLUME)
        {
            for (uint j=0;j<pModel->getVolume(entityIDs[i].getEid()).getNBoundaryConditions();j++)
            {
                bcType = pModel->getVolume(entityIDs[i].getEid()).getBoundaryCondition(j).getType();
                bcCount = bcMap.value(bcType,0) + 1;
                bcMap.insert(bcType,bcCount);
            }
        }
    }

    return bcMap;
}

bool BCManagerTree::resolveConflicts(RBoundaryConditionType bcType)
{
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    RProblemTypeMask problemTypeMask = R_PROBLEM_NONE;
    for (int i=0;i<modelIDs.size();i++)
    {
        problemTypeMask |= Application::instance()->getSession()->getModel(modelIDs[i]).getProblemTaskTree().getProblemTypeMask();
    }
    std::vector<RBoundaryConditionType> bcTypeConflicts = RBoundaryCondition::getConflictTypes(problemTypeMask,bcType);

    QString message = "Following boundary condition(s) will be deselected:<ul>";
    uint nConflicts = 0;
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->topLevelItem(i);
        if (item->checkState(BC_TREE_COLUMN_NAME) != Qt::Checked)
        {
            continue;
        }
        for (uint j=0;j<bcTypeConflicts.size();j++)
        {
            if (bcTypeConflicts[j] == RBoundaryConditionType(item->data(BC_TREE_COLUMN_NAME,Qt::UserRole).toInt()))
            {
                message += "<li><b>" + item->text(BC_TREE_COLUMN_NAME) + "</b></li>";
                nConflicts++;
            }
        }
    }
    message += "</ul>";

    if (nConflicts == 0)
    {
        return true;
    }

    int response = QMessageBox::question(this,
                                         "Remove boundary condition(s)?",
                                         message,
                                         QMessageBox::Ok | QMessageBox::Cancel,
                                         QMessageBox::Cancel);
    if (response == QMessageBox::Cancel)
    {
        return false;
    }

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->topLevelItem(i);
        for (uint j=0;j<bcTypeConflicts.size();j++)
        {
            if (bcTypeConflicts[j] == RBoundaryConditionType(item->data(BC_TREE_COLUMN_NAME,Qt::UserRole).toInt()))
            {
                item->setCheckState(BC_TREE_COLUMN_NAME,Qt::Unchecked);
            }
        }
    }
    return true;
}

void BCManagerTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    Qt::CheckState checkState = item->checkState(column);
    RBoundaryConditionType bcType = (RBoundaryConditionType) item->data(BC_TREE_COLUMN_NAME,Qt::UserRole).toInt();
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    if (checkState == Qt::Checked)
    {
        if (!this->resolveConflicts(bcType))
        {
            this->blockSignals(true);
            item->setCheckState(column,Qt::Unchecked);
            this->blockSignals(false);
            return;
        }
    }

    for (int i=0;i<entityIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(entityIDs[i].getMid());
        RBoundaryCondition defaultBoundaryCondition = pModel->generateDefaultBoundayCondition(bcType,entityIDs[i].getType(),entityIDs[i].getEid());

        if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getPoint(entityIDs[i].getEid()).addBoundaryCondition(defaultBoundaryCondition);
            }
            else
            {
                if (pModel->getPoint(entityIDs[i].getEid()).hasBoundaryCondition(bcType))
                {
                    pModel->getPoint(entityIDs[i].getEid()).removeBoundaryCondition(bcType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getLine(entityIDs[i].getEid()).addBoundaryCondition(defaultBoundaryCondition);
            }
            else
            {
                if (pModel->getLine(entityIDs[i].getEid()).hasBoundaryCondition(bcType))
                {
                    pModel->getLine(entityIDs[i].getEid()).removeBoundaryCondition(bcType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getSurface(entityIDs[i].getEid()).addBoundaryCondition(defaultBoundaryCondition);
            }
            else
            {
                if (pModel->getSurface(entityIDs[i].getEid()).hasBoundaryCondition(bcType))
                {
                    pModel->getSurface(entityIDs[i].getEid()).removeBoundaryCondition(bcType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_VOLUME)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getVolume(entityIDs[i].getEid()).addBoundaryCondition(defaultBoundaryCondition);
            }
            else
            {
                if (pModel->getVolume(entityIDs[i].getEid()).hasBoundaryCondition(bcType))
                {
                    pModel->getVolume(entityIDs[i].getEid()).removeBoundaryCondition(bcType);
                }
            }
        }
    }

    if (item->isSelected())
    {
        emit this->bcSelected(bcType,(checkState == Qt::Checked));
    }
    else
    {
        emit this->bcSelected(R_BOUNDARY_CONDITION_NONE,false);
    }
}

void BCManagerTree::onItemSelectionChanged()
{
    QList<QTreeWidgetItem *> items = this->selectedItems();
    emit this->bcSelected(R_BOUNDARY_CONDITION_NONE,false);
    for (int i=0;i<items.size();i++)
    {
        RBoundaryConditionType bcType = (RBoundaryConditionType)items[i]->data(BC_TREE_COLUMN_NAME,Qt::UserRole).toInt();
        bool applied = (items[i]->checkState(BC_TREE_COLUMN_NAME) == Qt::Checked);
        emit this->bcSelected(bcType,applied);
    }
}

void BCManagerTree::onBcSelected()
{
    this->doNotPopulate = true;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<modelIDs.size();i++)
    {
        Application::instance()->getSession()->setProblemChanged(modelIDs[i]);
    }
    this->doNotPopulate = false;
}
