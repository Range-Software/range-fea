#ifndef CENTRAL_TAB_WIDGET_H
#define CENTRAL_TAB_WIDGET_H

#include <QTabWidget>
#include <QMdiArea>
#include <QMenu>
#include <QToolButton>

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

        //! Minimum value of maximum number of lines kept in process output.
        static const int processOutputMinMaxLines;
        //! Maximum value of maximum number of lines kept in process output.
        static const int processOutputMaxMaxLines;

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
        //! Tile windows button.
        QToolButton *tileWindowsButton;
        //! Windows can be tiled.
        bool tileWindowsAvailable;
        //! Windows menu button.
        QToolButton *windowsButton;
        //! There are multiple windows to choose from.
        bool windowListAvailable;

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

    public slots:

        //! Set whether windows can be tiled.
        void setTileWindowsAvailable(bool available);

        //! Set whether there are multiple windows to choose from.
        void setWindowListAvailable(bool available);

    signals:

        //! Tile windows button was clicked.
        void tileWindowsClicked();

        //! Windows menu is about to be shown.
        void windowsMenuAboutToShow(QMenu *menu);

    protected:

        //! Change event.
        void changeEvent(QEvent *event) override;

    private:

        //! Show window buttons when they can be used and model tab is displayed.
        void updateWindowButtons();

        //! Set text and icon for given tab.
        void setTabTitle(CentralTabWidget::Type tabType, RMessage::Type messageType = RMessage::Type::None, const QString &additionalText = QString());

    private slots:

        //! Print application info message.
        void onInfoPrinted(const RMessage &message);

        //! Print application notice message.
        void onNoticePrinted(const RMessage &message);

        //! Print application warning message.
        void onWarningPrinted(const RMessage &message);

        //! Print application error message.
        void onErrorPrinted(const RMessage &message);

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
