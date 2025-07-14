#include <rbl_error.h>
#include <rbl_logger.h>

#include "model_tree.h"
#include "action.h"
#include "application.h"

typedef enum _ModelTreeEntity
{
    MODEL_TREE_NONE = 0,
    MODEL_TREE_MODEL,
    MODEL_TREE_GROUP_POINT,
    MODEL_TREE_GROUP_LINE,
    MODEL_TREE_GROUP_SURFACE,
    MODEL_TREE_GROUP_VOLUME,
    MODEL_TREE_GROUP_CUT,
    MODEL_TREE_GROUP_ISO,
    MODEL_TREE_GROUP_STREAM_LINE,
    MODEL_TREE_GROUP_SCALAR_FIELD,
    MODEL_TREE_GROUP_VECTOR_FIELD,
    MODEL_TREE_POINT,
    MODEL_TREE_LINE,
    MODEL_TREE_SURFACE,
    MODEL_TREE_VOLUME,
    MODEL_TREE_CUT,
    MODEL_TREE_ISO,
    MODEL_TREE_STREAM_LINE,
    MODEL_TREE_SCALAR_FIELD,
    MODEL_TREE_VECTOR_FIELD,
    MODEL_TREE_N_TYPES
} ModelTreeEntity;

typedef enum _ModelTreeColumn
{
    MODEL_TREE_COLUMN_NAME = 0,
    MODEL_TREE_COLUMN_NUMBER,
    MODEL_TREE_COLUMN_TYPE,
    MODEL_TREE_COLUMN_MID,
    MODEL_TREE_COLUMN_EID,
    MODEL_TREE_N_COLUMNS
} ModelTreeColumn;

static REntityGroupType EntityTypeToGroupType(ModelTreeEntity entityType)
{
    switch (entityType)
    {
        case MODEL_TREE_POINT:
            return R_ENTITY_GROUP_POINT;
        case MODEL_TREE_LINE:
            return R_ENTITY_GROUP_LINE;
        case MODEL_TREE_SURFACE:
            return R_ENTITY_GROUP_SURFACE;
        case MODEL_TREE_VOLUME:
            return R_ENTITY_GROUP_VOLUME;
        case MODEL_TREE_CUT:
            return R_ENTITY_GROUP_CUT;
        case MODEL_TREE_ISO:
            return R_ENTITY_GROUP_ISO;
        case MODEL_TREE_STREAM_LINE:
            return R_ENTITY_GROUP_STREAM_LINE;
        case MODEL_TREE_SCALAR_FIELD:
            return R_ENTITY_GROUP_SCALAR_FIELD;
        case MODEL_TREE_VECTOR_FIELD:
            return R_ENTITY_GROUP_VECTOR_FIELD;
        default:
            return R_ENTITY_GROUP_NONE;
    }
}

ModelTree::ModelTree(ActionList *actionList, QWidget *parent)
    : QTreeWidget{parent}
    , actionList{actionList}
    , ignoreSignals{false}
{
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);

    this->setColumnCount(MODEL_TREE_N_COLUMNS);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setIcon(MODEL_TREE_COLUMN_NAME,QIcon(":/icons/mime/pixmaps/range-model.svg"));
    headerItem->setText(MODEL_TREE_COLUMN_NAME,tr("Name"));
    headerItem->setText(MODEL_TREE_COLUMN_NUMBER,QString("#"));
    headerItem->setText(MODEL_TREE_COLUMN_TYPE,QString("type"));
    headerItem->setText(MODEL_TREE_COLUMN_MID,QString("modelID"));
    headerItem->setText(MODEL_TREE_COLUMN_EID,QString("entityID"));
    this->setHeaderItem(headerItem);

    this->setColumnHidden(MODEL_TREE_COLUMN_TYPE,true);
    this->setColumnHidden(MODEL_TREE_COLUMN_MID,true);
    this->setColumnHidden(MODEL_TREE_COLUMN_EID,true);

    this->setContextMenuPolicy(Qt::ActionsContextMenu);
    this->addAction(this->actionList->getAction(Action::ACTION_MODEL_SAVE));
    this->addAction(this->actionList->getAction(Action::ACTION_MODEL_SAVE_AS));
    this->addAction(this->actionList->getAction(Action::ACTION_MODEL_CLOSE));
    this->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    this->addAction(this->actionList->getAction(Action::ACTION_MODEL_RENAME));
    this->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    this->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ENTITY_MERGE));
    this->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ENTITY_REMOVE));
    this->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    this->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_CUT_EDIT));
    this->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ISO_EDIT));
    this->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_STREAM_LINE_EDIT));
    this->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SCALAR_FIELD_EDIT));
    this->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_VECTOR_FIELD_EDIT));

    QObject::connect(Application::instance()->getSession(),
                     &Session::entitySelectionChanged,
                     this,
                     &ModelTree::onEntitySelectionChanged);

    QObject::connect(Application::instance()->getSession(),
                     &Session::entityVisibilityChanged,
                     this,
                     &ModelTree::onEntityVisibilityChanged);

    QObject::connect(this->model(),
                     &QAbstractItemModel::dataChanged,
                     this,
                     &ModelTree::onDataChanged);

    QObject::connect(this,
                     &ModelTree::itemSelectionChanged,
                     this,
                     &ModelTree::onSelectionChanged);

    QObject::connect(this,
                     &ModelTree::itemExpanded,
                     this,
                     &ModelTree::onItemExpanded);

    QObject::connect(this,
                     &ModelTree::itemCollapsed,
                     this,
                     &ModelTree::onItemExpanded);

    QObject::connect(this,
                     &ModelTree::itemDoubleClicked,
                     this,
                     &ModelTree::onItemDoubleClicked);

    QObject::connect(this,
                     &ModelTree::itemChanged,
                     this,
                     &ModelTree::onItemChanged);

    this->populate();
}

QList<uint> ModelTree::getSelectedModelIDs() const
{
    QList<uint> modelIDs;
    QList<QTreeWidgetItem *> items;

    items = this->selectedItems();
    for (int i = 0;i<items.size();i++)
    {
        QVariant id = items[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole);
        // Check whether the list already contains such ID
        bool foundID = false;
        for (int j=0;j<modelIDs.size();j++)
        {
            if (modelIDs[j] == id.toUInt())
            {
                foundID = true;
                break;
            }
        }
        if (!foundID)
        {
            modelIDs.push_back(id.toUInt());
        }
    }

    return modelIDs;
}

void ModelTree::setIgnoreSignals(bool ignoreSignals)
{
    this->ignoreSignals = ignoreSignals;
}

void ModelTree::populate()
{
    this->setIgnoreSignals(true);

    this->blockSignals(true);
    for (uint i=0;i<Application::instance()->getSession()->getNModels();i++)
    {
        this->insertModel(i);
    }
    this->blockSignals(false);

    this->setIgnoreSignals(false);
}

QList<QTreeWidgetItem *> ModelTree::getAllItems(QTreeWidgetItem *parent) const
{
    QList<QTreeWidgetItem *> itemList;

    int count = parent ? parent->childCount() : this->topLevelItemCount();

    for (int i=0;i<count;i++)
    {
        QTreeWidgetItem *item = parent ? parent->child(i) : this->topLevelItem(i);

        itemList.append(item);
        itemList.append(this->getAllItems(item));
    }

    return itemList;
}

void ModelTree::insertModel(uint modelID)
{
    const Model &rModel = Application::instance()->getSession()->getModel(modelID);

    QTreeWidgetItem *itemModel = this->findModelItem(modelID,true);
    itemModel->setText(MODEL_TREE_COLUMN_NAME, rModel.getName());
    itemModel->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_MODEL));
    itemModel->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

    bool modelChecked = false;
    bool modelUnChecked = false;

    for (int i=itemModel->childCount()-1;i>=0;i--)
    {
        if (
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_POINT && rModel.getNPoints() == 0)
            ||
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_LINE && rModel.getNLines() == 0)
            ||
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_SURFACE && rModel.getNSurfaces() == 0)
            ||
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_VOLUME && rModel.getNVolumes() == 0)
            ||
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_CUT && rModel.getNCuts() == 0)
            ||
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_ISO && rModel.getNIsos() == 0)
            ||
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_STREAM_LINE && rModel.getNStreamLines() == 0)
            ||
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_VECTOR_FIELD && rModel.getNVectorFields() == 0)
            ||
            (itemModel->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == MODEL_TREE_GROUP_SCALAR_FIELD && rModel.getNScalarFields() == 0)
            )
        {
            itemModel->takeChild(i);
        }
    }

    if (rModel.getNPoints() > 0)
    {
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_POINT,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("Points"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNPoints()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_POINT));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        bool itemChecked = false;
        bool itemUnChecked = false;
        for (uint i=0;i<rModel.getNPoints();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_POINT,i);

            if (visible) itemChecked = true;
            else itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getPoint(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getPoint(i).size()));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_POINT));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNPoints();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    if (rModel.getNLines() > 0)
    {
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_LINE,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("Lines"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNLines()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_LINE));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        bool itemChecked = false;
        bool itemUnChecked = false;
        for (uint i=0;i<rModel.getNLines();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_LINE,i);

            if (visible) itemChecked = true;
            else         itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getLine(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getLine(i).size()));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_LINE));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNLines();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    if (rModel.getNSurfaces() > 0)
    {
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_SURFACE,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("Surfaces"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNSurfaces()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_SURFACE));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        bool itemChecked = false;
        bool itemUnChecked = false;
        for (uint i=0;i<rModel.getNSurfaces();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_SURFACE,i);

            if (visible) itemChecked = true;
            else         itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getSurface(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getSurface(i).size()));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_SURFACE));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNSurfaces();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    if (rModel.getNVolumes() > 0)
    {
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_VOLUME,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("Volumes"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNVolumes()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_VOLUME));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        bool itemChecked = false;
        bool itemUnChecked = false;
        for (uint i=0;i<rModel.getNVolumes();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_VOLUME,i);

            if (visible) itemChecked = true;
            else         itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getVolume(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getVolume(i).size()));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_VOLUME));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNVolumes();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    if (rModel.getNCuts() > 0)
    {
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_CUT,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("Cuts"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNCuts()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_CUT));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        bool itemChecked = false;
        bool itemUnChecked = false;
        for (uint i=0;i<rModel.getNCuts();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_CUT,i);

            if (visible) itemChecked = true;
            else         itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getCut(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_CUT));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNCuts();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    if (rModel.getNIsos() > 0)
    {
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_ISO,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("ISOs"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNIsos()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_ISO));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        bool itemChecked = false;
        bool itemUnChecked = false;
        for (uint i=0;i<rModel.getNIsos();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_ISO,i);

            if (visible) itemChecked = true;
            else         itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getIso(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_ISO));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNIsos();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    if (rModel.getNStreamLines() > 0)
    {
        bool itemChecked = false;
        bool itemUnChecked = false;
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_STREAM_LINE,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("Stream lines"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNStreamLines()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_STREAM_LINE));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNStreamLines();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_STREAM_LINE,i);

            if (visible) itemChecked = true;
            else         itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getStreamLine(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_STREAM_LINE));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNStreamLines();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    if (rModel.getNScalarFields() > 0)
    {
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_SCALAR_FIELD,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("Scalar fields"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNScalarFields()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_SCALAR_FIELD));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        bool itemChecked = false;
        bool itemUnChecked = false;
        for (uint i=0;i<rModel.getNScalarFields();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_SCALAR_FIELD,i);

            if (visible) itemChecked = true;
            else         itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getScalarField(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_SCALAR_FIELD));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNScalarFields();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    if (rModel.getNVectorFields() > 0)
    {
        QTreeWidgetItem *itemGroup = this->findEntityGroupItem(itemModel,MODEL_TREE_GROUP_VECTOR_FIELD,true);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, tr("Vector fields"));
        itemGroup->setData(MODEL_TREE_COLUMN_NUMBER,Qt::DisplayRole,QVariant(rModel.getNVectorFields()));
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_VECTOR_FIELD));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        bool itemChecked = false;
        bool itemUnChecked = false;
        for (uint i=0;i<rModel.getNVectorFields();i++)
        {
            bool visible = rModel.getVisible(R_ENTITY_GROUP_VECTOR_FIELD,i);

            if (visible) itemChecked = true;
            else         itemUnChecked = true;

            QTreeWidgetItem *itemEntity = this->findEntityItem(itemGroup,i,true);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getVectorField(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(visible?Qt::Checked:Qt::Unchecked));
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_VECTOR_FIELD));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
        }
        for (int i=itemGroup->childCount()-1;i>=(int)rModel.getNVectorFields();i--)
        {
            itemGroup->takeChild(i);
        }

        Qt::CheckState itemGroupState = Qt::Unchecked;
        if (itemChecked)
        {
            itemGroupState = Qt::Checked;
            if (itemUnChecked)
            {
                itemGroupState = Qt::PartiallyChecked;
            }
        }
        itemGroup->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(itemGroupState));

        if (itemChecked) modelChecked = true;
        if (itemUnChecked) modelUnChecked = true;
    }

    Qt::CheckState modelState = Qt::Unchecked;
    if (modelChecked)
    {
        modelState = Qt::Checked;
        if (modelUnChecked)
        {
            modelState = Qt::PartiallyChecked;
        }
    }
    itemModel->setData(MODEL_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(modelState));
}

void ModelTree::removeModel(uint modelID)
{    QList<QTreeWidgetItem *> allItems = this->getAllItems();
     for (int i=0;i<allItems.size();i++)
     {
         uint mid = allItems[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
         if (mid > modelID)
         {
             allItems[i]->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(mid-1));
         }
     }
      delete this->takeTopLevelItem(modelID);
}

QTreeWidgetItem *ModelTree::findModelItem(uint modelID, bool create)
{
    QTreeWidgetItem *item = nullptr;
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        if (this->topLevelItem(i)->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt() == modelID)
        {
            item = this->topLevelItem(i);
        }
    }
    if (!item && create)
    {
        item = new QTreeWidgetItem(this);
    }
    return item;
}

QTreeWidgetItem *ModelTree::findEntityGroupItem(QTreeWidgetItem *parentItem, int entityGroupType, bool create)
{
    QTreeWidgetItem *item = nullptr;
    for (int i=0;i<parentItem->childCount();i++)
    {
        if (parentItem->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() == entityGroupType)
        {
            item = parentItem->child(i);
        }
    }
    if (!item && create)
    {
        item = new QTreeWidgetItem;
        int index = 0;
        for (int i=0;i<parentItem->childCount();i++)
        {
            if (parentItem->child(i)->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt() > entityGroupType)
            {
                break;
            }
            index = i+1;
        }
        parentItem->insertChild(index,item);
    }

    return item;
}

QTreeWidgetItem *ModelTree::findEntityItem(QTreeWidgetItem *parentItem, uint entityID, bool create)
{
    QTreeWidgetItem *item = nullptr;
    for (int i=0;i<parentItem->childCount();i++)
    {
        if (parentItem->child(i)->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt() == entityID)
        {
            item = parentItem->child(i);
        }
    }
    if (!item && create)
    {
        item = new QTreeWidgetItem(parentItem);
    }
    return item;
}

void ModelTree::onModelAdded(uint modelID)
{
    this->blockSignals(true);
    this->insertModel(modelID);
    this->blockSignals(false);
    this->selectSelected();
    this->expandSelected();
    Application::instance()->getSession()->setModelSelectionChanged(modelID);
    this->actionList->processAvailability();

    for (int i=0;i<MODEL_TREE_N_COLUMNS;i++)
    {
        this->resizeColumnToContents(i);
    }
}

void ModelTree::onModelRemoved(uint modelID)
{
    this->blockSignals(true);
    this->removeModel(modelID);
    this->blockSignals(false);
    emit this->itemSelectionChanged();
    Application::instance()->getSession()->setModelSelectionChanged(modelID);
    this->actionList->processAvailability();
}

void ModelTree::onModelChanged(uint modelID)
{
    this->blockSignals(true);
    this->insertModel(modelID);
    this->blockSignals(false);
    Application::instance()->getSession()->setModelSelectionChanged(modelID);
    this->actionList->processAvailability();
    this->onSelectionChanged();
}

void ModelTree::onSelectionChanged()
{
    QList<SessionEntityID> allEntityIDs = Application::instance()->getSession()->getAllEntityIDs();
    QMap<SessionEntityID,bool> entityIDsSelectedMap;
    QMap<uint,bool> emptyModelIDsSelectedMap;

    for (int i=0;i<allEntityIDs.size();i++)
    {
        entityIDsSelectedMap.insert(allEntityIDs[i],false);
    }
    for (uint i=0;i<Application::instance()->getSession()->getNModels();i++)
    {
        if (Application::instance()->getSession()->getModel(i).isEmpty())
        {
            emptyModelIDsSelectedMap.insert(i,false);
        }
    }

    QList<QTreeWidgetItem *> items = this->selectedItems();
    for (int i=0;i<items.size();i++)
    {
        ModelTreeEntity type = ModelTreeEntity(items[i]->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt());
        uint mid = items[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
        uint eid = items[i]->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt();

        if (Application::instance()->getSession()->getModel(mid).isEmpty())
        {
            emptyModelIDsSelectedMap.insert(mid,true);
        }
        SessionEntityID entityID;
        entityID.setMid(mid);
        if (type == MODEL_TREE_MODEL)
        {
            // Select all entities
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_POINT)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_POINT)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_LINE)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_LINE)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_SURFACE)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_SURFACE)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_VOLUME)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_VOLUME)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_CUT)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_CUT)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_ISO)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_ISO)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_STREAM_LINE)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_STREAM_LINE)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_SCALAR_FIELD)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_SCALAR_FIELD)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_GROUP_VECTOR_FIELD)
        {
            // Select all entities in the group
            for (int j=0;j<allEntityIDs.size();j++)
            {
                if (allEntityIDs[j].getMid() == mid &&
                    allEntityIDs[j].getType() == R_ENTITY_GROUP_VECTOR_FIELD)
                {
                    entityIDsSelectedMap[allEntityIDs[j]] = true;
                }
            }
        }
        else if (type == MODEL_TREE_POINT ||
                 type == MODEL_TREE_LINE ||
                 type == MODEL_TREE_SURFACE ||
                 type == MODEL_TREE_VOLUME ||
                 type == MODEL_TREE_CUT ||
                 type == MODEL_TREE_ISO ||
                 type == MODEL_TREE_STREAM_LINE ||
                 type == MODEL_TREE_SCALAR_FIELD ||
                 type == MODEL_TREE_VECTOR_FIELD)
        {
            entityID.setType(EntityTypeToGroupType(type));
            entityID.setEid(eid);
            entityIDsSelectedMap[entityID] = true;
        }
    }

    this->setIgnoreSignals(true);

    for (QMap<SessionEntityID,bool>::const_iterator iter = entityIDsSelectedMap.constBegin();
         iter != entityIDsSelectedMap.constEnd();
         ++iter) {
        SessionEntityID entityID = iter.key();
        bool selected = iter.value();
        Application::instance()->getSession()->setEntitySelected(entityID.getMid(),
                                                 entityID.getType(),
                                                 entityID.getEid(),
                                                 selected);
    }

    for (QMap<uint,bool>::const_iterator iter = emptyModelIDsSelectedMap.constBegin();
         iter != emptyModelIDsSelectedMap.constEnd();
         ++iter)
    {
        uint mid = iter.key();
        bool selected = iter.value();
        Application::instance()->getSession()->setModelSelected(mid,selected);
    }

    this->setIgnoreSignals(false);

    this->selectSelected();
    this->actionList->processAvailability();
}

void ModelTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    ModelTreeEntity entityType = (ModelTreeEntity)item->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt();
    REntityGroupType elementGrpType = EntityTypeToGroupType(entityType);
    uint mid = item->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
    uint eid = item->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt();
    QString newName = item->text(MODEL_TREE_COLUMN_NAME);
    if (column == MODEL_TREE_COLUMN_NAME)
    {
        this->setIgnoreSignals(true);
        bool visible = item->checkState(column);
        if (entityType == MODEL_TREE_MODEL)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNPoints();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_POINT,i,visible);
            }
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNLines();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_LINE,i,visible);
            }
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNSurfaces();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_SURFACE,i,visible);
            }
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNVolumes();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_VOLUME,i,visible);
            }
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNCuts();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_CUT,i,visible);
            }
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNIsos();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_ISO,i,visible);
            }
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNStreamLines();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_STREAM_LINE,i,visible);
            }
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNVectorFields();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_VECTOR_FIELD,i,visible);
            }
            Application::instance()->getSession()->getModel(mid).setName(newName);
        }
        else if (entityType == MODEL_TREE_GROUP_POINT)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNPoints();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_POINT,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_GROUP_LINE)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNLines();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_LINE,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_GROUP_SURFACE)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNSurfaces();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_SURFACE,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_GROUP_VOLUME)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNVolumes();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_VOLUME,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_GROUP_CUT)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNCuts();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_CUT,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_GROUP_ISO)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNIsos();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_ISO,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_GROUP_STREAM_LINE)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNStreamLines();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_STREAM_LINE,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_GROUP_SCALAR_FIELD)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNScalarFields();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_SCALAR_FIELD,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_GROUP_VECTOR_FIELD)
        {
            for (uint i=0;i<Application::instance()->getSession()->getModel(mid).getNVectorFields();i++)
            {
                Application::instance()->getSession()->setEntityVisible(mid,R_ENTITY_GROUP_VECTOR_FIELD,i,visible);
            }
        }
        else if (entityType == MODEL_TREE_POINT ||
                 entityType == MODEL_TREE_LINE ||
                 entityType == MODEL_TREE_SURFACE ||
                 entityType == MODEL_TREE_VOLUME ||
                 entityType == MODEL_TREE_CUT ||
                 entityType == MODEL_TREE_ISO ||
                 entityType == MODEL_TREE_STREAM_LINE ||
                 entityType == MODEL_TREE_SCALAR_FIELD ||
                 entityType == MODEL_TREE_VECTOR_FIELD)
        {
            Application::instance()->getSession()->setEntityVisible(mid,elementGrpType,eid,visible);
            if (entityType == MODEL_TREE_POINT)
            {
                Application::instance()->getSession()->getModel(mid).getPoint(eid).setName(newName);
            }
            else if (entityType == MODEL_TREE_LINE)
            {
                Application::instance()->getSession()->getModel(mid).getLine(eid).setName(newName);
            }
            else if (entityType == MODEL_TREE_SURFACE)
            {
                Application::instance()->getSession()->getModel(mid).getSurface(eid).setName(newName);
            }
            else if (entityType == MODEL_TREE_VOLUME)
            {
                Application::instance()->getSession()->getModel(mid).getVolume(eid).setName(newName);
            }
            else if (entityType == MODEL_TREE_CUT)
            {
                Application::instance()->getSession()->getModel(mid).getCut(eid).setName(newName);
            }
            else if (entityType == MODEL_TREE_ISO)
            {
                Application::instance()->getSession()->getModel(mid).getIso(eid).setName(newName);
            }
            else if (entityType == MODEL_TREE_STREAM_LINE)
            {
                Application::instance()->getSession()->getModel(mid).getStreamLine(eid).setName(newName);
            }
            else if (entityType == MODEL_TREE_SCALAR_FIELD)
            {
                Application::instance()->getSession()->getModel(mid).getScalarField(eid).setName(newName);
            }
            else if (entityType == MODEL_TREE_VECTOR_FIELD)
            {
                Application::instance()->getSession()->getModel(mid).getVectorField(eid).setName(newName);
            }
        }
        this->setIgnoreSignals(false);
        Application::instance()->getSession()->setModelRenamed(mid);
    }

    this->checkVisible();
}

void ModelTree::onEntitySelectionChanged(uint             mid,
                                         REntityGroupType elementGrpType,
                                         uint             eid,
                                         bool             selected)
{
    if (this->ignoreSignals)
    {
        return;
    }
    QList<QTreeWidgetItem *> allItems = this->getAllItems();
    for (int i=0;i<allItems.size();i++)
    {
        ModelTreeEntity entityType = (ModelTreeEntity)allItems[i]->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt();
        REntityGroupType iElementGrpType = EntityTypeToGroupType(entityType);
        uint iMid = allItems[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
        uint iEid = allItems[i]->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt();
        if (iMid == mid && iElementGrpType == elementGrpType && iEid == eid)
        {
            allItems[i]->setSelected(selected);
        }
    }
}

void ModelTree::onEntityVisibilityChanged(uint mid, REntityGroupType elementGrpType, uint eid, bool visible)
{
    if (this->ignoreSignals)
    {
        return;
    }
    QList<QTreeWidgetItem *> allItems = this->getAllItems();
    for (int i=0;i<allItems.size();i++)
    {
        ModelTreeEntity entityType = (ModelTreeEntity)allItems[i]->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt();
        REntityGroupType iElementGrpType = EntityTypeToGroupType(entityType);
        uint iMid = allItems[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
        uint iEid = allItems[i]->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt();
        if (iMid == mid && iElementGrpType == elementGrpType && iEid == eid)
        {
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,visible?Qt::Checked:Qt::Unchecked);
        }
    }
}

void ModelTree::selectSelected()
{
    QList<QTreeWidgetItem *> allItems = this->getAllItems();
    this->blockSignals(true);
    for (int i=0;i<allItems.size();i++)
    {
        ModelTreeEntity entityType = (ModelTreeEntity)allItems[i]->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt();
        REntityGroupType elementGrpType = EntityTypeToGroupType(entityType);
        uint mid = allItems[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
        uint eid = allItems[i]->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt();

        if (entityType == MODEL_TREE_MODEL)
        {
            bool modelSelected = Application::instance()->getSession()->getModel(mid).isSelected(true);
            allItems[i]->setSelected(modelSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_POINT)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_POINT,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_LINE)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_LINE,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_SURFACE)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_SURFACE,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_VOLUME)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_VOLUME,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_CUT)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_CUT,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_ISO)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_ISO,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_STREAM_LINE)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_STREAM_LINE,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_SCALAR_FIELD)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_SCALAR_FIELD,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_GROUP_VECTOR_FIELD)
        {
            bool groupSelected = Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_VECTOR_FIELD,true);
            allItems[i]->setSelected(groupSelected);
        }
        else if (entityType == MODEL_TREE_POINT ||
                 entityType == MODEL_TREE_LINE ||
                 entityType == MODEL_TREE_SURFACE ||
                 entityType == MODEL_TREE_VOLUME ||
                 entityType == MODEL_TREE_CUT ||
                 entityType == MODEL_TREE_ISO ||
                 entityType == MODEL_TREE_STREAM_LINE ||
                 entityType == MODEL_TREE_SCALAR_FIELD ||
                 entityType == MODEL_TREE_VECTOR_FIELD)
        {
            bool entitySelected = Application::instance()->getSession()->getModel(mid).getSelected(elementGrpType,eid);
            allItems[i]->setSelected(entitySelected);
        }
        else
        {
            RError error(RError::Type::Application,R_ERROR_REF,"Unknown entity type \'%d\'",entityType);
            RLogger::warning("%s\n",error.getMessage().toUtf8().constData());
        }
    }
    this->blockSignals(false);
}

void ModelTree::expandSelected()
{

    QList<QTreeWidgetItem *> allItems = this->getAllItems();
    this->blockSignals(true);
    for (int i=0;i<allItems.size();i++)
    {
        ModelTreeEntity entityType = ModelTreeEntity(allItems[i]->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt());
        uint mid = allItems[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();

        bool expand = false;
        switch (entityType) {
            case MODEL_TREE_MODEL:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(false));
                break;
            case MODEL_TREE_GROUP_POINT:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_POINT,false));
                break;
            case MODEL_TREE_GROUP_LINE:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_LINE,false));
                break;
            case MODEL_TREE_GROUP_SURFACE:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_SURFACE,false));
                break;
            case MODEL_TREE_GROUP_VOLUME:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_VOLUME,false));
                break;
            case MODEL_TREE_GROUP_CUT:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_CUT,false));
                break;
            case MODEL_TREE_GROUP_ISO:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_ISO,false));
                break;
            case MODEL_TREE_GROUP_STREAM_LINE:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_STREAM_LINE,false));
                break;
            case MODEL_TREE_GROUP_SCALAR_FIELD:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_SCALAR_FIELD,false));
                break;
            case MODEL_TREE_GROUP_VECTOR_FIELD:
                expand = (!Application::instance()->getSession()->getModel(mid).isSelected(R_ENTITY_GROUP_VECTOR_FIELD,false));
                break;
            default:
                break;
        }
        if (expand)
        {
            allItems[i]->setExpanded(true);
        }
    }
    this->blockSignals(false);
}

void ModelTree::checkVisible()
{
    QList<QTreeWidgetItem *> allItems = this->getAllItems();
    bool visible;
    bool hidden;
    Qt::CheckState checkState;
    this->blockSignals(true);
    for (int i=0;i<allItems.size();i++)
    {
        ModelTreeEntity entityType = (ModelTreeEntity)allItems[i]->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt();
        REntityGroupType elementGrpType = EntityTypeToGroupType(entityType);
        uint mid = allItems[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
        uint eid = allItems[i]->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt();

        checkState = Qt::PartiallyChecked;
        if (entityType == MODEL_TREE_MODEL)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_POINT)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_POINT,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_POINT,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_LINE)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_LINE,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_LINE,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_SURFACE)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_SURFACE,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_SURFACE,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_VOLUME)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_VOLUME,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_VOLUME,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_CUT)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_CUT,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_CUT,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_ISO)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_ISO,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_ISO,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_STREAM_LINE)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_STREAM_LINE,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_STREAM_LINE,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_SCALAR_FIELD)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_SCALAR_FIELD,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_SCALAR_FIELD,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_GROUP_VECTOR_FIELD)
        {
            visible = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_VECTOR_FIELD,true);
            hidden = Application::instance()->getSession()->getModel(mid).isVisible(R_ENTITY_GROUP_VECTOR_FIELD,false);
            if (visible)
            {
                checkState = Qt::Checked;
            }
            if (hidden)
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
        else if (entityType == MODEL_TREE_POINT ||
                 entityType == MODEL_TREE_LINE ||
                 entityType == MODEL_TREE_SURFACE ||
                 entityType == MODEL_TREE_VOLUME ||
                 entityType == MODEL_TREE_CUT ||
                 entityType == MODEL_TREE_ISO ||
                 entityType == MODEL_TREE_STREAM_LINE ||
                 entityType == MODEL_TREE_SCALAR_FIELD ||
                 entityType == MODEL_TREE_VECTOR_FIELD)
        {
            if (Application::instance()->getSession()->getModel(mid).getVisible(elementGrpType,eid))
            {
                checkState = Qt::Checked;
            }
            else
            {
                checkState = Qt::Unchecked;
            }
            allItems[i]->setCheckState(MODEL_TREE_COLUMN_NAME,checkState);
        }
    }
    this->blockSignals(false);
}

void ModelTree::onDataChanged(QModelIndex topleft, QModelIndex bottomRight)
{
    for (int i=topleft.column();i<bottomRight.column();i++)
    {
        this->resizeColumnToContents(i);
    }
}

void ModelTree::onItemExpanded(QTreeWidgetItem *item)
{
    for (int i=0;i<item->columnCount();i++)
    {
        this->resizeColumnToContents(i);
    }
}

void ModelTree::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    bool isEditable = false;

    ModelTreeEntity entityType = ModelTreeEntity(item->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt());

    if (column == MODEL_TREE_COLUMN_NAME)
    {
        if (entityType == MODEL_TREE_POINT ||
            entityType == MODEL_TREE_LINE ||
            entityType == MODEL_TREE_SURFACE ||
            entityType == MODEL_TREE_VOLUME ||
            entityType == MODEL_TREE_CUT ||
            entityType == MODEL_TREE_ISO ||
            entityType == MODEL_TREE_STREAM_LINE ||
            entityType == MODEL_TREE_SCALAR_FIELD ||
            entityType == MODEL_TREE_VECTOR_FIELD)
        {
            isEditable = true;
        }
    }

    this->blockSignals(true);
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
    this->blockSignals(false);
}
