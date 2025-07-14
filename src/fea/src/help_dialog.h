#ifndef HELP_DIALOG_H
#define HELP_DIALOG_H

#include <rgl_help_dialog.h>

class HelpDialog : public RHelpDialog
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit HelpDialog(const QString &title, const QMap<QString, QString> &topics, QWidget *parent = nullptr);

};

#endif // HELP_DIALOG_H
