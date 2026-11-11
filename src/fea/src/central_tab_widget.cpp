#include <algorithm>


#include <QApplication>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSpinBox>

#include <rgl_logger_handler.h>
#include <rgl_message_box.h>

#include "central_tab_widget.h"
#include "application.h"
#include "solver_manager.h"
#include "session.h"
#include "pick_list.h"
#include <rgl_icon.h>

const QString CentralTabWidget::informationIconFile(":/icons/file/pixmaps/range-information.svg");
const QString CentralTabWidget::importantIconFile(":/icons/file/pixmaps/range-important.svg");
const QString CentralTabWidget::severeIconFile(":/icons/file/pixmaps/range-severe.svg");

const int CentralTabWidget::processOutputMinMaxLines = 100;
const int CentralTabWidget::processOutputMaxMaxLines = 10000000;

const bool useTabIcons = false;

//! Append text at the end of the browser. Colors are given only to highlight
//! text, otherwise the text follows the palette, also after the color scheme
//! changes.
static void appendText(QTextEdit *browser, const QString &text, const QColor &foreground = QColor(), const QColor &background = QColor())
{
    QTextCharFormat charFormat;
    if (foreground.isValid())
    {
        charFormat.setForeground(foreground);
    }
    if (background.isValid())
    {
        charFormat.setBackground(background);
    }
    QTextCursor cursor(browser->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text,charFormat);
    browser->moveCursor(QTextCursor::End);
}

CentralTabWidget::CentralTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    this->setTabPosition(QTabWidget::South);

    this->mdiArea = new QMdiArea(this);
    this->mdiArea->setObjectName(QString::fromUtf8("mdiArea"));
    this->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->mdiArea->setBackground(this->palette().brush(QPalette::Active,QPalette::Window));

    this->applicationOutputBrowser = new RTextBrowser(true);
    this->processOutputBrowser = new RTextBrowser(true);

    // Oldest lines are removed first once the process output exceeds the maximum number of lines.
    int processOutputMaxLines = int(Application::instance()->getApplicationSettings()->getProcessOutputMaxLines());
    processOutputMaxLines = std::clamp(processOutputMaxLines,CentralTabWidget::processOutputMinMaxLines,CentralTabWidget::processOutputMaxMaxLines);
    this->processOutputBrowser->document()->setMaximumBlockCount(processOutputMaxLines);

    QSpinBox *processOutputMaxLinesSpin = new QSpinBox;
    processOutputMaxLinesSpin->setRange(CentralTabWidget::processOutputMinMaxLines,CentralTabWidget::processOutputMaxMaxLines);
    processOutputMaxLinesSpin->setSingleStep(1000);
    processOutputMaxLinesSpin->setValue(processOutputMaxLines);
    processOutputMaxLinesSpin->setPrefix(tr("Max lines") + ": ");
    processOutputMaxLinesSpin->setKeyboardTracking(false);
    processOutputMaxLinesSpin->setToolTip(tr("Maximum number of lines kept in process output. Oldest lines are removed first."));
    this->processOutputBrowser->addCornerWidget(processOutputMaxLinesSpin);

    QObject::connect(processOutputMaxLinesSpin,&QSpinBox::valueChanged,this,[this](int value){
        this->processOutputBrowser->document()->setMaximumBlockCount(value);
        Application::instance()->getApplicationSettings()->setProcessOutputMaxLines(uint(value));
    });
    this->pickDetailsTree = new PickDetailsTree;


    this->modelTabPosition = this->addTab(this->mdiArea,QString());
    this->applicationOutputTabPosition = this->addTab(this->applicationOutputBrowser,QString());
    this->processOutputTabPosition = this->addTab(this->processOutputBrowser,QString());
    this->pickDetailsTabPosition = this->addTab(this->pickDetailsTree,QString());

    this->setTabTitle(CentralTabWidget::Model);
    this->setTabTitle(CentralTabWidget::ApplicationOutput);
    this->setTabTitle(CentralTabWidget::ProcessOutput);
    this->setTabTitle(CentralTabWidget::PickDetails);

    this->tileWindowsAvailable = false;
    this->tileWindowsButton = new QToolButton(this);
    this->tileWindowsButton->setText(tr("Tile windows"));
    this->tileWindowsButton->setIcon(RIcon::fromFile(":/icons/file/pixmaps/range-tile_windows.svg"));
    this->tileWindowsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    this->tileWindowsButton->setAutoRaise(true);
    this->tileWindowsButton->hide();
    QObject::connect(this->tileWindowsButton,&QToolButton::clicked,this,&CentralTabWidget::tileWindowsClicked);

    QMenu *windowsMenu = new QMenu(this);
    QObject::connect(windowsMenu,&QMenu::aboutToShow,this,[this,windowsMenu]() { emit this->windowsMenuAboutToShow(windowsMenu); });

    this->windowListAvailable = false;
    this->windowsButton = new QToolButton(this);
    this->windowsButton->setText(tr("Windows"));
    this->windowsButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    this->windowsButton->setAutoRaise(true);
    this->windowsButton->setMenu(windowsMenu);
    this->windowsButton->setPopupMode(QToolButton::InstantPopup);
    this->windowsButton->hide();

    QWidget *windowButtons = new QWidget(this);
    QHBoxLayout *windowButtonsLayout = new QHBoxLayout(windowButtons);
    windowButtonsLayout->setContentsMargins(0,0,0,0);
    windowButtonsLayout->setSpacing(0);
    windowButtonsLayout->addWidget(this->tileWindowsButton);
    windowButtonsLayout->addWidget(this->windowsButton);
    this->setCornerWidget(windowButtons,Qt::BottomRightCorner);

    QObject::connect(this,&QTabWidget::currentChanged,this,&CentralTabWidget::onCurrentChanged);

    // Pick list signals
    QObject::connect(&Application::instance()->getSession()->getPickList(),&PickList::pickListChanged,this,&CentralTabWidget::onPickLostChanged);

    // RLoggerHandler signals
    QObject::connect(&RLoggerHandler::getInstance(),&RLoggerHandler::infoPrinted,this,&CentralTabWidget::onInfoPrinted);
    QObject::connect(&RLoggerHandler::getInstance(),&RLoggerHandler::noticePrinted,this,&CentralTabWidget::onNoticePrinted);
    QObject::connect(&RLoggerHandler::getInstance(),&RLoggerHandler::warningPrinted,this,&CentralTabWidget::onWarningPrinted);
    QObject::connect(&RLoggerHandler::getInstance(),&RLoggerHandler::errorPrinted,this,&CentralTabWidget::onErrorPrinted);

    // Solver manager signals
    QObject::connect(&SolverManager::getInstance(),&SolverManager::readyReadStandardOutput,this,&CentralTabWidget::onProcessReadyStandardOutput);
    QObject::connect(&SolverManager::getInstance(),&SolverManager::readyReadStandardError,this,&CentralTabWidget::onProcessReadyStandardError);
    QObject::connect(&SolverManager::getInstance(),&SolverManager::jobStarted,this,&CentralTabWidget::onSolverJobStarted);
    QObject::connect(&SolverManager::getInstance(),&SolverManager::jobFinished,this,&CentralTabWidget::onSolverJobFinished);
    QObject::connect(&SolverManager::getInstance(),&SolverManager::jobFailed,this,&CentralTabWidget::onSolverJobFailed);
}

void CentralTabWidget::setTileWindowsAvailable(bool available)
{
    this->tileWindowsAvailable = available;
    this->updateWindowButtons();
}

void CentralTabWidget::setWindowListAvailable(bool available)
{
    this->windowListAvailable = available;
    this->updateWindowButtons();
}

void CentralTabWidget::updateWindowButtons()
{
    const bool modelTabShown = this->currentIndex() == this->modelTabPosition;
    this->tileWindowsButton->setVisible(this->tileWindowsAvailable && modelTabShown);
    this->windowsButton->setVisible(this->windowListAvailable && modelTabShown);
}

void CentralTabWidget::changeEvent(QEvent *event)
{
    // Area behind sub windows follows the color scheme.
    if (event->type() == QEvent::PaletteChange)
    {
        this->mdiArea->setBackground(this->palette().brush(QPalette::Active,QPalette::Window));
    }
    QTabWidget::changeEvent(event);
}

QMdiArea *CentralTabWidget::getMdiArea()
{
    return this->mdiArea;
}

void CentralTabWidget::setTabTitle(CentralTabWidget::Type tabType, RMessage::Type messageType, const QString &additionalText)
{
    QString defaultText;
    int tabPosition = 0;
    QString iconFile;

    if (tabType == CentralTabWidget::Model)
    {
        defaultText = tr("Model");
        tabPosition = this->modelTabPosition;
    }
    else if (tabType == CentralTabWidget::ApplicationOutput)
    {
        defaultText = tr("Application output");
        tabPosition = this->applicationOutputTabPosition;
        iconFile = this->applicationOutputLastIcon;
    }
    else if (tabType == CentralTabWidget::ProcessOutput)
    {
        defaultText = tr("Process output");
        tabPosition = this->processOutputTabPosition;
        iconFile = this->processOutputLastIcon;
    }
    else if (tabType == CentralTabWidget::PickDetails)
    {
        defaultText = tr("Pick details");
        tabPosition = this->pickDetailsTabPosition;
    }
    else
    {
        return;
    }

    if (!additionalText.isEmpty())
    {
        defaultText += " (" + additionalText + ")";
    }

    if (this->currentIndex() == tabPosition)
    {
        iconFile = QString();
    }
    else
    {
        if (messageType == RMessage::Type::Info)
        {
            if (iconFile != CentralTabWidget::importantIconFile && iconFile != CentralTabWidget::severeIconFile)
            {
                iconFile = CentralTabWidget::informationIconFile;
            }
        }
        else if (messageType == RMessage::Type::Warning)
        {
            if (iconFile != CentralTabWidget::severeIconFile)
            {
                iconFile = CentralTabWidget::importantIconFile;
            }
        }
        else if (messageType == RMessage::Type::Error)
        {
            iconFile = CentralTabWidget::severeIconFile;
        }
    }

    if (tabType == CentralTabWidget::ApplicationOutput)
    {
        this->applicationOutputLastIcon = iconFile;
    }
    else if (tabType == CentralTabWidget::ProcessOutput)
    {
        this->processOutputLastIcon = iconFile;
    }

    // An invalid color lets the tab follow the palette, also after the color
    // scheme changes. The palette color at this moment would stay.
    QColor tabTextColor;
    if (!iconFile.isEmpty())
    {
        // TODO: For unknown reason this hangs if text is printed from QDialog::exec()
        if (useTabIcons)
        {
            this->setTabIcon(tabPosition,RIcon::fromFile(iconFile));
        }
        if (iconFile == CentralTabWidget::informationIconFile)
        {
            if (!useTabIcons)
            {
                defaultText.prepend("* ");
            }
            tabTextColor = Qt::darkCyan;
        }
        else if (iconFile == CentralTabWidget::importantIconFile)
        {
            if (!useTabIcons)
            {
                defaultText.prepend("! ");
            }
            tabTextColor = Qt::red;
        }
        else if (iconFile == CentralTabWidget::severeIconFile)
        {
            if (!useTabIcons)
            {
                defaultText.prepend("!!! ");
                defaultText.append(" !!!");
            }
            tabTextColor = Qt::red;
        }
    }
    this->tabBar()->setTabTextColor(tabPosition,tabTextColor);
    this->setTabText(tabPosition,defaultText);
}

void CentralTabWidget::onInfoPrinted(const RMessage &message)
{
    QScrollBar *sb = this->applicationOutputBrowser->verticalScrollBar();
    int scrollBarValue = sb->value();
    bool scrollBarAtMax = ((sb->maximum() - scrollBarValue) <= 10);

    appendText(this->applicationOutputBrowser,RMessage::messageToLogString(message));
    if (scrollBarAtMax)
    {
        scrollBarValue = sb->maximum();
    }
    sb->setValue(scrollBarValue);

    this->setTabTitle(CentralTabWidget::ApplicationOutput,RMessage::Type::Info);
}

void CentralTabWidget::onNoticePrinted(const RMessage &message)
{
    RMessageBox::information(this,tr("Notice"),QString(message).replace("NOTICE: ","").trimmed());
}

void CentralTabWidget::onWarningPrinted(const RMessage &message)
{
    appendText(this->applicationOutputBrowser,RMessage::messageToLogString(message),QColor(170,0,0));
    QScrollBar *sb = this->applicationOutputBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());
    this->setTabTitle(CentralTabWidget::ApplicationOutput,RMessage::Type::Warning);
}

void CentralTabWidget::onErrorPrinted(const RMessage &message)
{
    appendText(this->applicationOutputBrowser,RMessage::messageToLogString(message),QColor(255,255,255),QColor(170,0,0));
    QScrollBar *sb = this->applicationOutputBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());
//    this->setCurrentIndex(this->applicationOutputTabPosition);
    this->setTabTitle(CentralTabWidget::ApplicationOutput,RMessage::Type::Error);
}

void CentralTabWidget::onProcessReadyStandardOutput(const QString &message)
{
    QScrollBar *sb = this->processOutputBrowser->verticalScrollBar();
    int scrollBarValue = sb->value();
    bool scrollBarAtMax = ((sb->maximum() - scrollBarValue) <= 10);

    appendText(this->processOutputBrowser,message);
    if (scrollBarAtMax)
    {
        scrollBarValue = sb->maximum();
    }
    sb->setValue(scrollBarValue);

    this->setTabTitle(CentralTabWidget::ProcessOutput,RMessage::Type::Info);
}

void CentralTabWidget::onProcessReadyStandardError(const QString &message)
{
    appendText(this->processOutputBrowser,message,QColor(255,255,255),QColor(170,0,0));
    QScrollBar *sb = this->processOutputBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());
    this->setCurrentIndex(this->processOutputTabPosition);
    this->setTabTitle(CentralTabWidget::ProcessOutput,RMessage::Type::Error);
}

void CentralTabWidget::onCurrentChanged(int tabPosition)
{
    this->updateWindowButtons();
    if (useTabIcons)
    {
        this->setTabIcon(tabPosition,QIcon());
    }
    if (tabPosition == this->applicationOutputTabPosition)
    {
        this->setTabTitle(CentralTabWidget::ApplicationOutput);
    }
    else if (tabPosition == this->processOutputTabPosition)
    {
        this->setTabTitle(CentralTabWidget::ProcessOutput);
    }
}

void CentralTabWidget::onPickLostChanged()
{
    int nItems = Application::instance()->getSession()->getPickList().getItems().size();
    if (nItems > 0)
    {
        this->setTabTitle(CentralTabWidget::PickDetails,RMessage::Type::None,QString::number(nItems));
    }
    else
    {
        this->setTabTitle(CentralTabWidget::PickDetails);
    }
}

void CentralTabWidget::onSolverJobStarted()
{
    this->setCurrentIndex(this->processOutputTabPosition);
}

void CentralTabWidget::onSolverJobFinished()
{
    // Results have been loaded into the model by now - show them unless the
    // user has switched to another tab while the solver was running.
    if (this->currentIndex() == this->processOutputTabPosition)
    {
        this->setCurrentIndex(this->modelTabPosition);
    }
}

void CentralTabWidget::onSolverJobFailed()
{
    this->setCurrentIndex(this->processOutputTabPosition);
}
