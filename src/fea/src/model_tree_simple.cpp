#include "model_tree_simple.h"
#include "application.h"

typedef enum _ModelTreeSimpleEntity
{
    MODEL_TREE_NONE = 0,
    MODEL_TREE_MODEL,
    MODEL_TREE_GROUP_POINT,
    MODEL_TREE_GROUP_LINE,
    MODEL_TREE_GROUP_SURFACE,
    MODEL_TREE_GROUP_VOLUME,
    MODEL_TREE_GROUP_VECTOR_FIELD,
    MODEL_TREE_GROUP_SCALAR_FIELD,
    MODEL_TREE_GROUP_STREAM_LINE,
    MODEL_TREE_GROUP_CUT,
    MODEL_TREE_GROUP_ISO,
    MODEL_TREE_POINT,
    MODEL_TREE_LINE,
    MODEL_TREE_SURFACE,
    MODEL_TREE_VOLUME,
    MODEL_TREE_VECTOR_FIELD,
    MODEL_TREE_SCALAR_FIELD,
    MODEL_TREE_STREAM_LINE,
    MODEL_TREE_CUT,
    MODEL_TREE_ISO,
    MODEL_TREE_N_TYPES
} ModelTreeSimpleEntity;

typedef enum _ModelTreeSimpleColumn
{
    MODEL_TREE_COLUMN_NAME = 0,
    MODEL_TREE_COLUMN_TYPE,
    MODEL_TREE_COLUMN_MID,
    MODEL_TREE_COLUMN_EID,
    MODEL_TREE_N_COLUMNS
} ModelTreeSimpleColumn;

static REntityGroupType EntityTypeToGroupType(ModelTreeSimpleEntity entityType)
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
        case MODEL_TREE_VECTOR_FIELD:
            return R_ENTITY_GROUP_VECTOR_FIELD;
        case MODEL_TREE_SCALAR_FIELD:
            return R_ENTITY_GROUP_SCALAR_FIELD;
        case MODEL_TREE_STREAM_LINE:
            return R_ENTITY_GROUP_STREAM_LINE;
        case MODEL_TREE_CUT:
            return R_ENTITY_GROUP_CUT;
        case MODEL_TREE_ISO:
            return R_ENTITY_GROUP_ISO;
        default:
            return R_ENTITY_GROUP_NONE;
    }
}

ModelTreeSimple::ModelTreeSimple(uint modelID, REntityGroupTypeMask modelEntitiesMask, QWidget *parent) :
    QTreeWidget(parent)
{
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);

    this->setColumnCount(MODEL_TREE_N_COLUMNS);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(MODEL_TREE_COLUMN_NAME,QString("Model"));
    headerItem->setText(MODEL_TREE_COLUMN_TYPE,QString("type"));
    headerItem->setText(MODEL_TREE_COLUMN_MID,QString("modelID"));
    headerItem->setText(MODEL_TREE_COLUMN_EID,QString("entityID"));
    this->setHeaderItem(headerItem);

    this->setColumnHidden(MODEL_TREE_COLUMN_TYPE,true);
    this->setColumnHidden(MODEL_TREE_COLUMN_MID,true);
    this->setColumnHidden(MODEL_TREE_COLUMN_EID,true);

    this->addModel(modelID,modelEntitiesMask);

    QObject::connect(this,&ModelTreeSimple::itemSelectionChanged,this,&ModelTreeSimple::onSelectionChanged);
}

void ModelTreeSimple::selectEntity(uint modelID, REntityGroupType entityGroupType, uint entityID)
{
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *topItem = this->topLevelItem(i);
        for (int j=0;j<topItem->childCount();j++)
        {
            QTreeWidgetItem *groupItem = topItem->child(j);
            for (int k=0;k<groupItem->childCount();k++)
            {
                QTreeWidgetItem *entityItem = groupItem->child(k);
                uint mid = entityItem->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
                uint eid = entityItem->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt();
                REntityGroupType gid = EntityTypeToGroupType(ModelTreeSimpleEntity(entityItem->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toUInt()));
                if (modelID == mid && entityGroupType == gid && entityID == eid)
                {
                    entityItem->setSelected(true);
                }
            }
        }
    }
}

QList<SessionEntityID> ModelTreeSimple::getSelected() const
{
    QList<SessionEntityID> selectedIDs;

    QList<QTreeWidgetItem*> items = this->selectedItems();

    for (int i=0;i<items.size();i++)
    {
        ModelTreeSimpleEntity type = ModelTreeSimpleEntity(items[i]->data(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole).toInt());
        uint mid = items[i]->data(MODEL_TREE_COLUMN_MID,Qt::DisplayRole).toUInt();
        uint eid = items[i]->data(MODEL_TREE_COLUMN_EID,Qt::DisplayRole).toUInt();

        REntityGroupType grpType = EntityTypeToGroupType(type);

        if (grpType != R_ENTITY_GROUP_NONE)
        {
            selectedIDs.append(SessionEntityID(mid,grpType,eid));
        }
    }

    return selectedIDs;
}

void ModelTreeSimple::addModel(uint modelID, REntityGroupTypeMask modelEntitiesMask)
{
    const Model &rModel = Application::instance()->getSession()->getModel(modelID);

    this->blockSignals(true);
    QTreeWidgetItem *itemModel = new QTreeWidgetItem(this);
    itemModel->setText(MODEL_TREE_COLUMN_NAME, rModel.getName());
    itemModel->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_MODEL));
    itemModel->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

    if (rModel.getNPoints() > 0 && modelEntitiesMask & R_ENTITY_GROUP_POINT)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Points");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_POINT));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNPoints();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getPoint(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_POINT));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getPoint(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    if (rModel.getNLines() > 0 && modelEntitiesMask & R_ENTITY_GROUP_LINE)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Lines");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_LINE));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNLines();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getLine(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_LINE));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getLine(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    if (rModel.getNSurfaces() > 0 && modelEntitiesMask & R_ENTITY_GROUP_SURFACE)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Surfaces");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_SURFACE));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNSurfaces();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getSurface(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_SURFACE));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getSurface(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    if (rModel.getNVolumes() > 0 && modelEntitiesMask & R_ENTITY_GROUP_VOLUME)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Volumes");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_VOLUME));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNVolumes();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getVolume(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_VOLUME));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getVolume(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    if (rModel.getNCuts() > 0 && modelEntitiesMask & R_ENTITY_GROUP_CUT)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Cuts");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_CUT));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNCuts();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getCut(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_CUT));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getCut(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    if (rModel.getNIsos() > 0 && modelEntitiesMask & R_ENTITY_GROUP_ISO)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Isos");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_ISO));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNIsos();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getIso(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_ISO));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getIso(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    if (rModel.getNStreamLines() > 0 && modelEntitiesMask & R_ENTITY_GROUP_STREAM_LINE)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Stream lines");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_STREAM_LINE));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNStreamLines();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getStreamLine(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_STREAM_LINE));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getStreamLine(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    if (rModel.getNScalarFields() > 0 && modelEntitiesMask & R_ENTITY_GROUP_SCALAR_FIELD)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Scalar fields");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_SCALAR_FIELD));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNScalarFields();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getScalarField(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_SCALAR_FIELD));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getScalarField(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    if (rModel.getNVectorFields() > 0 && modelEntitiesMask & R_ENTITY_GROUP_VECTOR_FIELD)
    {
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(itemModel);
        itemGroup->setText(MODEL_TREE_COLUMN_NAME, "Vector fields");
        itemGroup->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_GROUP_VECTOR_FIELD));
        itemGroup->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));

        for (uint i=0;i<rModel.getNVectorFields();i++)
        {
            QTreeWidgetItem *itemEntity = new QTreeWidgetItem(itemGroup);
            itemEntity->setText(MODEL_TREE_COLUMN_NAME, rModel.getVectorField(i).getName());
            itemEntity->setData(MODEL_TREE_COLUMN_TYPE,Qt::DisplayRole,QVariant(MODEL_TREE_VECTOR_FIELD));
            itemEntity->setData(MODEL_TREE_COLUMN_MID,Qt::DisplayRole,QVariant(modelID));
            itemEntity->setData(MODEL_TREE_COLUMN_EID,Qt::DisplayRole,QVariant(i));
            if (rModel.getVectorField(i).getData().getSelected())
            {
                itemEntity->setSelected(true);
            }
        }
    }

    this->expandAll();

    this->blockSignals(false);

    this->onSelectionChanged();
}

void ModelTreeSimple::onSelectionChanged()
{
    this->blockSignals(true);

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *itemModel = this->topLevelItem(i);

        int nGroupsSelected = 0;
        for (int j=0;j<itemModel->childCount();j++)
        {
            QTreeWidgetItem *itemGroup = itemModel->child(j);
            if (itemModel->isSelected())
            {
                itemGroup->setSelected(true);
            }

            int nEntitiesSelected = 0;
            for (int k=0;k<itemGroup->childCount();k++)
            {
                QTreeWidgetItem *itemEntity = itemGroup->child(k);
                if (itemGroup->isSelected())
                {
                    itemEntity->setSelected(true);
                }
                if (itemEntity->isSelected())
                {
                    nEntitiesSelected++;
                }
            }
            if (nEntitiesSelected == itemGroup->childCount())
            {
                itemGroup->setSelected(true);
            }

            if (itemGroup->isSelected())
            {
                nGroupsSelected++;
            }
        }
        if (nGroupsSelected == itemModel->childCount())
        {
            itemModel->setSelected(true);
        }
    }

    this->blockSignals(false);
}
