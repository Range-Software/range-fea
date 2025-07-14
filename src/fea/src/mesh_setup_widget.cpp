#include <QVBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>

#include "mesh_setup_widget.h"
#include "application.h"

MeshSetupWidget::MeshSetupWidget(const RMeshSetup &meshSetup,
                                 const std::vector<RVariableType> &variableTypes,
                                 QWidget *parent)
    : QWidget(parent)
    , meshSetup(meshSetup)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QGroupBox *groupBox = new QGroupBox(tr("Mesh refinement setup"));
    mainLayout->addWidget(groupBox);

    QGridLayout *groupLayout = new QGridLayout;
    groupBox->setLayout(groupLayout);

    int groupLayoutRow = 0;

    QListWidget *variablesListWidget = new QListWidget();
    variablesListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    groupLayout->addWidget(variablesListWidget,groupLayoutRow,0,1,3);

    for (uint i=0;i<variableTypes.size();i++)
    {
        QListWidgetItem *item = new QListWidgetItem(variablesListWidget);
        item->setText(RVariable::getName(variableTypes[i]));
        item->setData(Qt::UserRole,QVariant(variableTypes[i]));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(meshSetup.getVariables().contains(variableTypes[i]) ? Qt::Checked : Qt::Unchecked);
    }

    QObject::connect(variablesListWidget,&QListWidget::itemChanged,this,&MeshSetupWidget::onVariableListItemChanged);

    groupLayoutRow++;

    QLabel *minEdgeLengthLabel = new QLabel(tr("Minimum edge length"));
    groupLayout->addWidget(minEdgeLengthLabel,groupLayoutRow,0,1,1);

    this->minEdgeLengthEdit = new ValueLineEdit(0.0,1e10);
    this->minEdgeLengthEdit->setValue(this->meshSetup.getMinEdgeLength());
    this->minEdgeLengthEdit->setEnabled(this->meshSetup.getVariables().size() > 0);
    groupLayout->addWidget(this->minEdgeLengthEdit,groupLayoutRow,1,1,1);

    QObject::connect(this->minEdgeLengthEdit,&ValueLineEdit::valueChanged,this,&MeshSetupWidget::onMinEdgeLengthChanged);

    QLabel *minEdgeLengthUnitLabel = new QLabel("[m]");
    groupLayout->addWidget(minEdgeLengthUnitLabel,groupLayoutRow,2,1,1);

    groupLayoutRow++;

    QLabel *maxEdgeLengthLabel = new QLabel(tr("Maximum edge length"));
    groupLayout->addWidget(maxEdgeLengthLabel,groupLayoutRow,0,1,1);

    this->maxEdgeLengthEdit = new ValueLineEdit(0.0,1e10);
    this->maxEdgeLengthEdit->setValue(this->meshSetup.getMaxEdgeLength());
    this->maxEdgeLengthEdit->setEnabled(this->meshSetup.getVariables().size() > 0);
    groupLayout->addWidget(this->maxEdgeLengthEdit,groupLayoutRow,1,1,1);

    QObject::connect(this->maxEdgeLengthEdit,&ValueLineEdit::valueChanged,this,&MeshSetupWidget::onMaxEdgeLengthChanged);

    QLabel *maxEdgeLengthUnitLabel = new QLabel("[m]");
    groupLayout->addWidget(maxEdgeLengthUnitLabel,groupLayoutRow,2,1,1);

}

void MeshSetupWidget::onVariableListItemChanged(QListWidgetItem *item)
{
    RVariableType variableType = RVariableType(item->data(Qt::UserRole).toInt());
    if (item->checkState() == Qt::Checked)
    {
        this->meshSetup.getVariables().insert(variableType);
    }
    else
    {
        QSet<RVariableType>::iterator iter = this->meshSetup.getVariables().find(variableType);
        if (iter != this->meshSetup.getVariables().end())
        {
            this->meshSetup.getVariables().erase(iter);
        }
    }

    this->minEdgeLengthEdit->setEnabled(this->meshSetup.getVariables().size() > 0);
    this->maxEdgeLengthEdit->setEnabled(this->meshSetup.getVariables().size() > 0);

    emit this->changed(this->meshSetup);
}

void MeshSetupWidget::onMinEdgeLengthChanged(double value)
{
    this->meshSetup.setMinEdgeLength(value);
    this->maxEdgeLengthEdit->setRange(value,this->maxEdgeLengthEdit->getMaximum());
    emit this->changed(this->meshSetup);
}

void MeshSetupWidget::onMaxEdgeLengthChanged(double value)
{
    this->meshSetup.setMaxEdgeLength(value);
    this->minEdgeLengthEdit->setRange(this->minEdgeLengthEdit->getMinimum(),value);
    emit this->changed(this->meshSetup);
}
