#include <QVBoxLayout>
#include <QScrollArea>
#include <QDialogButtonBox>

#include "move_node_dialog.h"
#include "position_widget.h"
#include "application.h"

MoveNodeDialog::MoveNodeDialog(uint modelID, const QList<uint> &nodeIDs, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    nodeIDs(nodeIDs)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Move node"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);

    QWidget *containerWidget = new QWidget;
    scrollArea->setWidget(containerWidget);

    QVBoxLayout *containerLayout = new QVBoxLayout(containerWidget);

    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    for (int i=0;i<nodeIDs.size();i++)
    {
        QString labelStr = tr("Node") + QString(" # ") + QLocale().toString(this->nodeIDs[i]);
        PositionWidget *positionWidget = new PositionWidget(labelStr,rModel.getNode(this->nodeIDs[i]).toVector(),true);
        positionWidget->hidePickButton();
        containerLayout->addWidget(positionWidget);

        QObject::connect(positionWidget,&PositionWidget::changed,this,&MoveNodeDialog::onPositionChanged);

        this->positionWidgets.append(positionWidget);
    }

    containerLayout->addStretch(1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setEnabled(false);
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    Application::instance()->getSession()->setBeginDrawMoveNodes(this->findNodeIdPositionMap());
}

int MoveNodeDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Move node"));
        Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

        for (int i=0;i<this->nodeIDs.size();i++)
        {
            RR3Vector position(this->positionWidgets[i]->getPosition());

            rModel.getNode(this->nodeIDs[i]).set(position[0],position[1],position[2]);
        }
    }

    Application::instance()->getSession()->setEndDrawMoveNodes();
    Application::instance()->getSession()->setModelChanged(this->modelID);

    return retVal;
}

QMap<SessionNodeID, RR3Vector> MoveNodeDialog::findNodeIdPositionMap() const
{
    QMap<SessionNodeID, RR3Vector> nodeIDMap;

    for (int i=0;i<this->nodeIDs.size();i++)
    {
        nodeIDMap.insert(SessionNodeID(this->modelID,this->nodeIDs[i]),this->positionWidgets[i]->getPosition());
    }

    return nodeIDMap;
}

void MoveNodeDialog::onPositionChanged(const RR3Vector &)
{
    this->okButton->setEnabled(true);
    Application::instance()->getSession()->setBeginDrawMoveNodes(this->findNodeIdPositionMap());
}
