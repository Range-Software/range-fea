#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QDockWidget>
#include <QToolBar>
#include <QMainWindow>
#include <QLabel>
#include <QTabWidget>

#include <rgl_progress_bar.h>
#include <rgl_application_settings.h>
#include <rml_entity_group.h>

#include "central_tab_widget.h"

class ActionList;
class ModelTree;
class QMenuBar;
class RCloudSessionManager;
class RRibbonBar;
class RSplitToolButton;
class Session;
class SubWindowManager;
class RProgressBar;

class MainWindow : public QMainWindow
{

    Q_OBJECT

    private:

        //! Session.
        Session *session;
        //! Cloud cloudSession.
        RCloudSessionManager *cloudSession;
        //! Application settings.
        RApplicationSettings *applicationSettings;
        //! List of actions.
        ActionList *actionList;
        //! Subwindow manager.
        SubWindowManager *modelSubWindows;
        //! List of toolbars containing action types.
        QList<QToolBar*> toolBars;
        //! Drop-down buttons on tool bars.
        QList<RSplitToolButton*> toolBarSplitButtons;
        //! Menu bar shown in the layout with menus and tool bars.
        QMenuBar *classicMenuBar;
        //! Ribbon shown in the ribbon layout.
        RRibbonBar *ribbonBar;
        //! Model manager tab.
        QWidget *modelsTab;
        //! Document manager tab.
        QWidget *documentsTab;
        //! Record manager tab.
        QWidget *recordsTab;
        //! Problem setup manager tab.
        QWidget *problemTab;
        //! Boundary conditions manager tab.
        QWidget *bcTab;
        //! Initial conditions manager tab.
        QWidget *icTab;
        //! Environment conditions manager tab.
        QWidget *ecTab;
        //! Material manager tab.
        QWidget *materialTab;
        //! Results manager tab.
        QWidget *resultsTab;
        //! Tab widget holding problem setup tabs.
        QTabWidget *solverDockTabWidget;
        //! Central tab widget.
        CentralTabWidget *centralTabWidget;
        //! Model manager.
        ModelTree *modelManagerTree;
        //! Progress bar.
        RProgressBar* mainProgressBar;
        //! First run flag.
        bool isFirstRun;

    public:

        //! Constructor.
        explicit MainWindow(Session *session,
                            RCloudSessionManager *cloudSession,
                            RApplicationSettings *applicationSettings,
                            ActionList *actionList,
                            QWidget *parent = nullptr);

        //! Show first tab which needs input after given problem types were selected.
        void showProblemSetupTab(RProblemTypeMask problemTypeMask);

        //! Switch between ribbon layout and layout with menus and tool bars and remember the choice.
        void setRibbonLayout(bool ribbonLayout);

        //! Show transform geometry dock widget.
        void showTransformGeometryWidget();

        //! Hide transform geometry dock widget.
        void hideTransformGeometryWidget();

        //! Show draw dock widget.
        void showDrawWidget();

        //! Hide draw dock widget.
        void hideDrawWidget();

        //! Enable progress-bar autohide.
        void progressAutoHideEnable();

        //! Disable progress-bar autohide.
        void progressAutoHideDisable();

    protected:

        //! Find custom dock widget.
        QDockWidget *findCustomDockWidget(const QString &name);

        //! Show custom dock widget.
        void showCustomDockWidget(QWidget *widget, const QString &name, const QString &title);

        //! Hide custom dock widget.
        void hideCustomDockWidget(const QString &name);

    protected:

        //! Return list of selected model IDs.
        QList<uint> getSelectedModelIDs() const;

        //! Create all menu items.
        void createMenus();

        //! Create file menu.
        QMenu *createFileMenu(QWidget *parent);

        //! Create menu of special geometry tools.
        QMenu *createSpecialToolsMenu(QWidget *parent);

        //! Create cloud menu.
        QMenu *createCloudMenu(QWidget *parent);

        //! Create help menu.
        QMenu *createHelpMenu(QWidget *parent);

        //! Create all tool bars.
        void createToolBars();

        //! Create tool bar.
        QToolBar *createToolBar(const QString &name, const QString &title);

        //! Add drop-down button with given actions to given tool bar.
        void addToolBarSplitButton(QToolBar *toolBar, const QString &name, const QList<QAction*> &actions, bool rememberLastUsed);

        //! Place tool bars to their default positions.
        void placeToolBars();

        //! Create ribbon.
        void createRibbon();

        //! Show given tab of the panel with problem setup.
        void showSolverDockTab(QWidget *tab);

        //! Create status bar.
        void createStatusBar();

        //! Create central widget.
        void createCentralWidget();

        //! Create model tab.
        QWidget *createModelTab();

        //! Create document tab.
        QWidget *createDocumentTab();

        //! Create records tab.
        QWidget *createRecordsTab();

        //! Create problem setup tab.
        QWidget *createProblemTab();

        //! Create boundary condition tab.
        QWidget *createBcTab();

        //! Create initial condition tab.
        QWidget *createIcTab();

        //! Create environmant condition tab.
        QWidget *createEcTab();

        //! Create material tab.
        QWidget *createMaterialTab();

        //! Create results tab.
        QWidget *createResultsTab();

        //! Create custom popup menu.
        QMenu * createPopupMenu ();

        //! Apply shortcuts to actions.
        void applyShortcutsToActions();

        //! Read application settings.
        void readSettings();

        //! Write application settings.
        void writeSettings() const;

        //! Overloaded closeEvent function to catch close event.
        void closeEvent(QCloseEvent *event);

        //! Find first dock widget.
        QDockWidget *findFirstDockWidget(Qt::DockWidgetArea area);

        //! Set progress fraction bar.
        void progressBar(RProgressBar *progressBar, double fraction);

        //! Initialize progress bar.
        void progressBarInitialize(RProgressBar *progressBar, const QString &message, bool pulseType);

        //! Finalize progress bar.
        void progressBarFinalize(RProgressBar *progressBar, const QString &message);

    protected slots:

        //! Enable whole window.
        void setEnabled(bool enabled);

        //! Enable whole window.
        void enable();

        //! Disable whole window.
        void disable();

        //! Set main progress fraction.
        void onMainProgress(double fraction);

        //! Initialize main progress.
        void onMainProgressInitialize(const QString &message, bool pulseType);

        //! Finalize main progress.
        void onMainProgressFinalize(const QString &message);

        //! Draw object was added.
        void onDrawObjectAdded();

        //! Draw object was removed.
        void onDrawObjectRemoved();

        //! Job is blocking.
        void onJobBlocking(bool blocking);

        //! New job has started.
        void onJobStarted();

        //! Job has ended.
        void onJobEnded();

        //! Solver job has started, finished or failed.
        void onSolverJobStateChanged();

        //! New model has been added.
        void onModelAdded(uint position);

        //! Model has been removed.
        void onModelRemoved(uint position);

        //! Model has been changed.
        void onModelChanged(uint position);

        //! Problem has been changed.
        void onProblemChanged(uint);

        //! Results has been changed.
        void onResultsChanged(uint);

        //! Model visibility has changed.
        void onEntityVisibilityChanged(uint modelID, REntityGroupType, uint, bool visible);

        //! Finalize geometry transform widget.
        void onGeometryTransformFinalize();

        //! Called when number of history records has changed.
        void onNHistoryRecordsChanged(uint);

        //! Toolbar icon size has changed.
        void onToolbarIconSizeChanged(int toolbarIconSize);

};

#endif /* MAIN_WINDOW_H */
