#include <QHBoxLayout>

#include "pick_details_tree.h"
#include "application.h"

typedef enum _PickDetailsTreeColumn
{
    PICK_DETAILS_TREE_COLUMN_1 = 0,
    PICK_DETAILS_TREE_COLUMN_2,
    PICK_DETAILS_TREE_COLUMN_3,
    PICK_DETAILS_TREE_COLUMN_4,
    PICK_DETAILS_TREE_COLUMN_5,
    PICK_DETAILS_TREE_N_COLUMNS
} PickDetailsTreeColumn;

PickDetailsTree::PickDetailsTree(QWidget *parent) :
    QTreeWidget(parent)
{
    R_LOG_TRACE;
    this->setColumnCount(PICK_DETAILS_TREE_N_COLUMNS);
    this->setHeaderHidden(true);

    this->populate();

    QObject::connect(Application::instance()->getSession(),
                     &Session::pickListChanged,
                     this,
                     &PickDetailsTree::onPickListChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelAdded,
                     this,
                     &PickDetailsTree::onPickListChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelRemoved,
                     this,
                     &PickDetailsTree::onPickListChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelChanged,
                     this,
                     &PickDetailsTree::onPickListChanged);
    QObject::connect(this,
                     &PickDetailsTree::itemExpanded,
                     this,
                     &PickDetailsTree::onItemExpanded);
}

void PickDetailsTree::populate()
{
    R_LOG_TRACE;
    this->clear();

    const QVector<PickItem> &rPickItems = Application::instance()->getSession()->getPickList().getItems();

    for (int i=0;i<rPickItems.size();i++)
    {
        if (rPickItems[i].getItemType() != PICK_ITEM_ELEMENT &&
            rPickItems[i].getItemType() != PICK_ITEM_NODE &&
            rPickItems[i].getItemType() != PICK_ITEM_HOLE_ELEMENT)
        {
            continue;
        }

        QTreeWidgetItem *topItem = new QTreeWidgetItem(this);
        QString itemText;

        const Model &rModel = Application::instance()->getSession()->getModel(rPickItems[i].getEntityID().getMid());

        const SessionEntityID &rEntityID = rPickItems[i].getEntityID();
        uint elementID = rPickItems[i].getElementID();
        uint nodeID = rPickItems[i].getNodeID();

        if (rPickItems[i].getItemType() == PICK_ITEM_ELEMENT)
        {
            QTreeWidgetItem *childItem = nullptr;

            // Type
            childItem = new QTreeWidgetItem(topItem);
            childItem->setText(PICK_DETAILS_TREE_COLUMN_1,tr("Type") + ":");
            childItem->setText(PICK_DETAILS_TREE_COLUMN_2,REntityGroup::getTypeName(rEntityID.getType()));

            // Nodes
            if (REntityGroup::typeIsElementGroup(rEntityID.getType()))
            {
                itemText = tr("Element") + " # " + QLocale().toString(elementID);
                const RElement &rElement = rModel.getElement(elementID);
                for (uint j=0;j<rElement.size();j++)
                {
                    const RNode &rNode = rModel.getNode(rElement.getNodeId(j));

                    childItem = new QTreeWidgetItem(topItem);
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_1,tr("Node") + " " + QLocale().toString(j+1) + ":");
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_2,QLocale().toString(rNode.getX()));
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_3,QLocale().toString(rNode.getY()));
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_4,QLocale().toString(rNode.getZ()));
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_5,"[m]");
                }

                std::vector<RElement> edgeElements = rElement.generateEdgeElements();

                for (uint j=0;j<edgeElements.size();j++)
                {
                    double edgeLength = 0.0;

                    if (edgeElements[j].findLength(rModel.getNodes(),edgeLength))
                    {
                        childItem = new QTreeWidgetItem(topItem);
                        childItem->setText(PICK_DETAILS_TREE_COLUMN_1,tr("Edge") + " " + QLocale().toString(edgeElements[j].getNodeId(0)) + "-" + QLocale().toString(edgeElements[j].getNodeId(1)) + ":");
                        childItem->setText(PICK_DETAILS_TREE_COLUMN_2,QLocale().toString(edgeLength));
                        childItem->setText(PICK_DETAILS_TREE_COLUMN_5,"[m]");
                    }
                }
            }
            else
            {
                itemText = tr("Interpolated element") + " # " + QLocale().toString(rPickItems[i].getElementPosition());
                const RInterpolatedElement *pIElement = nullptr;
                switch (rEntityID.getType())
                {
                    case R_ENTITY_GROUP_CUT:
                        pIElement = &rModel.getCut(rEntityID.getEid()).at(rPickItems[i].getElementPosition());
                        break;
                    case R_ENTITY_GROUP_ISO:
                        pIElement = &rModel.getIso(rEntityID.getEid()).at(rPickItems[i].getElementPosition());
                        break;
                    default:
                        break;
                }
                if (!pIElement)
                {
                    continue;
                }
                for (uint j=0;j<pIElement->size();j++)
                {
                    const RInterpolatedNode &rINode = pIElement->at(j);

                    childItem = new QTreeWidgetItem(topItem);
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_1,tr("Node") + " " + QLocale().toString(j+1) + ":");
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_2,QLocale().toString(rINode.getX()));
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_3,QLocale().toString(rINode.getY()));
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_4,QLocale().toString(rINode.getZ()));
                    childItem->setText(PICK_DETAILS_TREE_COLUMN_5,"[m]");
                }
            }

            break;
        }
        else if (rPickItems[i].getItemType() == PICK_ITEM_NODE)
        {
            QTreeWidgetItem *childItem = nullptr;

            if (REntityGroup::typeIsElementGroup(rEntityID.getType()))
            {
                itemText = tr("Node") + " # " + QLocale().toString(nodeID);

                const RNode &rNode = rModel.getNode(nodeID);

                childItem = new QTreeWidgetItem(topItem);
                childItem->setText(PICK_DETAILS_TREE_COLUMN_1,tr("Coordinates") + ":");
                childItem->setText(PICK_DETAILS_TREE_COLUMN_2,QLocale().toString(rNode.getX()));
                childItem->setText(PICK_DETAILS_TREE_COLUMN_3,QLocale().toString(rNode.getY()));
                childItem->setText(PICK_DETAILS_TREE_COLUMN_4,QLocale().toString(rNode.getZ()));
                childItem->setText(PICK_DETAILS_TREE_COLUMN_5,"[m]");
            }
            else if (REntityGroup::typeIsInterpolatedElementGroup(rEntityID.getType()))
            {
                itemText = tr("Interpolated node") + " # " + QLocale().toString(rPickItems[i].getElementPosition()) + " / " + QLocale().toString(rPickItems[i].getNodePosition());
            }

            // Type
            childItem = new QTreeWidgetItem(topItem);
            childItem->setText(PICK_DETAILS_TREE_COLUMN_1,tr("Type") + ":");
            childItem->setText(PICK_DETAILS_TREE_COLUMN_5,REntityGroup::getTypeName(rEntityID.getType()));

            break;
        }
        else if (rPickItems[i].getItemType() == PICK_ITEM_HOLE_ELEMENT)
        {
            itemText = tr("Edge") + " # " + QLocale().toString(rPickItems[i].getElementPosition());
            break;
        }

        QMultiMap<RVariableType, PickValue> resultsValues = rModel.getPickedResultsValues(rPickItems[i]);
        if (resultsValues.size() > 0)
        {
            QTreeWidgetItem *childItem = new QTreeWidgetItem(topItem);
            childItem->setText(PICK_DETAILS_TREE_COLUMN_1,"Computed results");
            childItem->setFirstColumnSpanned(true);

            QMultiMap<RVariableType, PickValue>::iterator iter;

            for (iter = resultsValues.begin(); iter != resultsValues.end(); ++iter)
            {
                RVariableType variableType = RVariableType(iter.key());
                RRVector values(iter.value().getValues());

                QTreeWidgetItem *variableItem = new QTreeWidgetItem(childItem);
                variableItem->setText(PICK_DETAILS_TREE_COLUMN_1,RVariable::getName(variableType));

                if (values.size() == 1)
                {
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_2,QLocale().toString(values[0]));
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_5,"[" +RVariable::getUnits(variableType) + "]");
                }
                else if (values.size() == 2)
                {
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_2,QLocale().toString(values[0]));
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_3,QLocale().toString(values[1]));
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_5,"[" + RVariable::getUnits(variableType) + "]");
                }
                else if (values.size() == 3)
                {
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_2,QLocale().toString(values[0]));
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_3,QLocale().toString(values[1]));
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_4,QLocale().toString(values[2]));
                    variableItem->setText(PICK_DETAILS_TREE_COLUMN_5,"[" + RVariable::getUnits(variableType) + "]");
                }
            }
        }

        topItem->setText(PICK_DETAILS_TREE_COLUMN_1,itemText);
        topItem->setFirstColumnSpanned(true);
        topItem->setExpanded(true);
    }

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        this->topLevelItem(i)->setForeground(PICK_DETAILS_TREE_COLUMN_1,this->palette().brush(QPalette::Active,QPalette::Text));
        QFont font(this->topLevelItem(i)->font(PICK_DETAILS_TREE_COLUMN_1));
        font.setBold(true);
        this->topLevelItem(i)->setFont(PICK_DETAILS_TREE_COLUMN_1,font);

        for (int j=0;j<this->topLevelItem(i)->childCount();j++)
        {
            this->topLevelItem(i)->child(j)->setForeground(PICK_DETAILS_TREE_COLUMN_1,this->palette().brush(QPalette::Active,QPalette::Text));
            QFont font(this->topLevelItem(i)->child(j)->font(PICK_DETAILS_TREE_COLUMN_1));
            font.setBold(true);
            this->topLevelItem(i)->child(j)->setFont(PICK_DETAILS_TREE_COLUMN_1,font);
        }
    }

    this->resizeColumnsToContent();
}

void PickDetailsTree::onPickListChanged()
{
    R_LOG_TRACE;
    this->populate();
}

void PickDetailsTree::onItemExpanded(QTreeWidgetItem *)
{
    R_LOG_TRACE;
    this->resizeColumnsToContent();
}

void PickDetailsTree::resizeColumnsToContent()
{
    R_LOG_TRACE;
    this->resizeColumnToContents(PICK_DETAILS_TREE_COLUMN_5);
    this->resizeColumnToContents(PICK_DETAILS_TREE_COLUMN_4);
    this->resizeColumnToContents(PICK_DETAILS_TREE_COLUMN_3);
    this->resizeColumnToContents(PICK_DETAILS_TREE_COLUMN_2);
    this->resizeColumnToContents(PICK_DETAILS_TREE_COLUMN_1);
}
