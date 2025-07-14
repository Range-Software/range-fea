#include <QAbstractItemModel>
#include <QFileDialog>
#include <QVector2D>
#include <QTextStream>
#include <QHeaderView>

#include <rbl_logger.h>

#include "application.h"
#include "value_table.h"
#include "graph_dialog.h"
#include "graph_object.h"
#include "value_set_generator_dialog.h"

namespace ValueTableTreeTypes
{
    typedef enum _Column
    {
        COLUMN_KEY = 0,
        COLUMN_VALUE,
        N_COLUMNS
    } Column;
}

ValueTable::ValueTable(QWidget *parent) :
    QTableWidget(parent),
    dataType(R_VARIABLE_DATA_DOUBLE)
{
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);

    this->setColumnCount(ValueTableTreeTypes::N_COLUMNS);

    this->setHorizontalHeaderItem(ValueTableTreeTypes::COLUMN_KEY,new QTableWidgetItem);
    this->setHorizontalHeaderItem(ValueTableTreeTypes::COLUMN_VALUE,new QTableWidgetItem);

    this->setKeyHeader("Key");
    this->setValueHeader("Value");

    this->resizeColumnsToContents();

    this->horizontalHeader()->setStretchLastSection(true);

    this->setContextMenuPolicy(Qt::ActionsContextMenu);

    this->insertValueAction = new QAction(this);
    this->insertValueAction->setText(tr("Insert"));
    this->addAction(this->insertValueAction);

    QObject::connect(this->insertValueAction,
                     &QAction::triggered,
                     this,
                     &ValueTable::onInsertValue);

    this->deleteValueAction = new QAction(this);
    this->deleteValueAction->setText(tr("Remove selected"));
    this->addAction(this->deleteValueAction);

    QObject::connect(this->deleteValueAction,
                     &QAction::triggered,
                     this,
                     &ValueTable::onDeleteValue);

    QAction *separator1 = new QAction(this);
    separator1->setSeparator(true);
    this->addAction(separator1);

    this->importFromFileAction = new QAction(this);
    this->importFromFileAction->setText(tr("Import from file"));
    this->importFromFileAction->setIcon(QIcon(":/icons/file/pixmaps/range-open.svg"));
    this->addAction(this->importFromFileAction);

    QObject::connect(this->importFromFileAction,
                     &QAction::triggered,
                     this,
                     &ValueTable::onImportFromFile);

    this->fillValuesAction = new QAction(this);
    this->fillValuesAction->setText(tr("Fill values"));
    this->addAction(this->fillValuesAction);

    QObject::connect(this->fillValuesAction,
                     &QAction::triggered,
                     this,
                     &ValueTable::onFillValues);

    this->viewGraphAction = new QAction(this);
    this->viewGraphAction->setText(tr("View graph"));
    this->addAction(this->viewGraphAction);

    QAction *separator2 = new QAction(this);
    separator2->setSeparator(true);
    this->addAction(separator2);

    QObject::connect(this->viewGraphAction,
                     &QAction::triggered,
                     this,
                     &ValueTable::onViewGraph);
}

void ValueTable::setDataType(RVariableDataType dataType)
{
    this->dataType = dataType;
}

void ValueTable::setKeyHeader(const QString &header)
{
    this->horizontalHeaderItem(ValueTableTreeTypes::COLUMN_KEY)->setText(header);
    this->resizeColumnToContents(ValueTableTreeTypes::COLUMN_KEY);
}

void ValueTable::setValueHeader(const QString &header)
{
    this->horizontalHeaderItem(ValueTableTreeTypes::COLUMN_VALUE)->setText(header);
}

void ValueTable::addValue(double key, double value, int rowNumber)
{
    int iRow = rowNumber >= 0 && rowNumber < this->rowCount() ? rowNumber : this->rowCount();

    this->insertRow(iRow);

    QTableWidgetItem *itemKey = new QTableWidgetItem;
    itemKey->setText(QLocale().toString(key));
    itemKey->setData(Qt::UserRole,QVariant(key));
    this->setItem(iRow,ValueTableTreeTypes::COLUMN_KEY,itemKey);

    QTableWidgetItem *itemValue = new QTableWidgetItem;
    itemValue->setData(Qt::UserRole,QVariant(value));
    if (this->dataType == R_VARIABLE_DATA_BOOLEAN)
    {
        itemValue->setData(Qt::CheckStateRole,QVariant(value != 0.0 ? Qt::Checked : Qt::Unchecked));
    }
    else
    {
        itemValue->setText(QLocale().toString(value));
    }
    this->setItem(iRow,ValueTableTreeTypes::COLUMN_VALUE,itemValue);

    QObject::connect(this,
                     &QTableWidget::itemChanged,
                     this,
                     &ValueTable::onItemChanged);

    this->resizeColumnToContents(ValueTableTreeTypes::COLUMN_KEY);

    emit this->sizeChanged(uint(this->rowCount()));
}

void ValueTable::removeValue(int rowNumber)
{
    this->removeRow(rowNumber);
    emit this->sizeChanged(uint(this->rowCount()));
}

double ValueTable::getKey(uint row) const
{
    return QLocale().toDouble(this->item(int(row),ValueTableTreeTypes::COLUMN_KEY)->text());
}

double ValueTable::getValue(uint row) const
{
    if (this->dataType == R_VARIABLE_DATA_BOOLEAN)
    {
        return (this->item(int(row),ValueTableTreeTypes::COLUMN_VALUE)->data(Qt::CheckStateRole).toInt() != Qt::Unchecked ? 1.0 : 0.0);
    }
    else
    {
        return QLocale().toDouble(this->item(int(row),ValueTableTreeTypes::COLUMN_VALUE)->text());
    }
}

void ValueTable::onItemChanged(QTableWidgetItem *item)
{
    bool ok = true;
    double value = 0.0;

    switch (item->column())
    {
        case ValueTableTreeTypes::COLUMN_VALUE:
            if (this->dataType == R_VARIABLE_DATA_BOOLEAN)
            {
                value = item->data(Qt::CheckStateRole).toInt(&ok) != Qt::Unchecked ? 1.0 : 0.0;
            }
            else
            {
                value = QLocale().toDouble(item->text(),&ok);
            }
            if (!ok)
            {
                if (this->dataType == R_VARIABLE_DATA_BOOLEAN)
                {
                    item->setData(Qt::CheckStateRole,QVariant(value != 0.0 ? Qt::Checked : Qt::Unchecked));
                }
                else
                {
                    item->setText(QLocale().toString(value));
                }
                break;
            }
            item->setData(Qt::UserRole,QVariant(value));
            break;
        case ValueTableTreeTypes::COLUMN_KEY:
            value = QLocale().toDouble(item->text(),&ok);
            if (!ok)
            {
                item->setText(QLocale().toString(value));
                break;
            }
            item->setData(Qt::UserRole,QVariant(value));
            break;
        default:
            break;
    }
}

void ValueTable::onInsertValue()
{
    QList<QTableWidgetItem*> selectionRangeList = this->selectedItems();
    QListIterator<QTableWidgetItem*> selectionRangeListIter(selectionRangeList);

    QList<int> selectedRows;

    while(selectionRangeListIter.hasNext())
    {
        int selectedRow = this->row(selectionRangeListIter.next());
        if (!selectedRows.contains(selectedRow))
        {
            selectedRows.append(selectedRow);
        }
    }

    std::sort(selectedRows.begin(), selectedRows.end(), std::greater<int>());

    for (int i=0;i<selectedRows.size();i++)
    {
        this->addValue(this->item(selectedRows[i],ValueTableTreeTypes::COLUMN_KEY)->data(Qt::UserRole).toDouble(),
                       this->item(selectedRows[i],ValueTableTreeTypes::COLUMN_VALUE)->data(Qt::UserRole).toDouble(),
                       selectedRows[i]+1);
    }
}

void ValueTable::onDeleteValue()
{
    QList<QTableWidgetItem*> selectionRangeList = this->selectedItems();
    QListIterator<QTableWidgetItem*> selectionRangeListIter(selectionRangeList);

    while(selectionRangeListIter.hasNext())
    {
        this->removeValue(this->row(selectionRangeListIter.next()));
    }
}

void ValueTable::onImportFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(dynamic_cast<QWidget*>(this->parent()),
                                                    tr("Open data file"),
                                                    Application::instance()->getApplicationSettings()->getHomeDir(),
                                                    "Any files (*)");
    if (fileName.isEmpty())
    {
        return;
    }

    while (this->rowCount() > 0)
    {
        this->removeRow(0);
    }

    RLogger::info("Reading file \'%s\'\n",fileName.toUtf8().constData());

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        RLogger::warning("Failed to open the file \'%s\' for reading.\n", fileName.toUtf8().constData());
        return;
    }

    try
    {
        QTextStream in(&file);

        bool isOdd = true;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList stringList = line.split(' ',Qt::SkipEmptyParts);

            QVector2D value;
            for (int i=0;i<stringList.size();i++)
            {
                value[isOdd?0:1] = stringList[i].toFloat();
                if (!isOdd)
                {
                    this->addValue(double(value[0]),double(value[1]));
                }
                isOdd = !isOdd;
            }
        }
    }
    catch (...)
    {
        RLogger::warning("Failed to read the file \'%s\'.\n", fileName.toUtf8().constData());
    }
    file.close();
}

void ValueTable::onViewGraph()
{
    GraphObject *graphObject = new GraphObject;
    GraphData &rGraphData = graphObject->getData();

    rGraphData.setTitle("Table values");
    rGraphData.setXLabel(this->horizontalHeaderItem(ValueTableTreeTypes::COLUMN_KEY)->text());
    rGraphData.setYLabel(this->horizontalHeaderItem(ValueTableTreeTypes::COLUMN_VALUE)->text());
    rGraphData.setTypeMask(GRAPH_LINES);

    double minKey = 0, maxKey = 0;
    for (int i=0;i<this->rowCount();i++)
    {
        double key = QLocale().toDouble(this->item(i,ValueTableTreeTypes::COLUMN_KEY)->text());
        RRVector &rValues = rGraphData[key];
        rValues.resize(1);
        rValues[0] = QLocale().toDouble(this->item(i,ValueTableTreeTypes::COLUMN_VALUE)->text());
        if (i==0)
        {
            minKey = maxKey = key;
        }
        minKey = std::min(minKey,key);
        maxKey = std::max(maxKey,key);
    }

    rGraphData.setLimits(minKey,maxKey);

    GraphDialog graphDialog(graphObject, Application::instance()->getMainWindow());
    graphDialog.getGraphWidget()->setShowPointer(true,true,true);
    graphDialog.exec();
}

void ValueTable::onFillValues()
{
    RValueTable valueTable;
    valueTable.setKeyName(this->horizontalHeaderItem(ValueTableTreeTypes::COLUMN_KEY)->text());
    valueTable.setValueName(this->horizontalHeaderItem(ValueTableTreeTypes::COLUMN_VALUE)->text());
    for (int i=0;i<this->rowCount();i++)
    {
        valueTable.add(QLocale().toDouble(this->item(i,ValueTableTreeTypes::COLUMN_KEY)->text()),
                       QLocale().toDouble(this->item(i,ValueTableTreeTypes::COLUMN_VALUE)->text()));
    }
    ValueSetGeneratorDialog valueSetGeneratorDialog(valueTable,this);
    if (valueSetGeneratorDialog.exec() == QDialog::Accepted)
    {
        bool isSomethingSelected = (this->selectedItems().size() > 0);
        for (int i=0;i<this->rowCount();i++)
        {
            if (!isSomethingSelected || this->item(i,ValueTableTreeTypes::COLUMN_VALUE)->isSelected())
            {
                this->item(i,ValueTableTreeTypes::COLUMN_VALUE)->setText(QLocale().toString(valueTable.getValue(uint(i))));
            }
        }
    }
}
