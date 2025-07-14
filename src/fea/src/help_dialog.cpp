#include "help_dialog.h"
#include "help_gl_action_event_widget.h"

HelpDialog::HelpDialog(const QString &title, const QMap<QString, QString> &topics, QWidget *parent)
    : RHelpDialog(title,topics,parent)
{
    HelpGLActionEventWidget *helpActionsWidget = new HelpGLActionEventWidget;
    this->tabWidget->addTab(helpActionsWidget,tr("3D area"));
}
