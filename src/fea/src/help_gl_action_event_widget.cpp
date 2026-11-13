#include <QSplitter>
#include <QVBoxLayout>
#include <QDir>
#include <QFile>
#include <QTextBlockFormat>
#include <QTextCursor>

#include "gl_acion_event.h"
#include "help_gl_action_event_widget.h"

HelpGLActionEventWidget::HelpGLActionEventWidget(const QString &searchPath, QWidget *parent)
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

#ifndef Q_OS_MACOS
    Q_UNUSED(searchPath);
#endif

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

#ifdef Q_OS_MACOS
        QString helpFile;
        switch (GLActionEvent::Type(glActionEventType))
        {
            case GLActionEvent::Translate:   helpFile = "macos_translate.md"; break;
            case GLActionEvent::Rotate:      helpFile = "macos_rotate.md"; break;
            case GLActionEvent::Zoom:        helpFile = "macos_zoom.md"; break;
            case GLActionEvent::PickElement: helpFile = "macos_pick-element.md"; break;
            case GLActionEvent::PickNode:    helpFile = "macos_pick-node.md"; break;
            case GLActionEvent::PickClear:   helpFile = "macos_pick-clear.md"; break;
            default: break;
        }
        if (!helpFile.isEmpty())
        {
            QFile file(QDir(searchPath).filePath(helpFile));
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                item->setData(Qt::UserRole + 1,QString::fromUtf8(file.readAll()));
            }
        }
#endif
    }

    this->listWidget->setCurrentRow(0);
}

void HelpGLActionEventWidget::onListSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems = this->listWidget->selectedItems();

    this->textBrowser->clear();
    if (selectedItems.size() != 0)
    {
        const QListWidgetItem *item = selectedItems.at(0);
        this->textBrowser->setHtml(item->data(Qt::UserRole).toString());
        const QString markdown = item->data(Qt::UserRole + 1).toString();
        if (!markdown.isEmpty())
        {
            QTextCursor cursor = this->textBrowser->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertBlock(QTextBlockFormat());
            cursor.insertMarkdown(markdown);
        }
    }
}
