#include "application.h"
#include "color_combo_box.h"
#include "model_entity_display_properties_tree.h"

typedef enum _DisplayPropertyType
{
    DISPLAY_PROPERTY_NONE = 0,
    DISPLAY_PROPERTY_DRAW_WIRE,
    DISPLAY_PROPERTY_DRAW_EDGES,
    DISPLAY_PROPERTY_DRAW_NODES,
    DISPLAY_PROPERTY_DRAW_ELEMENT_NUMS,
    DISPLAY_PROPERTY_DRAW_NODE_NUMS,
    DISPLAY_PROPERTY_DRAW_ARROW_HEADS,
    DISPLAY_PROPERTY_DRAW_EQUAL_ARROW_LENGTHS,
    DISPLAY_PROPERTY_DRAW_ARROW_FROM,
    DISPLAY_PROPERTY_COLOR_BY_PATCH,
    DISPLAY_PROPERTY_COLOR_BY_VIEW_FACTOR,
    DISPLAY_PROPERTY_N_TYPES
} DisplayPropertyType;

typedef enum _DisplayPropertiesTreeColumn
{
    DP_TREE_COLUMN_NAME = 0,
    DP_TREE_N_COLUMNS
} DisplayPropertiesTreeColumn;

ModelEntityDisplayPropertiesTree::ModelEntityDisplayPropertiesTree(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setColumnCount(DP_TREE_N_COLUMNS);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setRootIsDecorated(false);
    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(DP_TREE_COLUMN_NAME,tr("Display properties"));
    this->setHeaderItem(headerItem);

    QObject::connect(Application::instance()->getSession(),
                     &Session::modelSelectionChanged,
                     this,
                     &ModelEntityDisplayPropertiesTree::onModelSelectionChanged);

    QObject::connect(Application::instance()->getSession(),
                     &Session::modelChanged,
                     this,
                     &ModelEntityDisplayPropertiesTree::onModelChanged);

    QObject::connect(Application::instance()->getSession(),
                     &Session::problemSelectionChanged,
                     this,
                     &ModelEntityDisplayPropertiesTree::onProblemChanged);

    QObject::connect(this,
                     &ModelEntityDisplayPropertiesTree::itemChanged,
                     this,
                     &ModelEntityDisplayPropertiesTree::onItemChanged);

    this->populate();
}

void ModelEntityDisplayPropertiesTree::populate()
{
    this->clear();
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    if (entityIDs.size() == 0)
    {
        return;
    }
    this->blockSignals(true);

    int nDrawWire = 0;
    int nDrawEdges = 0;
    int nDrawNodes = 0;
    int nDrawElementNums = 0;
    int nDrawNodeNums = 0;
    int nDrawArrowHeads = 0;
    int nDrawEqualArrowLengths = 0;
    int nDrawArrowFrom = 0;
    int nColorByPatch = 0;
    int nColorByViewFactor = 0;
    bool canDrawWire = false;
    bool canDrawEdges = false;
    bool canDrawNodes = false;
    bool canDrawElementNums = false;
    bool canDrawNodeNums = false;
    bool canDrawArrowHeads = false;
    bool canDrawEqualArrowLengths = false;
    bool canDrawArrowFrom = false;
    bool canColorByPatch = false;
    bool canColorByViewFactor = false;
    QColor color;
    for (int i=0;i<entityIDs.size();i++)
    {
        const Model &rModel = Application::instance()->getSession()->getModel(entityIDs[i].getMid());
        if (rModel.getDrawWire(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nDrawWire++;
        }
        if (rModel.getDrawEdges(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nDrawEdges++;
        }
        if (rModel.getDrawNodes(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nDrawNodes++;
        }
        if (rModel.getDrawElementNumbers(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nDrawElementNums++;
        }
        if (rModel.getDrawNodeNumbers(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nDrawNodeNums++;
        }
        if (rModel.getDrawArrowHeads(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nDrawArrowHeads++;
        }
        if (rModel.getDrawEqualArrowLengths(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nDrawEqualArrowLengths++;
        }
        if (rModel.getDrawArrowFrom(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nDrawArrowFrom++;
        }
        if (rModel.getColorByPatch(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nColorByPatch++;
        }
        if (rModel.getColorByViewFactor(entityIDs[i].getType(),entityIDs[i].getEid()))
        {
            nColorByViewFactor++;
        }
        color = rModel.getColor(entityIDs[i].getType(),entityIDs[i].getEid());

        switch (entityIDs[i].getType())
        {
            case R_ENTITY_GROUP_VOLUME:
            case R_ENTITY_GROUP_SURFACE:
                canDrawNodes = canDrawElementNums = canDrawNodeNums = true;
                canDrawWire = canDrawEdges = true;
                break;
            case R_ENTITY_GROUP_LINE:
            case R_ENTITY_GROUP_POINT:
                canDrawNodes = canDrawElementNums = canDrawNodeNums = true;
                canDrawWire = true;
                break;
            case R_ENTITY_GROUP_VECTOR_FIELD:
                canDrawArrowHeads = true;
                canDrawEqualArrowLengths = true;
                canDrawArrowFrom = true;
                break;
            case R_ENTITY_GROUP_SCALAR_FIELD:
                break;
            case R_ENTITY_GROUP_CUT:
            case R_ENTITY_GROUP_ISO:
                canDrawNodes = canDrawWire = canDrawEdges = true;
                break;
            case R_ENTITY_GROUP_STREAM_LINE:
            default:
                break;
        }
    }

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        const Model &rModel = Application::instance()->getSession()->getModel(modelIDs[i]);

        if (!canColorByPatch)
        {
            canColorByPatch = rModel.canColorByPatch();
        }
        if (!canColorByViewFactor)
        {
            canColorByViewFactor = rModel.canColorByViewFactor();
        }
    }

    Qt::CheckState csDrawWire = Qt::Unchecked;
    Qt::CheckState csDrawEdges = Qt::Unchecked;
    Qt::CheckState csDrawNodes = Qt::Unchecked;
    Qt::CheckState csDrawElementNums = Qt::Unchecked;
    Qt::CheckState csDrawNodeNums = Qt::Unchecked;
    Qt::CheckState csDrawArrowHeads = Qt::Unchecked;
    Qt::CheckState csDrawEqualArrowLengths = Qt::Unchecked;
    Qt::CheckState csDrawArrowFrom = Qt::Unchecked;
    Qt::CheckState csColorByPatch = Qt::Unchecked;
    Qt::CheckState csColorByViewFactor = Qt::Unchecked;
    if (nDrawWire > 0)
    {
        csDrawWire = (nDrawWire == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nDrawEdges > 0)
    {
        csDrawEdges = (nDrawEdges == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nDrawNodes > 0)
    {
        csDrawNodes = (nDrawNodes == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nDrawElementNums > 0)
    {
        csDrawElementNums = (nDrawElementNums == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nDrawNodeNums > 0)
    {
        csDrawNodeNums = (nDrawNodeNums == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nDrawArrowHeads > 0)
    {
        csDrawArrowHeads = (nDrawArrowHeads == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nDrawEqualArrowLengths > 0)
    {
        csDrawEqualArrowLengths = (nDrawEqualArrowLengths == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nDrawArrowFrom > 0)
    {
        csDrawArrowFrom = (nDrawArrowFrom == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nColorByPatch > 0)
    {
        csColorByPatch = (nColorByPatch == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }
    if (nColorByViewFactor > 0)
    {
        csColorByViewFactor = (nColorByViewFactor == entityIDs.size()) ? Qt::Checked : Qt::PartiallyChecked;
    }

    QTreeWidgetItem *item;

    if (canDrawWire)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Draw as wireframe"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csDrawWire));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_DRAW_WIRE));
    }

    if (canDrawEdges)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Draw element edges"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csDrawEdges));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_DRAW_EDGES));
    }

    if (canDrawNodes)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Draw element nodes"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csDrawNodes));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_DRAW_NODES));
    }

    if (canDrawElementNums)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Draw element numbers"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csDrawElementNums));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_DRAW_ELEMENT_NUMS));
    }

    if (canDrawNodeNums)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Draw node numbers"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csDrawNodeNums));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_DRAW_NODE_NUMS));
    }

    if (canDrawArrowHeads)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Draw arrow heads"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csDrawArrowHeads));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_DRAW_ARROW_HEADS));
    }

    if (canDrawEqualArrowLengths)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Draw arrows with same length"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csDrawEqualArrowLengths));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_DRAW_EQUAL_ARROW_LENGTHS));
    }

    if (canDrawArrowFrom)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Draw arrows pointing from"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csDrawArrowFrom));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_DRAW_ARROW_FROM));
    }

    if (canColorByPatch)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Color elements by surface patch ID"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csColorByPatch));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_COLOR_BY_PATCH));
    }

    if (canColorByViewFactor)
    {
        item = new QTreeWidgetItem(this);
        item->setText(DP_TREE_COLUMN_NAME,tr("Color elements by view-factor"));
        item->setData(DP_TREE_COLUMN_NAME,Qt::CheckStateRole,QVariant(csColorByViewFactor));
        item->setData(DP_TREE_COLUMN_NAME,Qt::UserRole,QVariant(DISPLAY_PROPERTY_COLOR_BY_VIEW_FACTOR));
    }

    item = new QTreeWidgetItem(this);
    ColorComboBox *colorCombo = new ColorComboBox(this);
    colorCombo->setColor(color);
    this->connect(colorCombo,SIGNAL(colorChanged(QColor)),SLOT(onColorChanged(QColor)));

    this->setItemWidget(item,DP_TREE_COLUMN_NAME,colorCombo);
    this->blockSignals(false);
}

void ModelEntityDisplayPropertiesTree::onModelSelectionChanged(uint)
{
    this->populate();
}

void ModelEntityDisplayPropertiesTree::onModelChanged(uint)
{
    this->populate();
}

void ModelEntityDisplayPropertiesTree::onProblemChanged(uint)
{
    this->populate();
}

void ModelEntityDisplayPropertiesTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    if (column == DP_TREE_COLUMN_NAME)
    {
        DisplayPropertyType type = (DisplayPropertyType) item->data(DP_TREE_COLUMN_NAME,Qt::UserRole).toInt();

        for (int i=0;i<entityIDs.size();i++)
        {
            Model *pModel = Application::instance()->getSession()->getModelPtr(entityIDs[i].getMid());

            if (type == DISPLAY_PROPERTY_DRAW_WIRE)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setDrawWire(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setDrawWire(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_DRAW_EDGES)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setDrawEdges(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setDrawEdges(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_DRAW_NODES)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setDrawNodes(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setDrawNodes(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_DRAW_ELEMENT_NUMS)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setDrawElementNumbers(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setDrawElementNumbers(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_DRAW_NODE_NUMS)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setDrawNodeNumbers(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setDrawNodeNumbers(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_DRAW_ARROW_HEADS)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setDrawArrowHeads(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setDrawArrowHeads(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_DRAW_EQUAL_ARROW_LENGTHS)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setDrawEqualArrowLengths(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setDrawEqualArrowLengths(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_DRAW_ARROW_FROM)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setDrawArrowFrom(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setDrawArrowFrom(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_COLOR_BY_PATCH)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setColorByPatch(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setColorByPatch(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            else if (type == DISPLAY_PROPERTY_COLOR_BY_VIEW_FACTOR)
            {
                if (item->checkState(column) == Qt::Checked)
                {
                    pModel->setColorByViewFactor(entityIDs[i].getType(), entityIDs[i].getEid(), true);
                }
                else if (item->checkState(column) == Qt::Unchecked)
                {
                    pModel->setColorByViewFactor(entityIDs[i].getType(), entityIDs[i].getEid(), false);
                }
            }
            Application::instance()->getSession()->setDisplayPropertiesChanged(entityIDs[i].getMid(),entityIDs[i].getType(),entityIDs[i].getEid());
        }
    }
}

void ModelEntityDisplayPropertiesTree::onColorChanged(QColor color)
{
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    for (int i=0;i<entityIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(entityIDs[i].getMid());
        pModel->setColor(entityIDs[i].getType(), entityIDs[i].getEid(), color);

        Application::instance()->getSession()->setDisplayPropertiesChanged(entityIDs[i].getMid(),entityIDs[i].getType(),entityIDs[i].getEid());
    }
}
