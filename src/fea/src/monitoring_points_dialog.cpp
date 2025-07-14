#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include "monitoring_points_dialog.h"
#include "application.h"
#include "variable_selector.h"

MonitoringPointsDialog::MonitoringPointsDialog(uint modelID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID)
{
    const RMonitoringPointManager &monitoringPointsManager = Application::instance()->getSession()->getModel(this->modelID).getMonitoringPointManager();

    this->setWindowTitle(tr("Define monitoring points"));
    this->resize(600,300);

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    this->tableWidget = new QTableWidget(0,4);
    this->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Variable") << "X" << "Y" << "Z");
    for (uint i=0;i<monitoringPointsManager.size();i++)
    {
        this->addMonitoringPoint(monitoringPointsManager[i]);
    }
    mainLayout->addWidget(this->tableWidget);

    QHBoxLayout *actionButtonsLayout = new QHBoxLayout;
    mainLayout->addLayout(actionButtonsLayout);

    QPushButton *addButton = new QPushButton(tr("Add"));
    actionButtonsLayout->addWidget(addButton);

    this->removeButton = new QPushButton(tr("Remove"));
    this->removeButton->setEnabled(this->tableWidget->selectedItems().size() > 0);
    actionButtonsLayout->addWidget(this->removeButton);

    this->usePickedButton = new QPushButton(tr("Set coordinates from picked element/node"));
    this->usePickedButton->setEnabled(!Application::instance()->getSession()->getPickList().isEmpty() && this->tableWidget->selectedItems().size() > 0);
    mainLayout->addWidget(this->usePickedButton);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->tableWidget,&QTableWidget::itemSelectionChanged,this,&MonitoringPointsDialog::onItemSelectionChanged);
    QObject::connect(addButton,&QPushButton::clicked,this,&MonitoringPointsDialog::onAddClicked);
    QObject::connect(this->removeButton,&QPushButton::clicked,this,&MonitoringPointsDialog::onRemoveClicked);
    QObject::connect(this->usePickedButton,&QPushButton::clicked,this,&MonitoringPointsDialog::onUsePickedClicked);
}

int MonitoringPointsDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Set monitoring points"));

        RMonitoringPointManager &monitoringPointsManager = Application::instance()->getSession()->getModel(this->modelID).getMonitoringPointManager();
        monitoringPointsManager.clear();

        for (int i=0;i<this->tableWidget->rowCount();i++)
        {
            VariableSelector *variableSelector = dynamic_cast<VariableSelector*>(this->tableWidget->cellWidget(i,0));
            QTableWidgetItem *xItem = this->tableWidget->item(i,1);
            QTableWidgetItem *yItem = this->tableWidget->item(i,2);
            QTableWidgetItem *zItem = this->tableWidget->item(i,3);

            RVariableType variableType = variableSelector->getCurrentVariableType();
            double x = QLocale().toDouble(xItem->text());
            double y = QLocale().toDouble(yItem->text());
            double z = QLocale().toDouble(zItem->text());

            monitoringPointsManager.push_back(RMonitoringPoint(variableType,RR3Vector(x,y,z)));
        }
        Application::instance()->getSession()->setModelChanged(this->modelID);
    }

    return retVal;
}

void MonitoringPointsDialog::addMonitoringPoint(const RMonitoringPoint &monitoringPoint)
{
    int row = this->tableWidget->rowCount();

    this->tableWidget->insertRow(row);

    VariableSelector *variableSelector = new VariableSelector(RProblem::getVariableTypes(Application::instance()->getSession()->getModel(this->modelID).getProblemTaskTree().getProblemTypeMask()));
    variableSelector->setCurrentVariableType(monitoringPoint.getVariableType());
    this->tableWidget->setCellWidget(row,0,variableSelector);

    QTableWidgetItem *xItem = new QTableWidgetItem(QLocale().toString(monitoringPoint.getPosition()[0]));
    this->tableWidget->setItem(row,1,xItem);

    QTableWidgetItem *yItem = new QTableWidgetItem(QLocale().toString(monitoringPoint.getPosition()[1]));
    this->tableWidget->setItem(row,2,yItem);

    QTableWidgetItem *zItem = new QTableWidgetItem(QLocale().toString(monitoringPoint.getPosition()[2]));
    this->tableWidget->setItem(row,3,zItem);
}

QList<int> MonitoringPointsDialog::findSelectedRows() const
{
    QList<QTableWidgetItem*> selectedItems = this->tableWidget->selectedItems();
    QMap<int,bool> rowBook;

    for (int i=0;i<selectedItems.size();i++)
    {
        rowBook[selectedItems.at(i)->row()] = true;
    }

    QList<int> rowList = rowBook.keys();
    std::sort(rowList.begin(),rowList.end());

    return rowList;
}

RR3Vector MonitoringPointsDialog::findPickedPosition() const
{
    const PickList &pickList = Application::instance()->getSession()->getPickList();

    const QVector<PickItem> &pickItems = pickList.getItems();

    for (int i=0;i<pickItems.size();i++)
    {
        const SessionEntityID &pickEntityID = pickItems[i].getEntityID();

        if (pickEntityID.getMid() != this->modelID)
        {
            continue;
        }

        if (pickItems[i].getItemType() == PICK_ITEM_NODE)
        {
            return Application::instance()->getSession()->getModel(pickEntityID.getMid()).getNode(pickItems[i].getNodeID()).toVector();
        }
        else if (pickItems[i].getItemType() == PICK_ITEM_ELEMENT)
        {
            double cx, cy, cz;
            const Model &rModel = Application::instance()->getSession()->getModel(pickEntityID.getMid());
            rModel.getElement(pickItems[i].getElementID()).findCenter(rModel.getNodes(),cx,cy,cz);
            return RR3Vector(cx, cy, cz);
        }
    }

    return RR3Vector();
}

void MonitoringPointsDialog::onItemSelectionChanged()
{
    this->removeButton->setEnabled(this->tableWidget->selectedItems().size() > 0);
    this->usePickedButton->setEnabled(this->tableWidget->selectedItems().size() > 0);
}

void MonitoringPointsDialog::onAddClicked()
{
    this->addMonitoringPoint(RMonitoringPoint());
}

void MonitoringPointsDialog::onRemoveClicked()
{
    QList<int> selectedRows = this->findSelectedRows();
    for(int i=selectedRows.count()-1;i>=0;i--)
    {
        this->tableWidget->removeRow(selectedRows.at(i));
    }
}

void MonitoringPointsDialog::onUsePickedClicked()
{
    RR3Vector position = this->findPickedPosition();

    QList<int> selectedRows = this->findSelectedRows();
    for (int i=0;i<selectedRows.size();i++)
    {
        this->tableWidget->item(selectedRows.at(i),1)->setText(QLocale().toString(position[0]));
        this->tableWidget->item(selectedRows.at(i),2)->setText(QLocale().toString(position[1]));
        this->tableWidget->item(selectedRows.at(i),3)->setText(QLocale().toString(position[2]));
    }
}
