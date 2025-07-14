#include <QGridLayout>
#include <QStackedLayout>
#include <QPushButton>
#include <QMessageBox>

#include "draw_input_widget.h"
#include "application.h"

DrawInputWidget::DrawInputWidget(QWidget *parent) :
    QWidget(parent)
{
    this->stackedLayout = new QStackedLayout;
    this->setLayout(this->stackedLayout);

    this->positionWidget = new PositionWidget(tr("Position"),RR3Vector());
    this->positionWidget->hideSliders();
    this->positionWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    this->stackedLayout->addWidget(this->positionWidget);

    QObject::connect(this->positionWidget,
                     &PositionWidget::changed,
                     this,
                     &DrawInputWidget::onPositionWidgetChanged);
    QObject::connect(this->positionWidget,
                     &PositionWidget::closed,
                     this,
                     &DrawInputWidget::onPositionWidgetClosed);

    this->localDirectionWidget = new LocalDirectionWidget(tr("Local direction"),RLocalDirection());
    this->localDirectionWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    this->stackedLayout->addWidget(this->localDirectionWidget);

    QObject::connect(this->localDirectionWidget,
                     &LocalDirectionWidget::changed,
                     this,
                     &DrawInputWidget::onLocalDirectionWidgetChanged);
    QObject::connect(this->localDirectionWidget,
                     &LocalDirectionWidget::closed,
                     this,
                     &DrawInputWidget::onLocalDirectionWidgetClosed);

    this->textWidget = new TextEditWidget;
    this->textWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    this->stackedLayout->addWidget(this->textWidget);

    QObject::connect(this->textWidget,
                     &TextEditWidget::changed,
                     this,
                     &DrawInputWidget::onTextWidgetChanged);
    QObject::connect(this->textWidget,
                     &TextEditWidget::closed,
                     this,
                     &DrawInputWidget::onPositionWidgetClosed);

    this->layoutWidget = new QWidget;
    this->stackedLayout->addWidget(this->layoutWidget);
    this->stackedLayout->setCurrentWidget(this->layoutWidget);

    QGridLayout *layout = new QGridLayout;
    this->layoutWidget->setLayout(layout);

    this->tree = new DrawInputTree;
    layout->addWidget(this->tree,0,0,1,2);

    QObject::connect(this->tree,
                     &DrawInputTree::objectSelectionChanged,
                     this,
                     &DrawInputWidget::onSelectionChanged);
    QObject::connect(this->tree,
                     &DrawInputTree::positionRequest,
                     this,
                     &DrawInputWidget::onPositionRequest);
    QObject::connect(this->tree,
                     &DrawInputTree::localDirectionRequest,
                     this,
                     &DrawInputWidget::onLocalDirectionRequest);
    QObject::connect(this->tree,
                     &DrawInputTree::textRequest,
                     this,
                     &DrawInputWidget::onTextRequest);
    QObject::connect(this->tree,
                     &DrawInputTree::inputParameterChanged,
                     this,
                     &DrawInputWidget::onInputParameterChanged);

    this->mergeNodesCheck = new QCheckBox(tr("Merge near nodes"));
    this->mergeNodesCheck->setCheckState(Qt::Checked);
    layout->addWidget(this->mergeNodesCheck,1,0,1,2);

    QIcon removeIcon(":/icons/file/pixmaps/range-remove.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");

    this->removeButton = new QPushButton(removeIcon,tr("Remove"));
    this->removeButton->setDisabled(true);
    layout->addWidget(this->removeButton,2,0,1,1);

    QObject::connect(this->removeButton,
                     &QPushButton::clicked,
                     this,
                     &DrawInputWidget::onRemoveClicked);

    this->okButton = new QPushButton(okIcon,tr("Ok"));
    layout->addWidget(this->okButton,2,1,1,1);

    QObject::connect(this->okButton,
                     &QPushButton::clicked,
                     this,
                     &DrawInputWidget::onOkClicked);

    QPushButton *cancelButton = new QPushButton(cancelIcon,tr("Cancel"));
    layout->addWidget(cancelButton,3,0,1,2);

    QObject::connect(cancelButton,
                     &QPushButton::clicked,
                     this,
                     &DrawInputWidget::onCancelClicked);
}

void DrawInputWidget::onPositionRequest(const RR3Vector &position)
{
    this->positionWidget->setPosition(position);
    this->stackedLayout->setCurrentWidget(this->positionWidget);
}

void DrawInputWidget::onPositionWidgetChanged(const RR3Vector &position)
{
    this->tree->setRequestedItemVectorValue(position);
}

void DrawInputWidget::onPositionWidgetClosed()
{
    this->stackedLayout->setCurrentWidget(this->layoutWidget);
}

void DrawInputWidget::onLocalDirectionRequest(const RLocalDirection &localDirection)
{
    this->localDirectionWidget->setLocalDirection(localDirection);
    this->stackedLayout->setCurrentWidget(this->localDirectionWidget);
}

void DrawInputWidget::onLocalDirectionWidgetChanged(const RLocalDirection &localDirection)
{
    this->tree->setRequestedItemLocalDirectionValue(localDirection);
}

void DrawInputWidget::onLocalDirectionWidgetClosed()
{
    this->stackedLayout->setCurrentWidget(this->layoutWidget);
}

void DrawInputWidget::onTextRequest(const QString &text)
{
    this->textWidget->setText(text);
    this->stackedLayout->setCurrentWidget(this->textWidget);
}

void DrawInputWidget::onTextWidgetChanged(const QString &text)
{
    this->tree->setRequestedItemTextValue(text);
}

void DrawInputWidget::onTextWidgetClosed()
{
    this->stackedLayout->setCurrentWidget(this->layoutWidget);
}

void DrawInputWidget::onSelectionChanged(const QList<uint> &objectIDs)
{
    this->removeButton->setEnabled(objectIDs.size() > 0);
}

void DrawInputWidget::onInputParameterChanged(uint objectID, uint)
{
    Application::instance()->getSession()->getDrawEngine()->getObject(objectID)->updateModel();
}

void DrawInputWidget::onRemoveClicked()
{
    QList<uint> objectIDs = this->tree->getSelectedObjectIDs();

    for (int i=objectIDs.size()-1;i>=0;--i)
    {
        Application::instance()->getSession()->getDrawEngine()->removeObject(objectIDs[i]);
    }
}

void DrawInputWidget::onOkClicked()
{
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    if (modelIDs.size() == 0)
    {
        QMessageBox::warning(this,tr("No model is selected"),tr("To draw an object at least one model must be selected."),QMessageBox::Ok);
        return;
    }

    QString message = "Prepared objects will be added to following (selected) models:<ul>";
    for (int i=0;i<modelIDs.size();i++)
    {
        message += "<li>" + Application::instance()->getSession()->getModel(modelIDs[i]).getName() + "</li>";
    }
    message += "</ul>";

    int response = QMessageBox::question(this,
                                         "Draw objects(s)?",
                                         message,
                                         QMessageBox::Ok | QMessageBox::Cancel,
                                         QMessageBox::Cancel);
    if (response == QMessageBox::Cancel)
    {
        return;
    }

    this->tree->releaseRequestedItem();
    Application::instance()->getSession()->getDrawEngine()->processObjects(modelIDs,
                                                           this->mergeNodesCheck->checkState() != Qt::Unchecked,
                                                           RConstants::eps,
                                                           true);
}

void DrawInputWidget::onCancelClicked()
{
    uint nObjects = Application::instance()->getSession()->getDrawEngine()->getNObjects();
    do
    {
        Application::instance()->getSession()->getDrawEngine()->removeObject(--nObjects);
    } while (nObjects > 0);
}
