#include "application.h"
#include "sub_window_manager.h"

SubWindowManager::SubWindowManager(QMdiArea *mdiArea, QObject *parent) : QObject(parent)
{
    R_LOG_TRACE_IN;
    this->mdiArea = mdiArea;
    R_LOG_TRACE_OUT;
}

uint SubWindowManager::findWindow(uint modelID)
{
    R_LOG_TRACE_IN;
    for (int i=0;i<this->subWindows.size();i++)
    {
        if (this->subWindows[i]->getModelID() == modelID)
        {
            R_LOG_TRACE_OUT;
            return uint(i);
        }
    }
    R_LOG_TRACE_OUT;
    return uint(this->subWindows.size());
}

bool SubWindowManager::windowCreate(uint modelID)
{
    R_LOG_TRACE_IN;
    if (this->windowExists(modelID))
    {
        R_LOG_TRACE_OUT;
        return false;
    }

    SubWindow *subWindow = new SubWindow(modelID,qobject_cast<QWidget*>(this->parent()));
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    this->mdiArea->addSubWindow(subWindow,Qt::SubWindow);

    if (this->subWindows.empty())
    {
        subWindow->showMaximized();
    }

    this->subWindows.append(subWindow);

    subWindow->show();

    QObject::connect(subWindow,
                     &SubWindow::closed,
                     this,
                     &SubWindowManager::onWindowClosed);

    emit this->windowCreated(modelID);

    R_LOG_TRACE_OUT;
    return true;
}

bool SubWindowManager::windowClose(uint modelID)
{
    R_LOG_TRACE_IN;
    uint winPos = this->findWindow(modelID);
    if (winPos == this->getNWindows())
    {
        R_LOG_TRACE_OUT;
        return false;
    }

    SubWindow *subWindow = this->subWindows[int(winPos)];
//    this->mdiArea->removeSubWindow(subWindow);
    delete subWindow;

    this->subWindows.removeAt(int(winPos));

    emit this->windowClosed(modelID);

    R_LOG_TRACE_OUT;
    return true;
}

bool SubWindowManager::windowExists(uint modelID)
{
    R_LOG_TRACE_IN;
    uint winPos = this->findWindow(modelID);
    if (winPos == this->getNWindows())
    {
        R_LOG_TRACE_OUT;
        return false;
    }
    R_LOG_TRACE_OUT;
    return true;
}

uint SubWindowManager::getNWindows() const
{
    R_LOG_TRACE;
    return this->subWindows.size();
}

void SubWindowManager::onModelAdded(uint modelID)
{
    R_LOG_TRACE_IN;
    for (int i=0;i<this->subWindows.size();i++)
    {
        if (this->subWindows[i]->getModelID() >= modelID)
        {
            this->subWindows[i]->setModelID(this->subWindows[i]->getModelID()+1);
        }
    }

    this->windowCreate(modelID);
    R_LOG_TRACE_OUT;
}

void SubWindowManager::onModelRemoved(uint modelID)
{
    R_LOG_TRACE_IN;
    this->windowClose(modelID);

    for (int i=0;i<this->subWindows.size();i++)
    {
        if (this->subWindows[i]->getModelID() > modelID)
        {
            this->subWindows[i]->setModelID(this->subWindows[i]->getModelID()-1);
        }
    }
    R_LOG_TRACE_OUT;
}

void SubWindowManager::onModelChanged(uint)
{
    R_LOG_TRACE;
}

void SubWindowManager::onWindowClosed(uint modelID)
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->setModelVisible(modelID,false);
    uint winPos = this->findWindow(modelID);
    this->subWindows.removeAt(winPos);
    R_LOG_TRACE_OUT;
}
