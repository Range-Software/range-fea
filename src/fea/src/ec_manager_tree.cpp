#include <QList>
#include <QMessageBox>

#include "application.h"
#include "ec_manager_tree.h"

typedef enum _EnvironmentConditionTreeColumn
{
    EC_TREE_COLUMN_NAME = 0,
    EC_TREE_N_COLUMNS
} EnvironmentConditionTreeColumn;

ECManagerTree::ECManagerTree(QObject *parent) :
    QTreeWidget((QWidget*)parent),
    doNotPopulate(false)
{
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setSortingEnabled(true);
    this->sortByColumn(EC_TREE_COLUMN_NAME,Qt::AscendingOrder);

    this->setColumnCount(EC_TREE_N_COLUMNS);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(EC_TREE_COLUMN_NAME,QString("Name"));
    this->setHeaderItem(headerItem);
    this->setHeaderHidden(true);
    this->setRootIsDecorated(false);

    QObject::connect(Application::instance()->getSession(),&Session::problemSelectionChanged,this,&ECManagerTree::onProblemChanged);
    QObject::connect(Application::instance()->getSession(),&Session::modelSelectionChanged,this,&ECManagerTree::onModelSelectionChanged);
    QObject::connect(this,&ECManagerTree::itemChanged,this,&ECManagerTree::onItemChanged);
    QObject::connect(this,&ECManagerTree::itemSelectionChanged,this,&ECManagerTree::onItemSelectionChanged);
    QObject::connect(this,&ECManagerTree::ecSelected,this,&ECManagerTree::onEcSelected);
}

void ECManagerTree::populate()
{
    if (this->doNotPopulate)
    {
        return;
    }

    RProblemTypeMask ecPrbTypeMask;
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
        this->selectedTypes.append(REnvironmentConditionType(selectedItemsList[i]->data(EC_TREE_COLUMN_NAME,Qt::UserRole).toInt()));
    }

    this->blockSignals(true);

    this->clear();

    // QMap<REnvironmentConditionType,uint> ecMap = this->getEcMap(selectedEntityIDs);

    for (REnvironmentConditionType ecType = (REnvironmentConditionType)((int)R_ENVIRONMENT_CONDITION_NONE + 1);
                                ecType < R_ENVIRONMENT_CONDITION_N_TYPES;
                                ecType ++)
    {
        ecPrbTypeMask = REnvironmentCondition::getProblemTypeMask(ecType);
        if (!(ecPrbTypeMask & modelPrbTypeMask))
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

//        Qt::CheckState state = Qt::Unchecked;
//        if (ecMap.contains(ecType))
//        {
//            if (ecMap.value(ecType,1) == (uint)selectedEntityIDs.size())
//            {
//                state = Qt::Checked;
//            }
//            else
//            {
//                state = Qt::PartiallyChecked;
//            }
//        }
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setText(EC_TREE_COLUMN_NAME, REnvironmentCondition::getName(ecType));
//        item->setData(EC_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(state));
        item->setData(EC_TREE_COLUMN_NAME,Qt::UserRole,QVariant(ecType));
    }

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        this->topLevelItem(i)->setSelected(false);
        for (int j=0;j<this->selectedTypes.size();j++)
        {
            if (this->selectedTypes[j] == REnvironmentConditionType(this->topLevelItem(i)->data(EC_TREE_COLUMN_NAME,Qt::UserRole).toInt()))
            {
                this->topLevelItem(i)->setSelected(true);
                break;
            }
        }
    }

    this->blockSignals(false);

    emit this->itemSelectionChanged();
}
void ECManagerTree::onModelSelectionChanged(uint)
{
    this->populate();
}

void ECManagerTree::onProblemChanged(uint)
{
    this->populate();
}

QMap<REnvironmentConditionType,uint> ECManagerTree::getEcMap(const QList<SessionEntityID> &entityIDs) const
{
    QMap<REnvironmentConditionType,uint> ecMap;
    REnvironmentConditionType ecType;
    uint ecCount;

    for (int i=0;i<entityIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(entityIDs[i].getMid());

        if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT)
        {
            for (uint j=0;j<pModel->getPoint(entityIDs[i].getEid()).getNEnvironmentConditions();j++)
            {
                ecType = pModel->getPoint(entityIDs[i].getEid()).getEnvironmentCondition(j).getType();
                ecCount = ecMap.value(ecType,0) + 1;
                ecMap.insert(ecType,ecCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE)
        {
            for (uint j=0;j<pModel->getLine(entityIDs[i].getEid()).getNEnvironmentConditions();j++)
            {
                ecType = pModel->getLine(entityIDs[i].getEid()).getEnvironmentCondition(j).getType();
                ecCount = ecMap.value(ecType,0) + 1;
                ecMap.insert(ecType,ecCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE)
        {
            for (uint j=0;j<pModel->getSurface(entityIDs[i].getEid()).getNEnvironmentConditions();j++)
            {
                ecType = pModel->getSurface(entityIDs[i].getEid()).getEnvironmentCondition(j).getType();
                ecCount = ecMap.value(ecType,0) + 1;
                ecMap.insert(ecType,ecCount);
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_VOLUME)
        {
            for (uint j=0;j<pModel->getVolume(entityIDs[i].getEid()).getNEnvironmentConditions();j++)
            {
                ecType = pModel->getVolume(entityIDs[i].getEid()).getEnvironmentCondition(j).getType();
                ecCount = ecMap.value(ecType,0) + 1;
                ecMap.insert(ecType,ecCount);
            }
        }
    }

    return ecMap;
}

void ECManagerTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    Qt::CheckState checkState = item->checkState(column);
    REnvironmentConditionType ecType = (REnvironmentConditionType) item->data(EC_TREE_COLUMN_NAME,Qt::UserRole).toInt();
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(entityIDs[i].getMid());
        if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getPoint(entityIDs[i].getEid()).addEnvironmentCondition(REnvironmentCondition(ecType));
            }
            else
            {
                if (pModel->getPoint(entityIDs[i].getEid()).hasEnvironmentCondition(ecType))
                {
                    pModel->getPoint(entityIDs[i].getEid()).removeEnvironmentCondition(ecType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getLine(entityIDs[i].getEid()).addEnvironmentCondition(REnvironmentCondition(ecType));
            }
            else
            {
                if (pModel->getLine(entityIDs[i].getEid()).hasEnvironmentCondition(ecType))
                {
                    pModel->getLine(entityIDs[i].getEid()).removeEnvironmentCondition(ecType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getSurface(entityIDs[i].getEid()).addEnvironmentCondition(REnvironmentCondition(ecType));
            }
            else
            {
                if (pModel->getSurface(entityIDs[i].getEid()).hasEnvironmentCondition(ecType))
                {
                    pModel->getSurface(entityIDs[i].getEid()).removeEnvironmentCondition(ecType);
                }
            }
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_VOLUME)
        {
            if (checkState == Qt::Checked)
            {
                pModel->getVolume(entityIDs[i].getEid()).addEnvironmentCondition(REnvironmentCondition(ecType));
            }
            else
            {
                if (pModel->getVolume(entityIDs[i].getEid()).hasEnvironmentCondition(ecType))
                {
                    pModel->getVolume(entityIDs[i].getEid()).removeEnvironmentCondition(ecType);
                }
            }
        }
    }

    if (item->isSelected())
    {
        emit this->ecSelected(ecType,(checkState != Qt::Unchecked));
    }
    else
    {
        emit this->ecSelected(R_ENVIRONMENT_CONDITION_NONE,false);
    }
}

void ECManagerTree::onItemSelectionChanged()
{
    QList<QTreeWidgetItem *> items = this->selectedItems();
    emit this->ecSelected(R_ENVIRONMENT_CONDITION_NONE,false);
    for (int i=0;i<items.size();i++)
    {
        REnvironmentConditionType ecType = (REnvironmentConditionType)items[i]->data(EC_TREE_COLUMN_NAME,Qt::UserRole).toInt();
//        bool applied = (items[i]->checkState(EC_TREE_COLUMN_NAME) == Qt::Checked);
        emit this->ecSelected(ecType,true);
    }
}

void ECManagerTree::onEcSelected()
{
    this->doNotPopulate = true;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<modelIDs.size();i++)
    {
        Application::instance()->getSession()->setProblemChanged(modelIDs[i]);
    }
    this->doNotPopulate = false;
}
