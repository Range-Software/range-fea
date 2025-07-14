#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "application.h"
#include "stream_line_dialog.h"

StreamLineDialog::StreamLineDialog(uint modelID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityID(RConstants::eod)
{
    this->createDialog();
}

StreamLineDialog::StreamLineDialog(uint modelID, uint entityID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityID(entityID)
{
    this->createDialog();
}

int StreamLineDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        RVariableType varType = this->getVariableType();

        if (varType != R_VARIABLE_NONE)
        {
            Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Create stream line"));

            if (this->entityID != RConstants::eod)
            {
                RStreamLine &streamLine = Application::instance()->getSession()->getModel(this->modelID).getStreamLine(this->entityID);

                streamLine.setVariableType(varType);
                streamLine.setPosition(this->positionWidget->getPosition());

                RLogger::info("Modified stream line \'%s\'\n",streamLine.getName().toUtf8().constData());
            }
            else
            {
                RStreamLine streamLine;

                streamLine.setName(RVariable::getName(varType));
                streamLine.setVariableType(varType);
                streamLine.setPosition(this->positionWidget->getPosition());

                Application::instance()->getSession()->getModel(this->modelID).addStreamLine(streamLine);
                RLogger::info("Created new stream line \'%s\'\n",streamLine.getName().toUtf8().constData());
            }

            Application::instance()->getSession()->setModelChanged(this->modelID);
        }
    }

    Application::instance()->getSession()->setEndDrawStreamLinePosition();

    return retVal;
}

void StreamLineDialog::createDialog()
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QString windowTitleStr = tr("Stream line editor");
    this->setWindowTitle(windowTitleStr);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QLabel *titleLabel = new QLabel;
    if (this->entityID == RConstants::eod)
    {
        titleLabel->setText(tr("Create new stream line."));
    }
    else
    {
        titleLabel->setText(tr("Modify stream line") + ": <b>" + Application::instance()->getSession()->getModel(modelID).getStreamLine(entityID).getName() + "</b>");
    }
    mainLayout->addWidget(titleLabel);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    mainLayout->addLayout(hBoxLayout);

    this->variableList = new QListWidget(this);
    this->variableList->setSelectionMode(QAbstractItemView::SingleSelection);

    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);
    RR3Vector startPosition;

    for (uint i=0;i<rModel.getNVariables();i++)
    {
        RVariable &rVariable = rModel.getVariable(i);
        if (rVariable.getNVectors() > 1)
        {
            QListWidgetItem *itemVariable = new QListWidgetItem(this->variableList);
            itemVariable->setText(rVariable.getName());
            itemVariable->setData(Qt::UserRole,QVariant(rVariable.getType()));
            if (this->entityID != RConstants::eod)
            {
                RStreamLine &rStreamLine = Application::instance()->getSession()->getModel(this->modelID).getStreamLine(this->entityID);
                itemVariable->setSelected(rVariable.getType() == rStreamLine.getVariableType());
                startPosition = rStreamLine.getPosition();
            }
        }
    }

    hBoxLayout->addWidget(this->variableList);

    QObject::connect(this->variableList,
                     &QListWidget::itemSelectionChanged,
                     this,
                     &StreamLineDialog::onVariableListSelectionChanged);

    double xMin = 0.0;
    double xMax = 0.0;
    double yMin = 0.0;
    double yMax = 0.0;
    double zMin = 0.0;
    double zMax = 0.0;

    rModel.findNodeLimits(xMin,xMax,yMin,yMax,zMin,zMax);

    this->positionWidget = new PositionWidget("Stream line position",startPosition);
    this->positionWidget->hideButtons();
    this->positionWidget->setXRange(xMin,xMax);
    this->positionWidget->setYRange(yMin,yMax);
    this->positionWidget->setZRange(zMin,zMax);
    hBoxLayout->addWidget(this->positionWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setEnabled(this->variableList->selectedItems().size());
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->positionWidget,&PositionWidget::changed,this,&StreamLineDialog::onPositionChanged);

    Application::instance()->getSession()->setBeginDrawStreamLinePosition(startPosition);
}

RVariableType StreamLineDialog::getVariableType() const
{
    QList<QListWidgetItem*> items = this->variableList->selectedItems();

    if (items.size() == 0)
    {
        return R_VARIABLE_NONE;
    }

    return RVariableType(items[0]->data(Qt::UserRole).toInt());
}

void StreamLineDialog::onPositionChanged(const RR3Vector &position)
{
    Application::instance()->getSession()->setBeginDrawStreamLinePosition(position);
}

void StreamLineDialog::onVariableListSelectionChanged()
{
    this->okButton->setEnabled(this->variableList->selectedItems().size());
}
