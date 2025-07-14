#include <QStandardItemModel>
#include <QLineEdit>
#include <QPushButton>

#include "application.h"
#include "ec_tree.h"
#include "variable_value_edit.h"

typedef enum _ECTreeColumn
{
    EC_TREE_PROPERTY_NAME = 0,
    EC_TREE_PROPERTY_VALUE,
    EC_TREE_PROPERTY_UNITS,
    EC_TREE_PROPERTY_TYPE,
    EC_TREE_N_COLUMNS
} ECTreeColumn;

ECTree::ECTree(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setRootIsDecorated(false);
    this->setSelectionMode(QAbstractItemView::NoSelection);

    this->setColumnCount(EC_TREE_N_COLUMNS);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(EC_TREE_PROPERTY_TYPE,QString("Property type"));
    headerItem->setText(EC_TREE_PROPERTY_NAME,QString("Property name"));
    headerItem->setText(EC_TREE_PROPERTY_VALUE,QString("Value"));
    headerItem->setText(EC_TREE_PROPERTY_UNITS,QString("Units"));
    this->setHeaderItem(headerItem);

    this->setColumnHidden(EC_TREE_PROPERTY_TYPE,true);

    this->populate();
}

void ECTree::populate()
{
    this->clear();

    for (uint i=0;i<this->ec.size();i++)
    {
        RConditionComponent component = this->ec.getComponent(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setData(EC_TREE_PROPERTY_TYPE,Qt::DisplayRole,QVariant(component.getType()));
        item->setText(EC_TREE_PROPERTY_NAME,component.getName());
        item->setText(EC_TREE_PROPERTY_UNITS,component.getUnits());
        VariableValueEdit *lineEdit = new VariableValueEdit(component.getType());
        lineEdit->setValue(component.getValue(0));
        this->setItemWidget(item,EC_TREE_PROPERTY_VALUE,lineEdit);

        QObject::connect(lineEdit,&VariableValueEdit::valueChanged,this,&ECTree::onEcValueChanged);
    }

    this->resizeColumnToContents(EC_TREE_PROPERTY_NAME);
    this->resizeColumnToContents(EC_TREE_PROPERTY_VALUE);
    this->resizeColumnToContents(EC_TREE_PROPERTY_UNITS);
}

void ECTree::updateSelectedEntities() const
{
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        switch (entityIDs[i].getType())
        {
            case R_ENTITY_GROUP_POINT:
                Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getPoint(entityIDs[i].getEid()).addEnvironmentCondition(this->ec);
                break;
            case R_ENTITY_GROUP_LINE:
                Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getLine(entityIDs[i].getEid()).addEnvironmentCondition(this->ec);
                break;
            case R_ENTITY_GROUP_SURFACE:
                Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getSurface(entityIDs[i].getEid()).addEnvironmentCondition(this->ec);
                break;
            case R_ENTITY_GROUP_VOLUME:
                Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getVolume(entityIDs[i].getEid()).addEnvironmentCondition(this->ec);
                break;
            default:
                break;
        }
        Application::instance()->getSession()->setEnvironmentConditionChanged(entityIDs[i].getMid(),
                                                           entityIDs[i].getType(),
                                                           entityIDs[i].getEid());
    }

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<modelIDs.size();i++)
    {
        Application::instance()->getSession()->setProblemChanged(modelIDs[i]);
    }
}

void ECTree::onEcSelected(REnvironmentConditionType ecType, bool applied)
{
    if (applied)
    {
        QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

        for (int i=0;i<entityIDs.size();i++)
        {
            switch (entityIDs[i].getType())
            {
                case R_ENTITY_GROUP_POINT:
                    this->ec = Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getPoint(entityIDs[i].getEid()).getEnvironmentCondition(ecType);
                    break;
                case R_ENTITY_GROUP_LINE:
                    this->ec = Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getLine(entityIDs[i].getEid()).getEnvironmentCondition(ecType);
                    break;
                case R_ENTITY_GROUP_SURFACE:
                    this->ec = Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getSurface(entityIDs[i].getEid()).getEnvironmentCondition(ecType);
                    break;
                case R_ENTITY_GROUP_VOLUME:
                    this->ec = Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getVolume(entityIDs[i].getEid()).getEnvironmentCondition(ecType);
                    break;
                default:
                    this->ec.setType(ecType);
                    break;
            }
        }

        this->populate();
    }
    else
    {
        this->ec.setType(R_ENVIRONMENT_CONDITION_NONE);
        this->clear();
    }
}

void ECTree::onEcValueChanged(RVariableType variableType, double value)
{
    for (uint i=0;i<this->ec.size();i++)
    {
        if (this->ec.getComponent(i).getType() == variableType)
        {
            this->ec.getComponent(i).add(0,value);
        }
    }

    this->updateSelectedEntities();
}
