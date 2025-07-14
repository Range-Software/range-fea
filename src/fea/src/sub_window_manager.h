#ifndef SUB_WINDOW_MANAGER_H
#define SUB_WINDOW_MANAGER_H

#include <QObject>
#include <QList>
#include <QMdiArea>

#include "sub_window.h"

//! Sub Window manager class.
class SubWindowManager : public QObject
{

    Q_OBJECT

    protected:

        //! Model subwindows.
        QList<SubWindow*>subWindows;

        //! MDI area.
        QMdiArea *mdiArea;

    private:

        //! Find window position for given model ID.
        uint findWindow(uint modelID);

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

    public slots:

        //! Model was added.
        void onModelAdded(uint modelID);

        //! Model was removed.
        void onModelRemoved(uint modelID);

        //! Model was changed.
        void onModelChanged(uint);

        //! Window was closed.
        void onWindowClosed(uint modelID);

    signals:

        //! Window was created for given model ID.
        void windowCreated(uint modelID);

        //! Window was closed for given model ID.
        void windowClosed(uint modelID);

};

#endif /* SUB_WINDOW_MANAGER_H */
