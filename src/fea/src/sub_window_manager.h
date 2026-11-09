#ifndef SUB_WINDOW_MANAGER_H
#define SUB_WINDOW_MANAGER_H

#include <QObject>
#include <QList>
#include <QMdiArea>
#include <QMenu>
#include <QPointer>
#include <QSettings>

#include "sub_window.h"

//! Sub Window manager class.
class SubWindowManager : public QObject
{

    Q_OBJECT

    private:

        //! Stored layout of one window, windows are matched by file name as models are not loaded in a fixed order.
        struct WindowLayout
        {
            QString fileName;
            QRect geometry;
            bool maximized = false;
            bool minimized = false;
            bool applied = false;
            QPointer<SubWindow> window;
        };

        //! Layout stored by previous run, windows are ordered from bottom to top.
        QList<WindowLayout> storedLayout;

        //! Size of MDI area when the layout was stored.
        QSize storedAreaSize;

        //! Window placed in a slot of the tile layout, window pointer is only compared.
        struct TiledWindow
        {
            const SubWindow *window = nullptr;
            int slot = 0;
            QRect geometry;
        };

        //! Windows placed by last tiling.
        QList<TiledWindow> tiledWindows;

    protected:

        //! Model subwindows.
        QList<SubWindow*>subWindows;

        //! MDI area.
        QMdiArea *mdiArea;

        //! Windows can be tiled.
        bool tilingAvailable;

        //! There are multiple windows to choose from.
        bool windowListAvailable;

    private:

        //! Find window position for given model ID.
        uint findWindow(uint modelID);

        //! Tell whether windows can be tiled and whether there are multiple windows to choose from.
        void updateTilingAvailable();

        //! Apply stored layout to given new window, return true if there was one.
        bool applyStoredLayout(SubWindow *subWindow);

        //! Restore stacking of windows with applied stored layout.
        void restoreStoredStacking();

        //! Return windows which take part in tiling.
        QList<SubWindow*> findTilableWindows() const;

        //! Return true if given windows are still placed as by last tiling.
        bool isTiled(const QList<SubWindow*> &windows) const;

        //! Move given windows to slots of the tile layout, returned by Qt for the current size of MDI area.
        void placeInSlots(const QList<SubWindow*> &windows, const QList<QRect> &slotGeometries, const QList<int> &windowSlots);

    protected:

        //! Keep rotated tile layout when MDI area is resized.
        bool eventFilter(QObject *watched, QEvent *event) override;

    public:

        //! Constructor.
        explicit SubWindowManager(QMdiArea *mdiArea, QObject *parent = nullptr);

        //! Create new window.
        bool windowCreate(uint modelID);

        //! Close window.
        bool windowClose(uint modelID);

        //! Check whether there is a window for given model ID.
        bool windowExists(uint modelID);

        //! Return number of windows.
        uint getNWindows() const;

        //! Read layout of windows stored by previous run, it is applied as windows are created.
        void readLayout(QSettings *settings);

        //! Store layout of windows.
        void writeLayout(QSettings *settings);

    public slots:

        //! Model was added.
        void onModelAdded(uint modelID);

        //! Model was removed.
        void onModelRemoved(uint modelID);

        //! Model was changed.
        void onModelChanged(uint);

        //! Window was closed.
        void onWindowClosed(uint modelID);

        //! Tile windows, windows which are already tiled move one position clockwise.
        void tileWindows();

        //! Fill given menu with windows, clicking on an item maximizes the window.
        void fillWindowsMenu(QMenu *menu);

    signals:

        //! Window was created for given model ID.
        void windowCreated(uint modelID);

        //! Window was closed for given model ID.
        void windowClosed(uint modelID);

        //! Availability of tiling windows has changed.
        void tilingAvailableChanged(bool available);

        //! Availability of multiple windows to choose from has changed.
        void windowListAvailableChanged(bool available);

};

#endif /* SUB_WINDOW_MANAGER_H */
