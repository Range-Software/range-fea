#ifndef HELP_GL_ACTION_EVENT_WIDGET_H
#define HELP_GL_ACTION_EVENT_WIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QTextBrowser>

class HelpGLActionEventWidget : public QWidget
{

    Q_OBJECT

    protected:

        //! List widget.
        QListWidget *listWidget;
        //! Text browser.
        QTextBrowser *textBrowser;

    public:

        //! Constructor
        explicit HelpGLActionEventWidget(QWidget *parent = nullptr);

    private slots:

        //! List selection changed
        void onListSelectionChanged();

};

#endif // HELP_GL_ACTION_EVENT_WIDGET_H
