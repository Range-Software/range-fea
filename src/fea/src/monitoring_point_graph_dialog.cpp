#include <QVBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_error.h>
#include <rbl_logger.h>

#include "monitoring_point_graph_dialog.h"
#include "graph_dialog.h"

MonitoringPointGraphDialog::MonitoringPointGraphDialog(const QString &monitoringFileName, QWidget *parent) :
    QDialog(parent)
{
    try
    {
        this->readDataFromFile(monitoringFileName);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to read monitoring point data from file \'%s\'. %s.\n",
                       monitoringFileName.toUtf8().constData(),
                       error.getMessage().toUtf8().constData());
    }

    QIcon closeIcon(":/icons/action/pixmaps/range-close.svg");

    this->setWindowTitle(tr("Monitoring point"));
    this->resize(600,400);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *label = new QLabel("Source: <i>" + monitoringFileName + "</i>");
    mainLayout->addWidget(label);

    this->monitoringPointsCombo = new QComboBox;
    QList<RMonitoringPoint> keys = this->data.keys();
    for (int i=0;i<keys.size();i++)
    {
        QString text(RVariable::getName(keys[i].getVariableType())
                     + " [" + QLocale().toString(keys[i].getPosition()[0])
                     + "; " + QLocale().toString(keys[i].getPosition()[1])
                     + "; " + QLocale().toString(keys[i].getPosition()[2]) + "]");
        this->monitoringPointsCombo->addItem(text);
    }
    mainLayout->addWidget(this->monitoringPointsCombo);

    this->valuesTree = new QTreeWidget;
    this->populateValuesTree();
    mainLayout->addWidget(this->valuesTree);

    QPushButton *showGraphButton = new QPushButton(tr("Display in graph"));
    mainLayout->addWidget(showGraphButton);

    QObject::connect(showGraphButton,&QPushButton::clicked,this,&MonitoringPointGraphDialog::onDisplayInGraphClicked);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *closeButton = new QPushButton(closeIcon, tr("Close"));
    buttonBox->addButton(closeButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->monitoringPointsCombo,&QComboBox::currentIndexChanged,this,&MonitoringPointGraphDialog::onCurrentIndexChanged);
}

int MonitoringPointGraphDialog::exec()
{
    int retVal = QDialog::exec();

    return retVal;
}

void MonitoringPointGraphDialog::readDataFromFile(const QString &fileName)
{
    if (fileName.isEmpty())
    {
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"Empty file name was provided");
    }

    RFile file(fileName,RFile::ASCII);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,"Failed to open the file \'%s\'.",fileName.toUtf8().constData());
    }

    RMonitoringPoint monitoringPoint;
    while (!file.getTextStream().atEnd())
    {
        uint iteration;
        RRVector valueVector;
        try
        {
            monitoringPoint.readRecordFromFile(file,iteration,valueVector);
        }
        catch (const RError &error)
        {
            if (file.getTextStream().atEnd())
            {
                continue;
            }
            file.close();
            throw error;
        }
        this->data[monitoringPoint].append(valueVector);
    }

    file.close();
}

void MonitoringPointGraphDialog::populateValuesTree()
{
    this->valuesTree->clear();

    int index = this->monitoringPointsCombo->currentIndex();
    if (index < 0)
    {
        return;
    }
    QList<RMonitoringPoint> keys = this->data.keys();
    QVector<RRVector> &values = this->data[keys[index]];

    uint nColumns = 0;
    for (int i=0;i<values.size();i++)
    {
        nColumns = std::max(nColumns,uint(values[i].size()));
    }

    this->valuesTree->setColumnCount(nColumns+1);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(0,"#");
    for (uint i=0;i<nColumns;i++)
    {
        headerItem->setText(i+1,QLocale().toString(i+1));
    }
    this->valuesTree->setHeaderItem(headerItem);

    for (int i=0;i<values.size();i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(this->valuesTree);
        item->setText(0,QLocale().toString(i+1));
        for (uint j=0;j<values[i].size();j++)
        {
            item->setText(j+1,QLocale().toString(values[i][j]));
        }
    }

    for (uint i=0;i<nColumns+1;i++)
    {
        this->valuesTree->resizeColumnToContents(i);
    }
}

void MonitoringPointGraphDialog::displayInGraph()
{
    GraphObject *graphObject = new GraphObject;

    GraphData &rGraphData = graphObject->getData();

    rGraphData.setTitle(this->monitoringPointsCombo->currentText());
    rGraphData.setTypeMask(GRAPH_LINES);
    rGraphData.setXLabel(tr("Iteration"));
    rGraphData.setYLabel(tr("Value"));

    int index = this->monitoringPointsCombo->currentIndex();
    if (index >= 0)
    {
        QList<RMonitoringPoint> keys = this->data.keys();
        QVector<RRVector> &values = this->data[keys[index]];
        for (int i=0;i<values.size();i++)
        {
            RRVector &rValues = this->data[keys[index]][i];
            rGraphData[i].resize(rValues.size());
            for (uint j=0;j<rValues.size();j++)
            {
                rGraphData[i][j] = this->data[keys[index]][i][j];
            }
        }
    }

    GraphDialog graphDialog(graphObject,this);
    graphDialog.getGraphWidget()->setShowPointer(true,true,true);
    graphDialog.exec();
}

void MonitoringPointGraphDialog::onCurrentIndexChanged(int)
{
    this->populateValuesTree();
}

void MonitoringPointGraphDialog::onDisplayInGraphClicked()
{
    this->displayInGraph();
}
