#include <QVBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QDialogButtonBox>

#include "graph_dialog.h"

GraphDialog::GraphDialog(GraphObject *graphObject, QWidget *parent) :
    QDialog(parent)
{
    QIcon closeIcon(":/icons/action/pixmaps/range-close.svg");

    this->setWindowTitle(tr("Graph"));
    this->resize(600,400);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    this->graphWidget = new GraphWidget(graphObject,true,this);
    mainLayout->addWidget(this->graphWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *closeButton = new QPushButton(closeIcon, tr("Close"));
    buttonBox->addButton(closeButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

GraphWidget *GraphDialog::getGraphWidget()
{
    return this->graphWidget;
}
