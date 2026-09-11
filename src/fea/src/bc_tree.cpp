#include <QStandardItemModel>
#include <QLineEdit>
#include <QPushButton>

#include "component_edit_dialog.h"
#include "application.h"
#include "bc_tree.h"
#include "direction_widget.h"
#include "variable_value_edit.h"
#include "push_button.h"

typedef enum _BCTreeColumn
{
    BC_TREE_PROPERTY_NAME = 0,
    BC_TREE_PROPERTY_VALUE,
    BC_TREE_PROPERTY_UNITS,
    BC_TREE_PROPERTY_TYPE,
    BC_TREE_N_COLUMNS
} BCTreeColumn;

BCTree::BCTree(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setRootIsDecorated(true);
    this->setSelectionMode(QAbstractItemView::NoSelection);

    this->setColumnCount(BC_TREE_N_COLUMNS);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(BC_TREE_PROPERTY_TYPE,tr("Property type"));
    headerItem->setText(BC_TREE_PROPERTY_NAME,tr("Property name"));
    headerItem->setText(BC_TREE_PROPERTY_VALUE,tr("Value"));
    headerItem->setText(BC_TREE_PROPERTY_UNITS,tr("Units"));
    this->setHeaderItem(headerItem);

    this->setColumnHidden(BC_TREE_PROPERTY_TYPE,true);

    QObject::connect(this,&QTreeWidget::itemChanged,this,&BCTree::onItemChanged);

    this->populate();
}

void BCTree::populate()
{
    // Check states are assigned below - do not take them for user edits.
    bool signalsWereBlocked = this->blockSignals(true);

    this->clear();

    for (uint i=0;i<this->bc.size();i++)
    {
        RConditionComponent component = this->bc.getComponent(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setData(BC_TREE_PROPERTY_TYPE,Qt::DisplayRole,QVariant(component.getType()));
        item->setText(BC_TREE_PROPERTY_NAME,component.getName());
        item->setText(BC_TREE_PROPERTY_UNITS,component.getUnits());
        if (this->bc.getOptional())
        {
            // Components of an optional condition can be switched off one by
            // one - a disabled component prescribes nothing.
            item->setCheckState(BC_TREE_PROPERTY_NAME,component.getEnabled() ? Qt::Checked : Qt::Unchecked);
            item->setToolTip(BC_TREE_PROPERTY_NAME,tr("Uncheck to leave this component unconstrained."));
        }
        VariableValueEdit *lineEdit = new VariableValueEdit(component.getType());
        lineEdit->setValue(component.getValue(0));
        this->setItemWidget(item,BC_TREE_PROPERTY_VALUE,lineEdit);
        if (component.size() > 1)
        {
            lineEdit->setDisabled(true);
        }
        if (this->bc.getOptional() && !component.getEnabled())
        {
            lineEdit->setDisabled(true);
        }

        QTreeWidgetItem *child = new QTreeWidgetItem(item);
        child->setFirstColumnSpanned(true);
        PushButton *buttonValues = new PushButton(uint(component.getType()),tr("Edit time dependent values"));
        this->setItemWidget(child,BC_TREE_PROPERTY_NAME,buttonValues);

        QObject::connect(lineEdit,&VariableValueEdit::valueChanged,this,&BCTree::onBcValueChanged);
        QObject::connect(buttonValues,&PushButton::clicked,this,&BCTree::onButtonValueClicked);
    }

    if (bc.getHasLocalDirection())
    {
        // A point entity has no geometry to derive a direction from, so the
        // entered one is always used. On a line or a surface the direction is
        // derived from the element direction or the element normals unless the
        // user asks for the entered one to be used instead.
        bool pointSelected = Session::selectedModelsHasEntitySelected(R_ENTITY_GROUP_POINT);

        if (!pointSelected)
        {
            QTreeWidgetItem *overrideItem = new QTreeWidgetItem(this);
            overrideItem->setText(BC_TREE_PROPERTY_NAME,tr("Use entered local direction"));
            overrideItem->setToolTip(BC_TREE_PROPERTY_NAME,
                                     tr("Uncheck to derive the local direction from the geometry of the entity."));
            overrideItem->setCheckState(BC_TREE_PROPERTY_NAME,
                                        bc.getExplicitLocalDirection() ? Qt::Checked : Qt::Unchecked);
            overrideItem->setData(BC_TREE_PROPERTY_TYPE,Qt::DisplayRole,QVariant(int(R_VARIABLE_NONE)));
        }

        if (pointSelected || bc.getExplicitLocalDirection())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(this);
            item->setFirstColumnSpanned(true);

            DirectionWidget *directionWidget = new DirectionWidget(tr("Local direction"),bc.getLocalDirection());
            directionWidget->hideButtons();
            this->setItemWidget(item,BC_TREE_PROPERTY_NAME,directionWidget);

            QObject::connect(directionWidget,&DirectionWidget::changed,this,&BCTree::onDirectionChanged);
        }
        this->drawLocalRotationBegin();
    }
    else
    {
        this->drawLocalRotationEnd();
    }

    this->resizeColumnToContents(BC_TREE_PROPERTY_NAME);
    this->resizeColumnToContents(BC_TREE_PROPERTY_VALUE);
    this->resizeColumnToContents(BC_TREE_PROPERTY_UNITS);

    this->blockSignals(signalsWereBlocked);
}

void BCTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (!item || column != BC_TREE_PROPERTY_NAME)
    {
        return;
    }

    bool checked = (item->checkState(BC_TREE_PROPERTY_NAME) == Qt::Checked);
    RVariableType variableType = RVariableType(item->data(BC_TREE_PROPERTY_TYPE,Qt::DisplayRole).toInt());

    if (variableType == R_VARIABLE_NONE)
    {
        // The local direction override.
        if (!this->bc.getHasLocalDirection() || this->bc.getExplicitLocalDirection() == checked)
        {
            return;
        }
        this->bc.setExplicitLocalDirection(checked);
    }
    else
    {
        // A component of an optional condition.
        if (!this->bc.getOptional())
        {
            return;
        }

        uint componentPosition = this->bc.findComponentPosition(variableType);
        if (componentPosition == RConstants::eod)
        {
            return;
        }
        if (this->bc.getComponent(componentPosition).getEnabled() == checked)
        {
            return;
        }
        this->bc.getComponent(componentPosition).setEnabled(checked);
    }

    this->updateSelectedEntities();
    this->populate();
}

void BCTree::updateSelectedEntities() const
{
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        switch (entityIDs[i].getType())
        {
            case R_ENTITY_GROUP_POINT:
                Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getPoint(entityIDs[i].getEid()).addBoundaryCondition(this->bc);
                break;
            case R_ENTITY_GROUP_LINE:
                Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getLine(entityIDs[i].getEid()).addBoundaryCondition(this->bc);
                break;
            case R_ENTITY_GROUP_SURFACE:
                Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getSurface(entityIDs[i].getEid()).addBoundaryCondition(this->bc);
                break;
            case R_ENTITY_GROUP_VOLUME:
                Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getVolume(entityIDs[i].getEid()).addBoundaryCondition(this->bc);
                break;
            default:
                break;
        }
        Application::instance()->getSession()->setBoundaryConditionChanged(entityIDs[i].getMid(),
                                                           entityIDs[i].getType(),
                                                           entityIDs[i].getEid());
    }

    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        Application::instance()->getSession()->setProblemChanged(modelID);
    }
}

void BCTree::drawLocalRotationBegin()
{
    Application::instance()->getSession()->setBeginDrawLocalDirections(this->findSelectedEntityLocalDirections());
}

void BCTree::drawLocalRotationEnd()
{
    Application::instance()->getSession()->setEndDrawLocalDirections();
}

QList<RLocalDirection> BCTree::findSelectedEntityLocalDirections() const
{
    QList<RLocalDirection> localDirections;

    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        QSet<uint> nodeIDs;

        const Model &rModel = Application::instance()->getSession()->getModel(modelID);
        foreach (const SessionEntityID &entityID, rModel.getSelectedEntityIDs(modelID))
        {
            switch (entityID.getType())
            {
                case R_ENTITY_GROUP_POINT:
                {
                    const RPoint &rPoint = rModel.getPoint(entityID.getEid());
                    for (uint i=0;i<rPoint.size();i++)
                    {
                        const RElement &rElement = rModel.getElement(rPoint.get(i));
                        for (uint j=0;j<rElement.size();j++)
                        {
                            nodeIDs.insert(rElement.getNodeId(j));
                        }
                    }
                    break;
                }
                case R_ENTITY_GROUP_LINE:
                {
                    const RLine &rLine = rModel.getLine(entityID.getEid());
                    for (uint i=0;i<rLine.size();i++)
                    {
                        const RElement &rElement = rModel.getElement(rLine.get(i));

                        RR3Vector center;
                        RR3Vector d1, d2, d3;

                        rElement.findCenter(rModel.getNodes(),center[0],center[1],center[2]);

                        if (this->bc.getExplicitLocalDirection())
                        {
                            localDirections.append(RLocalDirection(center,this->bc.getLocalDirection()));
                            continue;
                        }

                        RSegment(rModel.getNode(rElement.getNodeId(0)),rModel.getNode(rElement.getNodeId(1))).findPerpendicularVectors(d1,d2,d3);

                        localDirections.append(RLocalDirection(center,d2));
                        localDirections.append(RLocalDirection(center,d3));
                    }
                    break;
                }
                case R_ENTITY_GROUP_SURFACE:
                {
                    const RSurface &rSurface = rModel.getSurface(entityID.getEid());
                    for (uint i=0;i<rSurface.size();i++)
                    {
                        const RElement &rElement = rModel.getElement(rSurface.get(i));

                        RR3Vector center;
                        RR3Vector normal;
                        rElement.findCenter(rModel.getNodes(),center[0],center[1],center[2]);

                        if (this->bc.getExplicitLocalDirection())
                        {
                            localDirections.append(RLocalDirection(center,this->bc.getLocalDirection()));
                            continue;
                        }

                        rElement.findNormal(rModel.getNodes(),normal[0],normal[1],normal[2]);

                        localDirections.append(RLocalDirection(center,normal));
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        foreach (uint nodeID, nodeIDs)
        {
            const RNode &rNode = rModel.getNode(nodeID);
            localDirections.append(RLocalDirection(rNode.toVector(),bc.getLocalDirection()));
        }
    }

    return localDirections;
}

void BCTree::onBcSelected(RBoundaryConditionType bcType, bool applied)
{
    if (applied)
    {
        foreach (const SessionEntityID entityID,Application::instance()->getSession()->getSelectedEntityIDs())
        {
            switch (entityID.getType())
            {
                case R_ENTITY_GROUP_POINT:
                    this->bc = Application::instance()->getSession()->getModel(entityID.getMid()).getPoint(entityID.getEid()).getBoundaryCondition(bcType);
                    break;
                case R_ENTITY_GROUP_LINE:
                    this->bc = Application::instance()->getSession()->getModel(entityID.getMid()).getLine(entityID.getEid()).getBoundaryCondition(bcType);
                    break;
                case R_ENTITY_GROUP_SURFACE:
                    this->bc = Application::instance()->getSession()->getModel(entityID.getMid()).getSurface(entityID.getEid()).getBoundaryCondition(bcType);
                    break;
                case R_ENTITY_GROUP_VOLUME:
                    this->bc = Application::instance()->getSession()->getModel(entityID.getMid()).getVolume(entityID.getEid()).getBoundaryCondition(bcType);
                    break;
                default:
                    this->bc.setType(bcType);
                    break;
            }
        }

        this->populate();
    }
    else
    {
        this->bc.setType(R_BOUNDARY_CONDITION_NONE);
        this->drawLocalRotationEnd();
        this->clear();
    }
}

void BCTree::onBcValueChanged(RVariableType variableType, double value)
{
    for (uint i=0;i<this->bc.size();i++)
    {
        if (this->bc.getComponent(i).getType() == variableType)
        {
            this->bc.getComponent(i).add(0,value);
        }
    }

    this->updateSelectedEntities();
}

void BCTree::onButtonValueClicked(int id)
{
    unsigned int cPosition = this->bc.findComponentPosition(RVariableType(id));
    if (cPosition == RConstants::eod)
    {
        return;
    }
    ComponentEditDialog componentEditDialog(this->bc.getComponent(cPosition),Application::instance()->getMainWindow());
    if (componentEditDialog.exec() == QDialog::Accepted)
    {
        this->updateSelectedEntities();
        for (int i=0;i<this->topLevelItemCount();i++)
        {
            RVariableType variableType = RVariableType(this->topLevelItem(i)->data(BC_TREE_PROPERTY_TYPE,Qt::DisplayRole).toInt());
            if (variableType != RVariableType(id))
            {
                continue;
            }
            VariableValueEdit *lineEdit = dynamic_cast<VariableValueEdit*>(this->itemWidget(this->topLevelItem(i),BC_TREE_PROPERTY_VALUE));
            lineEdit->setValue(this->bc.getComponent(cPosition).getValue(0));
            lineEdit->setEnabled(this->bc.getComponent(cPosition).size() == 1);
        }
    }
}

void BCTree::onDirectionChanged(const RR3Vector &direction)
{
    this->bc.setLocalDirection(direction);
    this->drawLocalRotationBegin();

    this->updateSelectedEntities();
}
