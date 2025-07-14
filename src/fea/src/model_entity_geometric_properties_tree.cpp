#include "application.h"
#include "model_entity_geometric_properties_tree.h"
#include "value_line_edit.h"

typedef enum _GeometricPropertyType
{
    GEOMETRY_PROPERTY_NONE = 0,
    GEOMETRY_PROPERTY_VOLUME,
    GEOMETRY_PROPERTY_CROSS_AREA,
    GEOMETRY_PROPERTY_THICKNESS,
    GEOMETRY_PROPERTY_N_TYPES
} GeometricPropertyType;

typedef enum _GeometricPropertiesTreeColumn
{
    GP_TREE_COLUMN_NAME = 0,
    GP_TREE_COLUMN_VALUE,
    GP_TREE_COLUMN_UNITS,
    GP_TREE_N_COLUMNS
} GeometricPropertiesTreeColumn;

ModelEntityGeometricPropertiesTree::ModelEntityGeometricPropertiesTree(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setColumnCount(GP_TREE_N_COLUMNS);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setRootIsDecorated(false);
    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(GP_TREE_COLUMN_NAME,QString("Geometric properties"));
    headerItem->setText(GP_TREE_COLUMN_VALUE,QString("Value"));
    headerItem->setText(GP_TREE_COLUMN_UNITS,QString("Units"));
    headerItem->setFirstColumnSpanned(true);
    this->setHeaderItem(headerItem);

    QObject::connect(Application::instance()->getSession(),&Session::modelSelectionChanged,
                     this,&ModelEntityGeometricPropertiesTree::onModelSelectionChanged);
    QObject::connect(this,&ModelEntityGeometricPropertiesTree::itemChanged,
                     this,&ModelEntityGeometricPropertiesTree::onItemChanged);
    QObject::connect(this,&ModelEntityGeometricPropertiesTree::itemDoubleClicked,
                     this,&ModelEntityGeometricPropertiesTree::onItemDoubleClicked);

    this->populate();
}

void ModelEntityGeometricPropertiesTree::populate()
{
    this->clear();

    this->resizeColumnToContents(GP_TREE_COLUMN_NAME);
    this->resizeColumnToContents(GP_TREE_COLUMN_VALUE);
    this->resizeColumnToContents(GP_TREE_COLUMN_UNITS);

    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    if (entityIDs.size() == 0)
    {
        return;
    }

    double pointVolume = 0.0;
    double lineCrossArea = 0.0;
    double surfaceThickness = 0.0;

    bool hasVolume = false;
    bool hasCrossArea = false;
    bool hasThisckness = false;

    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT && !hasVolume)
        {
            pointVolume = Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getPoint(entityIDs[i].getEid()).getVolume();
            hasVolume = true;
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE && !hasCrossArea)
        {
            lineCrossArea = Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getLine(entityIDs[i].getEid()).getCrossArea();
            hasCrossArea = true;
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE && !hasThisckness)
        {
            surfaceThickness = Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getSurface(entityIDs[i].getEid()).getThickness();
            hasThisckness = true;
        }
    }

    this->blockSignals(true);

    if (hasVolume)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setText(GP_TREE_COLUMN_NAME,tr("Point volume"));
        item->setText(GP_TREE_COLUMN_VALUE,QLocale().toString(pointVolume));
        item->setData(GP_TREE_COLUMN_VALUE,Qt::UserRole,QVariant(GEOMETRY_PROPERTY_VOLUME));
        item->setText(GP_TREE_COLUMN_UNITS,RVariable::getUnits(R_VARIABLE_VOLUME));
    }

    if (hasCrossArea)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setText(GP_TREE_COLUMN_NAME,tr("Line cross area"));
        item->setText(GP_TREE_COLUMN_VALUE,QLocale().toString(lineCrossArea));
        item->setData(GP_TREE_COLUMN_VALUE,Qt::UserRole,QVariant(GEOMETRY_PROPERTY_CROSS_AREA));
        item->setText(GP_TREE_COLUMN_UNITS,RVariable::getUnits(R_VARIABLE_AREA));
    }

    if (hasThisckness)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setText(GP_TREE_COLUMN_NAME,tr("Surface thickness"));
        item->setText(GP_TREE_COLUMN_VALUE,QLocale().toString(surfaceThickness));
        item->setData(GP_TREE_COLUMN_VALUE,Qt::UserRole,QVariant(GEOMETRY_PROPERTY_THICKNESS));
        item->setText(GP_TREE_COLUMN_UNITS,RVariable::getUnits(R_VARIABLE_LENGTH));
    }

    this->blockSignals(false);

    this->resizeColumnToContents(GP_TREE_COLUMN_NAME);
    this->resizeColumnToContents(GP_TREE_COLUMN_VALUE);
    this->resizeColumnToContents(GP_TREE_COLUMN_UNITS);
}

void ModelEntityGeometricPropertiesTree::onModelSelectionChanged(uint)
{
    this->populate();
}

void ModelEntityGeometricPropertiesTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (column != GP_TREE_COLUMN_VALUE)
    {
        return;
    }

    GeometricPropertyType type = GeometricPropertyType(item->data(GP_TREE_COLUMN_VALUE,Qt::UserRole).toInt());
    double value = QLocale().toDouble(item->text(GP_TREE_COLUMN_VALUE));

    if (value < 0.0)
    {
        item->setText(GP_TREE_COLUMN_VALUE,QLocale().toString(0.0));
        return;
    }
    else if (value > 1.0e99)
    {
        item->setText(GP_TREE_COLUMN_VALUE,QLocale().toString(1.0e99));
        return;
    }

    QList<uint> selectedModeIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<selectedModeIDs.size();i++)
    {
        Application::instance()->getSession()->storeCurentModelVersion(selectedModeIDs[i],tr("Change geometric properties"));
    }

    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT && type == GEOMETRY_PROPERTY_VOLUME)
        {
            Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getPoint(entityIDs[i].getEid()).setVolume(value);
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE && type == GEOMETRY_PROPERTY_CROSS_AREA)
        {
            Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getLine(entityIDs[i].getEid()).setCrossArea(value);
        }
        else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE && type == GEOMETRY_PROPERTY_THICKNESS)
        {
            Application::instance()->getSession()->getModel(entityIDs[i].getMid()).getSurface(entityIDs[i].getEid()).setThickness(value);
        }
    }

    for (int i=0;i<selectedModeIDs.size();i++)
    {
        Application::instance()->getSession()->setModelChanged(selectedModeIDs[i]);
    }
}

void ModelEntityGeometricPropertiesTree::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
     this->blockSignals(true);
     if (column == GP_TREE_COLUMN_VALUE)
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
