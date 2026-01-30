#include <QMessageBox>
#include <QApplication>
#include <QScrollBar>

#include <rgl_logger_handler.h>

#include "central_tab_widget.h"
#include "application.h"
#include "solver_manager.h"

const QString CentralTabWidget::informationIconFile(":/icons/file/pixmaps/range-information.svg");
const QString CentralTabWidget::importantIconFile(":/icons/file/pixmaps/range-important.svg");
const QString CentralTabWidget::severeIconFile(":/icons/file/pixmaps/range-severe.svg");

const bool useTabIcons = false;

CentralTabWidget::CentralTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    this->setTabPosition(QTabWidget::South);

    this->mdiArea = new QMdiArea(this);
    this->mdiArea->setObjectName(QString::fromUtf8("mdiArea"));
    this->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    this->applicationOutputBrowser = new RTextBrowser(true);
    this->processOutputBrowser = new RTextBrowser(true);
    this->pickDetailsTree = new PickDetailsTree;


    this->modelTabPosition = this->addTab(this->mdiArea,QString());
    this->applicationOutputTabPosition = this->addTab(this->applicationOutputBrowser,QString());
    this->processOutputTabPosition = this->addTab(this->processOutputBrowser,QString());
    this->pickDetailsTabPosition = this->addTab(this->pickDetailsTree,QString());

    this->setTabTitle(CentralTabWidget::Model);
    this->setTabTitle(CentralTabWidget::ApplicationOutput);
    this->setTabTitle(CentralTabWidget::ProcessOutput);
    this->setTabTitle(CentralTabWidget::PickDetails);

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

    QColor tabTextColor = QApplication::palette().text().color();
    if (!iconFile.isEmpty())
    {
        // TODO: For unknown reason this hangs if text is printed from QDialog::exec()
        if (useTabIcons)
        {
            this->setTabIcon(tabPosition,QIcon(iconFile));
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

QString CentralTabWidget::messageToString(const RMessage &message)
{
    return QString("[%1] %2").arg(RMessage::aTimeToString(message.getAtime()),message);
}

void CentralTabWidget::onInfoPrinted(const RMessage &message)
{
    QScrollBar *sb = this->applicationOutputBrowser->verticalScrollBar();
    int scrollBarValue = sb->value();
    bool scrollBarAtMax = ((sb->maximum() - scrollBarValue) <= 10);

    this->applicationOutputBrowser->moveCursor(QTextCursor::End);
    this->applicationOutputBrowser->setTextBackgroundColor(QApplication::palette().base().color());
    this->applicationOutputBrowser->setTextColor(QApplication::palette().text().color());
    this->applicationOutputBrowser->insertPlainText(CentralTabWidget::messageToString(message));
    this->applicationOutputBrowser->moveCursor(QTextCursor::End);
    if (scrollBarAtMax)
    {
        scrollBarValue = sb->maximum();
    }
    sb->setValue(scrollBarValue);

    this->setTabTitle(CentralTabWidget::ApplicationOutput,RMessage::Type::Info);
}

void CentralTabWidget::onNoticePrinted(const RMessage &message)
{
    QMessageBox::information(this,tr("Notice"),QString(CentralTabWidget::messageToString(message)).replace("NOTICE: ",""));
}

void CentralTabWidget::onWarningPrinted(const RMessage &message)
{
    QTextCharFormat charFormat = this->applicationOutputBrowser->currentCharFormat();

    this->applicationOutputBrowser->moveCursor(QTextCursor::End);
    this->applicationOutputBrowser->setTextBackgroundColor(QApplication::palette().base().color());
    this->applicationOutputBrowser->setTextColor(QColor(170,0,0));
    this->applicationOutputBrowser->insertPlainText(CentralTabWidget::messageToString(message));
    this->applicationOutputBrowser->moveCursor(QTextCursor::End);
    this->applicationOutputBrowser->setTextColor(QApplication::palette().text().color());
    this->applicationOutputBrowser->setCurrentCharFormat(charFormat);
    QScrollBar *sb = this->applicationOutputBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());
    this->setTabTitle(CentralTabWidget::ApplicationOutput,RMessage::Type::Warning);
}

void CentralTabWidget::onErrorPrinted(const RMessage &message)
{
    QTextCharFormat charFormat = this->applicationOutputBrowser->currentCharFormat();

    this->applicationOutputBrowser->moveCursor(QTextCursor::End);
    this->applicationOutputBrowser->setTextBackgroundColor(QColor(170,0,0));
    this->applicationOutputBrowser->setTextColor(QColor(255,255,255));
    this->applicationOutputBrowser->insertPlainText(CentralTabWidget::messageToString(message));
    this->applicationOutputBrowser->moveCursor(QTextCursor::End);
    this->applicationOutputBrowser->setTextBackgroundColor(QApplication::palette().base().color());
    this->applicationOutputBrowser->setTextColor(QApplication::palette().text().color());
    this->applicationOutputBrowser->setCurrentCharFormat(charFormat);
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

    this->processOutputBrowser->moveCursor(QTextCursor::End);
    this->processOutputBrowser->setTextBackgroundColor(QApplication::palette().base().color());
    this->processOutputBrowser->setTextColor(QApplication::palette().text().color());
    this->processOutputBrowser->insertPlainText(message);
    this->processOutputBrowser->moveCursor(QTextCursor::End);
    if (scrollBarAtMax)
    {
        scrollBarValue = sb->maximum();
    }
    sb->setValue(scrollBarValue);

    this->setTabTitle(CentralTabWidget::ProcessOutput,RMessage::Type::Info);
}

void CentralTabWidget::onProcessReadyStandardError(const QString &message)
{
    QTextCharFormat charFormat = this->processOutputBrowser->currentCharFormat();

    this->processOutputBrowser->moveCursor(QTextCursor::End);
    this->processOutputBrowser->setTextBackgroundColor(QColor(170,0,0));
    this->processOutputBrowser->setTextColor(QColor(255,255,255));
    this->processOutputBrowser->insertPlainText(message);
    this->processOutputBrowser->moveCursor(QTextCursor::End);
    this->processOutputBrowser->setTextBackgroundColor(QApplication::palette().base().color());
    this->processOutputBrowser->setTextColor(QApplication::palette().text().color());
    this->processOutputBrowser->setCurrentCharFormat(charFormat);
    QScrollBar *sb = this->processOutputBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());
    this->setCurrentIndex(this->processOutputTabPosition);
    this->setTabTitle(CentralTabWidget::ProcessOutput,RMessage::Type::Error);
}

void CentralTabWidget::onCurrentChanged(int tabPosition)
{
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
    this->setCurrentIndex(this->processOutputTabPosition);
}

void CentralTabWidget::onSolverJobFailed()
{
    this->setCurrentIndex(this->processOutputTabPosition);
}
