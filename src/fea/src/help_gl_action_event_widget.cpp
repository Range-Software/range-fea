#include <QSplitter>
#include <QVBoxLayout>

#include "gl_acion_event.h"
#include "help_gl_action_event_widget.h"

HelpGLActionEventWidget::HelpGLActionEventWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Horizontal);
    mainLayout->addWidget(splitter);

    this->listWidget = new QListWidget;
    this->listWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    splitter->addWidget(this->listWidget);

    QObject::connect(this->listWidget,&QListWidget::itemSelectionChanged,this,&HelpGLActionEventWidget::onListSelectionChanged);

    this->textBrowser = new QTextBrowser;
    this->textBrowser->setReadOnly(true);
    this->textBrowser->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
    splitter->addWidget(this->textBrowser);

    splitter->setStretchFactor(1,1);

    for (uint glActionEventType=GLActionEvent::None+1;glActionEventType<GLActionEvent::nTypes;glActionEventType++)
    {
        QStringList keyMouseCombinationList = GLActionEvent::findKeyMouseCombinations(GLActionEvent::Type(glActionEventType));

        QListWidgetItem *item = new QListWidgetItem(this->listWidget);
        item->setText(GLActionEvent::toString(GLActionEvent::Type(glActionEventType)));
        QString keyMouseCombinationText = "<ul>";
        foreach (const QString &keyMouseCombination,keyMouseCombinationList)
        {
            keyMouseCombinationText += "<li>" + keyMouseCombination + "</li>";
        }
        keyMouseCombinationText += "</ul>";
        item->setData(Qt::UserRole,keyMouseCombinationText);
    }
}

void HelpGLActionEventWidget::onListSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems = this->listWidget->selectedItems();

    this->textBrowser->clear();
    if (selectedItems.size() != 0)
    {
        this->textBrowser->setHtml(selectedItems.at(0)->data(Qt::UserRole).toString());
    }
}
