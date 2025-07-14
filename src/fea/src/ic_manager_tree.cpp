#include <vector>
#include <QList>
#include <QMessageBox>

#include "application.h"
#include "ic_manager_tree.h"

typedef enum _InitialConditionTreeColumn
{
    IC_TREE_COLUMN_NAME = 0,
    IC_TREE_N_COLUMNS
} InitialConditionTreeColumn;

ICManagerTree::ICManagerTree(QObject *parent) :
    QTreeWidget((QWidget*)parent),
    doNotPopulate(false)
{
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setSortingEnabled(true);
    this->sortByColumn(IC_TREE_COLUMN_NAME,Qt::AscendingOrder);

    this->setColumnCount(IC_TREE_N_COLUMNS);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(IC_TREE_COLUMN_NAME,QString("Name"));
    this->setHeaderItem(headerItem);
    this->setHeaderHidden(true);
    this->setRootIsDecorated(false);

    QObject::connect(Application::instance()->getSession(),&Session::problemSelectionChanged,this,&ICManagerTree::onProblemChanged);
    QObject::connect(Application::instance()->getSession(),&Session::modelSelectionChanged,this,&ICManagerTree::onModelSelectionChanged);
    QObject::connect(this,&ICManagerTree::itemChanged,this,&ICManagerTree::onItemChanged);
    QObject::connect(this,&ICManagerTree::itemSelectionChanged,this,&ICManagerTree::onItemSelectionChanged);
    QObject::connect(this,&ICManagerTree::icSelected,this,&ICManagerTree::onIcSelected);
}

void ICManagerTree::populate()
{
    if (this->doNotPopulate)
    {
        return;
    }

    RProblemTypeMask icPrbTypeMask;
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
        this->selectedTypes.append(RInitialConditionType(selectedItemsList[i]->data(IC_TREE_COLUMN_NAME,Qt::UserRole).toInt()));
    }

    this->blockSignals(true);

    this->clear();

    QMap<RInitialConditionType,uint> icMap = this->getIcMap(selectedEntityIDs);

    for (RInitialConditionType icType = (RInitialConditionType)((int)R_INITIAL_CONDITION_NONE + 1);
                               icType < R_INITIAL_CONDITION_N_TYPES;
                               icType ++)
    {
        icPrbTypeMask = RInitialCondition::getProblemTypeMask(icType);
        if (!(icPrbTypeMask & modelPrbTypeMask))
        {
            continue;
        }
        bool applies = false;
        if (elementGroupTypeMask & R_ENTITY_GROUP_POINT ||
            elementGroupTypeMask & R_ENTITY_GROUP_LINE ||
            elementGroupTypeMask & R_ENTITY_GROUP_SURFACE ||
            elementGroupTypeMask & R_ENTITY_GROUP_VOLUME)
        {
            applies = true;
        }
        if (!applies)
        {
            continue;
        }

        Qt::CheckState state = Qt::Unchecked;
        if (icMap.contains(icType))
        {
            if (icMap.value(icType,1) == (uint)selectedEntityIDs.size())
            {
                state = Qt::Checked;
            }
            else
            {
                state = Qt::PartiallyChecked;
            }
        }
        QTreeWidgetItem *itemModel = new QTreeWidgetItem(this);
        itemModel->setText(IC_TREE_COLUMN_NAME, RInitialCondition::getName(icType));
        itemModel->setData(IC_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(state));
        itemModel->setData(IC_TREE_COLUMN_NAME,Qt::UserRole,QVariant(icType));
    }

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        this->topLevelItem(i)->setSelected(false);
        for (int j=0;j<this->selectedTypes.size();j++)
        {
            if (this->selectedTypes[j] == RInitialConditionType(this->topLevelItem(i)->data(IC_TREE_COLUMN_NAME,Qt::UserRole).toInt()))
            {
                this->topLevelItem(i)->setSelected(true);
                break;
            }
        }
    }

    this->blockSignals(false);

    emit this->itemSelectionChanged();
}

void ICManagerTree::onModelSelectionChanged(uint)
{
    this->populate();
}

void ICManagerTree::onProblemChanged(uint)
{
    this->populate();
}

QMap<RInitialConditionType,uint> ICManagerTree::getIcMap(const QList<SessionEntityID> &entityIDs) const
{
    QMap<RInitialConditionType,uint> icMap;
    RInitialConditionType icType;
    uint icCount;

    for (int i=0;i<entityIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(entityIDs[i].getMid());

        if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT)
        {
            for (uint j=0;j<pModel->getPoint(entityIDs[i].getEid()).getNInitialConditions();j++)
            {
                icType = pModel->getPoint(entityIDs[i].getEid()).getInitialCondition(j).getType();
                icCount = icMap.value(icType,0) + 1;
                icMap.insert(icType,icCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE)
        {
            for (uint j=0;j<pModel->getLine(entityIDs[i].getEid()).getNInitialConditions();j++)
            {
                icType = pModel->getLine(entityIDs[i].getEid()).getInitialCondition(j).getType();
                icCount = icMap.value(icType,0) + 1;
                icMap.insert(icType,icCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE)
        {
            for (uint j=0;j<pModel->getSurface(entityIDs[i].getEid()).getNInitialConditions();j++)
            {
                icType = pModel->getSurface(entityIDs[i].getEid()).getInitialCondition(j).getType();
                icCount = icMap.value(icType,0) + 1;
                icMap.insert(icType,icCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_VOLUME)
        {
            for (uint j=0;j<pModel->getVolume(entityIDs[i].getEid()).getNInitialConditions();j++)
            {
                icType = pModel->getVolume(entityIDs[i].getEid()).getInitialCondition(j).getType();
                icCount = icMap.value(icType,0) + 1;
                icMap.insert(icType,icCount);
            }
        }
    }

    return icMap;
}

void ICManagerTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    Qt::CheckState checkState = item->checkState(column);
    RInitialConditionType icType = (RInitialConditionType) item->data(IC_TREE_COLUMN_NAME,Qt::UserRole).toInt();
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(entityIDs[i].getMid());
        if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getPoint(entityIDs[i].getEid()).addInitialCondition(RInitialCondition(icType));
            }
            else
            {
                if (pModel->getPoint(entityIDs[i].getEid()).hasInitialCondition(icType))
                {
                    pModel->getPoint(entityIDs[i].getEid()).removeInitialCondition(icType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getLine(entityIDs[i].getEid()).addInitialCondition(RInitialCondition(icType));
            }
            else
            {
                if (pModel->getLine(entityIDs[i].getEid()).hasInitialCondition(icType))
                {
                    pModel->getLine(entityIDs[i].getEid()).removeInitialCondition(icType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getSurface(entityIDs[i].getEid()).addInitialCondition(RInitialCondition(icType));
            }
            else
            {
                if (pModel->getSurface(entityIDs[i].getEid()).hasInitialCondition(icType))
                {
                    pModel->getSurface(entityIDs[i].getEid()).removeInitialCondition(icType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_VOLUME)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getVolume(entityIDs[i].getEid()).addInitialCondition(RInitialCondition(icType));
            }
            else
            {
                if (pModel->getVolume(entityIDs[i].getEid()).hasInitialCondition(icType))
                {
                    pModel->getVolume(entityIDs[i].getEid()).removeInitialCondition(icType);
                }
            }
        }
    }

    if (item->isSelected())
    {
        emit this->icSelected(icType,(checkState != Qt::Unchecked));
    }
    else
    {
        emit this->icSelected(R_INITIAL_CONDITION_NONE,false);
    }
}

void ICManagerTree::onItemSelectionChanged()
{
    QList<QTreeWidgetItem *> items = this->selectedItems();
    emit this->icSelected(R_INITIAL_CONDITION_NONE,false);
    for (int i=0;i<items.size();i++)
    {
        RInitialConditionType icType = (RInitialConditionType)items[i]->data(IC_TREE_COLUMN_NAME,Qt::UserRole).toInt();
        bool applied = (items[i]->checkState(IC_TREE_COLUMN_NAME) == Qt::Checked);
        emit this->icSelected(icType,applied);
    }
}

void ICManagerTree::onIcSelected()
{
    this->doNotPopulate = true;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<modelIDs.size();i++)
    {
        Application::instance()->getSession()->setProblemChanged(modelIDs[i]);
    }
    this->doNotPopulate = false;
}
