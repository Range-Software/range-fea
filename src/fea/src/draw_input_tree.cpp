#include "draw_input_tree.h"
#include "application.h"

typedef enum _DrawInputTreeColumn
{
    DRAW_INPUT_TREE_COLUMN_1 = 0,
    DRAW_INPUT_TREE_COLUMN_2,
    DRAW_INPUT_TREE_COLUMN_3,
    DRAW_INPUT_TREE_COLUMN_4,
    DRAW_INPUT_TREE_N_COLUMNS
} DrawInputTreeColumn;

DrawInputTree::DrawInputTree(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setColumnCount(DRAW_INPUT_TREE_N_COLUMNS);
    this->setHeaderHidden(true);
    this->setRootIsDecorated(false);

    this->populate();

    QObject::connect(Application::instance()->getSession()->getDrawEngine(),
                     &DrawEngine::objectAdded,
                     this,
                     &DrawInputTree::onDrawObjectAdded);

    QObject::connect(Application::instance()->getSession()->getDrawEngine(),
                     &DrawEngine::objectRemoved,
                     this,
                     &DrawInputTree::onDrawObjectRemoved);

    QObject::connect(Application::instance()->getSession()->getDrawEngine(),
                     &DrawEngine::objectsRemoved,
                     this,
                     &DrawInputTree::onDrawObjectsRemoved);

    QObject::connect(this,
                     &QTreeWidget::itemDoubleClicked,
                     this,
                     &DrawInputTree::onItemDoubleClicked);

    QObject::connect(this,
                     &QTreeWidget::itemChanged,
                     this,
                     &DrawInputTree::onItemChanged);

    QObject::connect(this,
                     &QTreeWidget::itemSelectionChanged,
                     this,
                     &DrawInputTree::onItemSelectionChanged);

//    QObject::connect(Application::instance()->getSession(),
//                     &Session::pickListChanged,
//                     this,
//                     &DrawInputTree::onPickListChanged);
//    QObject::connect(Application::instance()->getSession(),
//                     &Session::modelAdded,
//                     this,
//                     &DrawInputTree::onPickListChanged);
//    QObject::connect(Application::instance()->getSession(),
//                     &Session::modelRemoved,
//                     this,
//                     &DrawInputTree::onPickListChanged);
//    QObject::connect(Application::instance()->getSession(),
//                     &Session::modelChanged,
//                     this,
//                     &DrawInputTree::onPickListChanged);
}

QList<uint> DrawInputTree::getSelectedObjectIDs() const
{
    QList<QTreeWidgetItem*> selectedItems = this->selectedItems();
    QSet<uint> objectIDs;

    for (int i=0;i<selectedItems.size();i++)
    {
        objectIDs.insert(this->getItemObjectID(selectedItems[i]));
    }
    return QList<uint>(objectIDs.begin(),objectIDs.end());
}

void DrawInputTree::setRequestedItemVectorValue(const RR3Vector &v)
{
    if (this->requestedItem)
    {
        DrawInputTree::setItemVectorValue(this->requestedItem,v);
    }
}

void DrawInputTree::setRequestedItemLocalDirectionValue(const RLocalDirection &localDirection)
{
    if (this->requestedItem)
    {
        DrawInputTree::setItemLocalDirectionValue(this->requestedItem,localDirection);
    }
}

void DrawInputTree::setRequestedItemTextValue(const QString &text)
{
    if (this->requestedItem)
    {
        DrawInputTree::setItemTextValue(this->requestedItem,text);
    }
}

void DrawInputTree::releaseRequestedItem()
{
    this->requestedItem = 0;
}

void DrawInputTree::populate()
{
    this->blockSignals(true);

    this->clear();

    for (uint i=0;i<Application::instance()->getSession()->getDrawEngine()->getNObjects();i++)
    {
        const DrawEngineObject *object = Application::instance()->getSession()->getDrawEngine()->getObject(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setText(DRAW_INPUT_TREE_COLUMN_1,object->getName());
        item->setFirstColumnSpanned(true);

        item->setForeground(DRAW_INPUT_TREE_COLUMN_1,this->palette().brush(QPalette::Active,QPalette::Text));
        QFont font1(item->font(DRAW_INPUT_TREE_COLUMN_1));
        font1.setBold(true);
        item->setFont(DRAW_INPUT_TREE_COLUMN_1,font1);

        for (uint j=0;j<object->getNInputParameters();j++)
        {
            QTreeWidgetItem *childItem = new QTreeWidgetItem(item);
            childItem->setText(DRAW_INPUT_TREE_COLUMN_1,object->getInputParamater(j).getName());
            childItem->setData(DRAW_INPUT_TREE_COLUMN_1,Qt::UserRole,QVariant(object->getInputParamater(j).getType()));
            childItem->setText(DRAW_INPUT_TREE_COLUMN_3,object->getInputParamater(j).getUnits());
            childItem->setText(DRAW_INPUT_TREE_COLUMN_4,object->getInputParamater(j).getDescription());

            childItem->setForeground(DRAW_INPUT_TREE_COLUMN_2,this->palette().brush(QPalette::Active,QPalette::Text));
            QFont font2(childItem->font(DRAW_INPUT_TREE_COLUMN_2));
            font2.setBold(true);
            childItem->setFont(DRAW_INPUT_TREE_COLUMN_2,font2);

            childItem->setForeground(DRAW_INPUT_TREE_COLUMN_4,this->palette().brush(QPalette::Active,QPalette::Text));
            QFont font4(childItem->font(DRAW_INPUT_TREE_COLUMN_4));
            font4.setItalic(true);
            childItem->setFont(DRAW_INPUT_TREE_COLUMN_4,font4);

            switch (object->getInputParamater(j).getType())
            {
                case DrawEngineInput::Bool:
                {
                    childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable);
                    childItem->setCheckState(DRAW_INPUT_TREE_COLUMN_2,object->getInputParamater(j).toBool()?Qt::Checked:Qt::Unchecked);
                    break;
                }
                case DrawEngineInput::Int:
                {
                    childItem->setText(DRAW_INPUT_TREE_COLUMN_2,QLocale().toString(object->getInputParamater(j).toInt()));
                    break;
                }
                case DrawEngineInput::Uint:
                {
                    childItem->setText(DRAW_INPUT_TREE_COLUMN_2,QLocale().toString(object->getInputParamater(j).toUint()));
                    break;
                }
                case DrawEngineInput::Double:
                {
                    childItem->setText(DRAW_INPUT_TREE_COLUMN_2,QLocale().toString(object->getInputParamater(j).toDouble()));
                    break;
                }
                case DrawEngineInput::Vector:
                {
                    DrawInputTree::setItemVectorValue(childItem,object->getInputParamater(j).toVector());
                    break;
                }
                case DrawEngineInput::LocalDirection:
                {
                    DrawInputTree::setItemLocalDirectionValue(childItem,object->getInputParamater(j).toLocalDirection());
                    break;
                }
                case DrawEngineInput::Text:
                {
                    DrawInputTree::setItemTextValue(childItem,object->getInputParamater(j).toText());
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }

    this->expandAll();

    this->resizeColumnToContents(DRAW_INPUT_TREE_COLUMN_1);
    this->resizeColumnToContents(DRAW_INPUT_TREE_COLUMN_2);
    this->resizeColumnToContents(DRAW_INPUT_TREE_COLUMN_3);
    this->resizeColumnToContents(DRAW_INPUT_TREE_COLUMN_4);

    this->blockSignals(false);
}

uint DrawInputTree::getItemObjectID(QTreeWidgetItem *item) const
{
    if (item->childCount() > 0)
    {
        return uint(this->indexOfTopLevelItem(item));
    }
    else
    {
        return uint(this->indexOfTopLevelItem(item->parent()));
    }
}

void DrawInputTree::setItemVectorValue(QTreeWidgetItem *item, const RR3Vector &v)
{
    QString vStr = QLocale().toString(v[0]) + ";" + QLocale().toString(v[1]) + ";" + QLocale().toString(v[2]);
    item->setText(DRAW_INPUT_TREE_COLUMN_2,vStr);
}

RR3Vector DrawInputTree::getItemVectorValue(QTreeWidgetItem *item)
{
    return DrawInputTree::stringToVector(item->text(DRAW_INPUT_TREE_COLUMN_2));
}

RR3Vector DrawInputTree::stringToVector(const QString &vectorStr)
{
    RR3Vector v;

    QStringList strList = vectorStr.split(';');
    if (strList.size() > 0)
    {
        v[0] = QLocale().toDouble(strList[0]);
    }
    if (strList.size() > 1)
    {
        v[1] = QLocale().toDouble(strList[1]);
    }
    if (strList.size() > 2)
    {
        v[2] = QLocale().toDouble(strList[2]);
    }

    return v;
}

void DrawInputTree::setItemLocalDirectionValue(QTreeWidgetItem *item, const RLocalDirection &localDirectionStr)
{
    const RR3Vector &p = localDirectionStr.getPosition();
    const RR3Vector &d = localDirectionStr.getDirection();
    QString vStr = QLocale().toString(p[0]) + ";" + QLocale().toString(p[1]) + ";" + QLocale().toString(p[2]) + "/" + QLocale().toString(d[0]) + ";" + QLocale().toString(d[1]) + ";" + QLocale().toString(d[2]);
    item->setText(DRAW_INPUT_TREE_COLUMN_2,vStr);
}

RLocalDirection DrawInputTree::getItemLocalDirectionValue(QTreeWidgetItem *item)
{
    return DrawInputTree::stringToLocalDirection(item->text(DRAW_INPUT_TREE_COLUMN_2));
}

RLocalDirection DrawInputTree::stringToLocalDirection(const QString &localDirectionStr)
{
    RR3Vector position(0.0,0.0,0.0);
    RR3Vector direction(0.0,0.0,1.0);

    QStringList strList = localDirectionStr.split('/');
    if (strList.size() > 0)
    {
        position = DrawInputTree::stringToVector(strList[0]);
    }
    if (strList.size() > 1)
    {
        direction = DrawInputTree::stringToVector(strList[1]);
    }
    return RLocalDirection(position,direction);
}

void DrawInputTree::setItemTextValue(QTreeWidgetItem *item, const QString &text)
{
    int nLines = 0;
    if (!text.isEmpty())
    {
        nLines++;
    }
    nLines += text.count(QLatin1Char('\n'));

    item->setText(DRAW_INPUT_TREE_COLUMN_2,"content (" + QLocale().toString(nLines) + " lines)");
    item->setData(DRAW_INPUT_TREE_COLUMN_2,Qt::UserRole,QVariant(text));
}

QString DrawInputTree::getItemTextValue(QTreeWidgetItem *item)
{
    return item->data(DRAW_INPUT_TREE_COLUMN_2,Qt::UserRole).toString();
}

void DrawInputTree::onDrawObjectAdded()
{
    this->populate();
}

void DrawInputTree::onDrawObjectRemoved(uint position)
{
    delete this->takeTopLevelItem(position);
}

void DrawInputTree::onDrawObjectsRemoved()
{
    this->populate();
}

void DrawInputTree::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (column == DRAW_INPUT_TREE_COLUMN_2)
    {
        if (item->data(DRAW_INPUT_TREE_COLUMN_1,Qt::UserRole).toInt() == DrawEngineInput::Vector)
        {
            this->requestedItem = item;
            emit this->positionRequest(DrawInputTree::getItemVectorValue(item));
        }
        else if (item->data(DRAW_INPUT_TREE_COLUMN_1,Qt::UserRole).toInt() == DrawEngineInput::LocalDirection)
        {
            this->requestedItem = item;
            emit this->localDirectionRequest(DrawInputTree::getItemLocalDirectionValue(item));
        }
        else if (item->data(DRAW_INPUT_TREE_COLUMN_1,Qt::UserRole).toInt() == DrawEngineInput::Text)
        {
            this->requestedItem = item;
            emit this->textRequest(DrawInputTree::getItemTextValue(item));
        }
        else
        {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    }
    else
    {
        if (item->flags() & Qt::ItemIsEditable)
        {
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        }
    }
}

void DrawInputTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (column != DRAW_INPUT_TREE_COLUMN_2)
    {
        return;
    }

    QString valueStr = item->text(DRAW_INPUT_TREE_COLUMN_2);

    uint objectID = this->getItemObjectID(item);
    uint parameterID = uint(item->parent()->indexOfChild(item));

    DrawEngineObject *object = Application::instance()->getSession()->getDrawEngine()->getObject(objectID);
    bool valueChanged = false;

    this->blockSignals(true);
    switch (item->data(DRAW_INPUT_TREE_COLUMN_1,Qt::UserRole).toInt())
    {
        case DrawEngineInput::Bool:
        {
            bool value = item->checkState(DRAW_INPUT_TREE_COLUMN_2) != Qt::Unchecked;
            valueChanged = object->getInputParamater(parameterID).setValue(value);
            break;
        }
        case DrawEngineInput::Int:
        {
            int value = QLocale().toInt(valueStr);
            if (!(valueChanged = object->getInputParamater(parameterID).setValue(value)))
            {
                value = object->getInputParamater(parameterID).toInt();
            }
            item->setText(DRAW_INPUT_TREE_COLUMN_2,QLocale().toString(value));
            break;
        }
        case DrawEngineInput::Uint:
        {
            uint value = QLocale().toUInt(valueStr);
            if (!(valueChanged = object->getInputParamater(parameterID).setValue(value)))
            {
                value = object->getInputParamater(parameterID).toUint();
            }
            item->setText(DRAW_INPUT_TREE_COLUMN_2,QLocale().toString(value));
            break;
        }
        case DrawEngineInput::Double:
        {
            double value = QLocale().toDouble(valueStr);
            if (!(valueChanged = object->getInputParamater(parameterID).setValue(value)))
            {
                value = object->getInputParamater(parameterID).toDouble();
            }
            item->setText(DRAW_INPUT_TREE_COLUMN_2,QLocale().toString(value));
            break;
        }
        case DrawEngineInput::Vector:
        {
            RR3Vector v(DrawInputTree::stringToVector(valueStr));
            if (!(valueChanged = object->getInputParamater(parameterID).setValue(v)))
            {
                v = object->getInputParamater(parameterID).toVector();
            }
            DrawInputTree::setItemVectorValue(item,v);
            break;
        }
        case DrawEngineInput::LocalDirection:
        {
            RLocalDirection localDirection(DrawInputTree::stringToLocalDirection(valueStr));
            if (!(valueChanged = object->getInputParamater(parameterID).setValue(localDirection)))
            {
                localDirection = object->getInputParamater(parameterID).toLocalDirection();
            }
            DrawInputTree::setItemLocalDirectionValue(item,localDirection);
            break;
        }
        case DrawEngineInput::Text:
        {
            QString valueStr = item->data(DRAW_INPUT_TREE_COLUMN_2,Qt::UserRole).toString();
            if (!(valueChanged = object->getInputParamater(parameterID).setValue(valueStr)))
            {
                valueStr = object->getInputParamater(parameterID).toText();
            }
            DrawInputTree::setItemTextValue(item,valueStr);
            break;
        }
        default:
        {
            break;
        }
    }

    if (valueChanged)
    {
        Application::instance()->getSession()->getDrawEngine()->setObjectChanged(objectID);
    }

    this->blockSignals(false);
    this->resizeColumnToContents(DRAW_INPUT_TREE_COLUMN_2);

    emit this->inputParameterChanged(objectID,parameterID);
}

void DrawInputTree::onItemSelectionChanged()
{
    emit this->objectSelectionChanged(this->getSelectedObjectIDs());
}
