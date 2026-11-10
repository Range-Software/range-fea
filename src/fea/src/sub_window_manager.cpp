#include <QEvent>
#include <QPainter>
#include <QPixmap>

#include <algorithm>
#include <cmath>
#include <numeric>

#include "application.h"
#include "sub_window_manager.h"

// Return positions of given rectangles ordered clockwise around their center.
static QList<int> findClockwiseOrder(const QList<QRect> &rects)
{
    QRect bounds;
    for (const QRect &rect : rects)
    {
        bounds |= rect;
    }
    const QPointF center = QRectF(bounds).center();

    QList<double> angles;
    for (const QRect &rect : rects)
    {
        const QPointF direction = QRectF(rect).center() - center;
        // Y axis points down, so the angle grows clockwise.
        angles.append(std::atan2(direction.y(),direction.x()));
    }

    QList<int> order(rects.size());
    std::iota(order.begin(),order.end(),0);
    std::sort(order.begin(),order.end(),[&angles](int a, int b) { return angles[a] < angles[b]; });
    return order;
}

// Return icon for item of given menu with a dot in given color, invalid color gives an empty icon.
static QIcon createMenuDotIcon(const QMenu *menu, const QColor &color)
{
    const qreal ratio = menu->devicePixelRatioF();
    QPixmap pixmap(QSize(16,16)*ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    if (color.isValid())
    {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawEllipse(QPointF(8.0,8.0),4.0,4.0);
    }

    return QIcon(pixmap);
}

SubWindowManager::SubWindowManager(QMdiArea *mdiArea, QObject *parent) : QObject(parent)
{
    R_LOG_TRACE_IN;
    this->mdiArea = mdiArea;
    this->tilingAvailable = false;
    this->windowListAvailable = false;
    this->mdiArea->viewport()->installEventFilter(this);
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

void SubWindowManager::updateTilingAvailable()
{
    R_LOG_TRACE_IN;
    int nTilableWindows = 0;
    for (const SubWindow *subWindow : std::as_const(this->subWindows))
    {
        nTilableWindows += subWindow->isMinimized() ? 0 : 1;
    }

    bool available = nTilableWindows > 1;
    if (available != this->tilingAvailable)
    {
        this->tilingAvailable = available;
        emit this->tilingAvailableChanged(available);
    }

    bool listAvailable = this->subWindows.size() > 1;
    if (listAvailable != this->windowListAvailable)
    {
        this->windowListAvailable = listAvailable;
        emit this->windowListAvailableChanged(listAvailable);
    }
    R_LOG_TRACE_OUT;
}

QList<SubWindow*> SubWindowManager::findTilableWindows() const
{
    QList<SubWindow*> windows;
    for (SubWindow *subWindow : std::as_const(this->subWindows))
    {
        if (!subWindow->isMinimized())
        {
            windows.append(subWindow);
        }
    }
    return windows;
}

bool SubWindowManager::isTiled(const QList<SubWindow*> &windows) const
{
    if (windows.isEmpty() || windows.size() != this->tiledWindows.size())
    {
        return false;
    }
    for (int i=0;i<windows.size();i++)
    {
        if (windows[i] != this->tiledWindows[i].window || windows[i]->geometry() != this->tiledWindows[i].geometry)
        {
            return false;
        }
    }
    return true;
}

void SubWindowManager::placeInSlots(const QList<SubWindow*> &windows, const QList<QRect> &slotGeometries, const QList<int> &windowSlots)
{
    this->tiledWindows.clear();
    for (int i=0;i<windows.size();i++)
    {
        windows[i]->setGeometry(slotGeometries[windowSlots[i]]);

        TiledWindow tiledWindow;
        tiledWindow.window = windows[i];
        tiledWindow.slot = windowSlots[i];
        tiledWindow.geometry = windows[i]->geometry();
        this->tiledWindows.append(tiledWindow);
    }
}

bool SubWindowManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize && watched == this->mdiArea->viewport())
    {
        const QList<SubWindow*> windows = this->findTilableWindows();
        if (this->isTiled(windows))
        {
            QList<int> windowSlots;
            for (const TiledWindow &tiledWindow : std::as_const(this->tiledWindows))
            {
                windowSlots.append(tiledWindow.slot);
            }

            this->mdiArea->tileSubWindows();

            QList<QRect> slotGeometries;
            for (const SubWindow *subWindow : windows)
            {
                slotGeometries.append(subWindow->geometry());
            }
            this->placeInSlots(windows,slotGeometries,windowSlots);
        }
    }
    return QObject::eventFilter(watched,event);
}

void SubWindowManager::fillWindowsMenu(QMenu *menu)
{
    R_LOG_TRACE_IN;
    menu->clear();

    // Items without the dot have an empty icon to keep names aligned.
    const QIcon dotIcon = createMenuDotIcon(menu,menu->palette().color(QPalette::WindowText));
    const QIcon emptyIcon = createMenuDotIcon(menu,QColor());

    for (SubWindow *subWindow : std::as_const(this->subWindows))
    {
        // Name is taken from the model, title of a window does not follow renaming.
        QString name = Application::instance()->getSession()->getModel(subWindow->getModelID()).getName();
        QAction *action = menu->addAction(name.replace("&","&&"));
        action->setIcon(subWindow->isMaximized() ? dotIcon : emptyIcon);
        action->setIconVisibleInMenu(true);
        QObject::connect(action,&QAction::triggered,subWindow,[this,subWindow]()
        {
            subWindow->showMaximized();
            this->mdiArea->setActiveSubWindow(subWindow);
        });
    }
    R_LOG_TRACE_OUT;
}

void SubWindowManager::tileWindows()
{
    R_LOG_TRACE_IN;
    const QList<SubWindow*> windows = this->findTilableWindows();

    QList<QRect> geometries;
    for (const SubWindow *subWindow : windows)
    {
        geometries.append(subWindow->geometry());
    }

    // Tiling places every window into its own slot, which is always the same one.
    this->mdiArea->tileSubWindows();

    QList<QRect> slotGeometries;
    for (const SubWindow *subWindow : windows)
    {
        slotGeometries.append(subWindow->geometry());
    }

    QList<int> windowSlots(windows.size());
    std::iota(windowSlots.begin(),windowSlots.end(),0);

    // Windows which were already tiled move to the next slot.
    if (std::is_permutation(geometries.cbegin(),geometries.cend(),slotGeometries.cbegin(),slotGeometries.cend()))
    {
        const QList<int> clockwiseOrder = findClockwiseOrder(slotGeometries);
        for (int i=0;i<windows.size();i++)
        {
            const int position = clockwiseOrder.indexOf(slotGeometries.indexOf(geometries[i]));
            windowSlots[i] = clockwiseOrder[(position+1)%clockwiseOrder.size()];
        }
    }

    this->placeInSlots(windows,slotGeometries,windowSlots);
    R_LOG_TRACE_OUT;
}

bool SubWindowManager::applyStoredLayout(SubWindow *subWindow)
{
    R_LOG_TRACE_IN;
    const QString &fileName = Application::instance()->getSession()->getModel(subWindow->getModelID()).getFileName();

    for (WindowLayout &layout : this->storedLayout)
    {
        if (layout.applied || layout.fileName != fileName)
        {
            continue;
        }

        // Main window might have a different size than when the layout was stored.
        const QSize areaSize = this->mdiArea->viewport()->size();
        double scaleX = 1.0;
        double scaleY = 1.0;
        if (!areaSize.isEmpty() && !this->storedAreaSize.isEmpty())
        {
            scaleX = double(areaSize.width()) / double(this->storedAreaSize.width());
            scaleY = double(areaSize.height()) / double(this->storedAreaSize.height());
        }
        subWindow->setGeometry(qRound(layout.geometry.x()*scaleX),
                               qRound(layout.geometry.y()*scaleY),
                               qRound(layout.geometry.width()*scaleX),
                               qRound(layout.geometry.height()*scaleY));

        if (layout.maximized)
        {
            subWindow->showMaximized();
        }
        else if (layout.minimized)
        {
            subWindow->showMinimized();
        }

        layout.applied = true;
        layout.window = subWindow;

        R_LOG_TRACE_OUT;
        return true;
    }

    R_LOG_TRACE_OUT;
    return false;
}

void SubWindowManager::restoreStoredStacking()
{
    R_LOG_TRACE_IN;
    SubWindow *topWindow = nullptr;
    for (const WindowLayout &layout : std::as_const(this->storedLayout))
    {
        if (layout.window)
        {
            layout.window->raise();
            if (!layout.minimized)
            {
                topWindow = layout.window;
            }
        }
    }
    if (topWindow)
    {
        this->mdiArea->setActiveSubWindow(topWindow);
    }
    R_LOG_TRACE_OUT;
}

void SubWindowManager::readLayout(QSettings *settings)
{
    R_LOG_TRACE_IN;
    this->storedLayout.clear();

    settings->beginGroup("modelWindows");
    this->storedAreaSize = settings->value("areaSize").toSize();
    const int nWindows = settings->beginReadArray("windows");
    for (int i=0;i<nWindows;i++)
    {
        settings->setArrayIndex(i);

        WindowLayout layout;
        layout.fileName = settings->value("fileName").toString();
        layout.geometry = settings->value("geometry").toRect();
        layout.maximized = settings->value("maximized",false).toBool();
        layout.minimized = settings->value("minimized",false).toBool();
        this->storedLayout.append(layout);
    }
    settings->endArray();
    settings->endGroup();
    R_LOG_TRACE_OUT;
}

void SubWindowManager::writeLayout(QSettings *settings)
{
    R_LOG_TRACE_IN;
    settings->remove("modelWindows");

    settings->beginGroup("modelWindows");
    settings->setValue("areaSize",this->mdiArea->viewport()->size());
    settings->beginWriteArray("windows");

    const QList<QMdiSubWindow*> windows = this->mdiArea->subWindowList(QMdiArea::StackingOrder);
    int nWritten = 0;
    for (QMdiSubWindow *window : windows)
    {
        SubWindow *subWindow = qobject_cast<SubWindow*>(window);
        if (!subWindow)
        {
            continue;
        }

        // Model which was never saved has no file to be matched with.
        const QString &fileName = Application::instance()->getSession()->getModel(subWindow->getModelID()).getFileName();
        if (fileName.isEmpty())
        {
            continue;
        }

        const bool maximized = subWindow->isMaximized();
        const bool minimized = subWindow->isMinimized();

        // Size of normal window can be read only in normal state.
        if (maximized || minimized)
        {
            subWindow->showNormal();
        }
        const QRect geometry = subWindow->geometry();
        if (maximized)
        {
            subWindow->showMaximized();
        }
        else if (minimized)
        {
            subWindow->showMinimized();
        }

        settings->setArrayIndex(nWritten++);
        settings->setValue("fileName",fileName);
        settings->setValue("geometry",geometry);
        settings->setValue("maximized",maximized);
        settings->setValue("minimized",minimized);
    }

    settings->endArray();
    settings->endGroup();
    R_LOG_TRACE_OUT;
}

bool SubWindowManager::windowCreate(uint modelID)
{
    R_LOG_TRACE_IN;
    if (this->windowExists(modelID))
    {
        R_LOG_TRACE_OUT;
        return false;
    }

    const QList<SubWindow*> previousWindows = this->findTilableWindows();
    const bool previousWindowMaximized = std::any_of(previousWindows.cbegin(),previousWindows.cend(),[](const SubWindow *window) { return window->isMaximized(); });
    const bool previousWindowsTiled = this->isTiled(previousWindows);

    SubWindow *subWindow = new SubWindow(modelID,qobject_cast<QWidget*>(this->parent()));
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    this->mdiArea->addSubWindow(subWindow,Qt::SubWindow);

    // Default size takes 60% of the area. This is also the size restored after being maximized.
    const QSize areaSize = this->mdiArea->viewport()->size();
    subWindow->resize((0.6*areaSize).expandedTo(subWindow->minimumSizeHint()));

    const bool layoutApplied = this->applyStoredLayout(subWindow);

    if (!layoutApplied && this->subWindows.empty())
    {
        subWindow->showMaximized();
    }

    this->subWindows.append(subWindow);

    subWindow->show();

    if (layoutApplied)
    {
        this->restoreStoredStacking();
    }
    else if (previousWindowMaximized || previousWindowsTiled)
    {
        this->tileWindows();
    }

    QObject::connect(subWindow,
                     &SubWindow::closed,
                     this,
                     &SubWindowManager::onWindowClosed);

    QObject::connect(subWindow,
                     &QMdiSubWindow::windowStateChanged,
                     this,
                     &SubWindowManager::updateTilingAvailable);

    this->updateTilingAvailable();

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
    // Deleting the window changes state of other windows, which goes through the list of windows.
    this->subWindows.removeAt(int(winPos));
//    this->mdiArea->removeSubWindow(subWindow);
    delete subWindow;

    this->updateTilingAvailable();

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
    this->updateTilingAvailable();
    R_LOG_TRACE_OUT;
}
