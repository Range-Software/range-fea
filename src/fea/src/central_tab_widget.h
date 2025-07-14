#ifndef CENTRAL_TAB_WIDGET_H
#define CENTRAL_TAB_WIDGET_H

#include <QTabWidget>
#include <QMdiArea>

#include <rbl_message.h>

#include <rgl_text_browser.h>

#include "pick_details_tree.h"

class CentralTabWidget : public QTabWidget
{

    Q_OBJECT

    protected:

        static const QString informationIconFile;
        static const QString importantIconFile;
        static const QString severeIconFile;

        //! Model tab position;
        int modelTabPosition;
        //! Application output tab position;
        int applicationOutputTabPosition;
        //! Process output tab position;
        int processOutputTabPosition;
        //! Pick details tab position;
        int pickDetailsTabPosition;

        //! Application output last icon.
        QString applicationOutputLastIcon;
        //! Process output last icon.
        QString processOutputLastIcon;

        //! MDI Area.
        QMdiArea *mdiArea;
        //! Application output browser.
        RTextBrowser *applicationOutputBrowser;
        //! Process output browser.
        RTextBrowser *processOutputBrowser;
        //! Pick Details tree.
        PickDetailsTree *pickDetailsTree;

    public:

        enum Type
        {
            Model = 0,
            ApplicationOutput,
            ProcessOutput,
            PickDetails
        };

    public:

        //! Constructor.
        explicit CentralTabWidget(QWidget *parent = nullptr);

        //! return MDI Area.
        QMdiArea *getMdiArea();

    private:

        //! Set text and icon for given tab.
        void setTabTitle(CentralTabWidget::Type tabType, RMessage::Type messageType = RMessage::Type::None, const QString &additionalText = QString());

    private slots:

        //! Print application info message.
        void onInfoPrinted(const QString &message);

        //! Print application notice message.
        void onNoticePrinted(const QString &message);

        //! Print application warning message.
        void onWarningPrinted(const QString &message);

        //! Print application error message.
        void onErrorPrinted(const QString &message);

        //! Print process standard output message.
        void onProcessReadyStandardOutput(const QString &message);

        //! Print process standard error message.
        void onProcessReadyStandardError(const QString &message);

        //! Central tab widget current changed.
        void onCurrentChanged(int tabPosition);

        //! Pick list has changed.
        void onPickLostChanged();

        //! Solver job has started.
        void onSolverJobStarted();

        //! Solver job has finished.
        void onSolverJobFinished();

        //! Solver job has failed.
        void onSolverJobFailed();

};

#endif // CENTRAL_TAB_WIDGET_H
