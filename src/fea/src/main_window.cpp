#include <QtGui/QCloseEvent>
#include <QMenuBar>
#include <QMenu>
#include <QGridLayout>
#include <QStatusBar>
#include <QSignalBlocker>
#include <QSplitter>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

#include <rbl_error.h>
#include <rbl_job_manager.h>

#include <rgl_first_run_dialog.h>
#include <rgl_progress_handler.h>
#include <rgl_message_box.h>
#include <rgl_ribbon_bar.h>
#include <rgl_split_tool_button.h>

#include "action.h"
#include "application.h"
#include "bc_manager_tree.h"
#include "bc_tree.h"
#include "document_tree.h"
#include "draw_input_widget.h"
#include "ic_manager_tree.h"
#include "ic_tree.h"
#include "ec_manager_tree.h"
#include "ec_tree.h"
#include "material_manager_tree.h"
#include "material_tree.h"
#include "model_entity_display_properties_tree.h"
#include "model_entity_geometric_properties_tree.h"
#include "model_records_selector.h"
#include "problem_tree.h"
#include "problem_task_dialog.h"
#include "solver_manager.h"
#include "results_variable_selector.h"
#include "results_variable_tree.h"
#include "main_window.h"
#include "session.h"
#include "geometry_transform_widget.h"
#include "action_list.h"
#include "sub_window_manager.h"
#include "model_tree.h"
#include <rgl_icon.h>

//! Version of tool bar arrangement. Increase it when tool bars change, to place them anew for existing users.
static constexpr int toolBarsVersion = 3;

MainWindow::MainWindow(Session *session,
                       RCloudSessionManager *cloudSession,
                       RApplicationSettings *applicationSettings,
                       ActionList *actionList,
                       QWidget *parent)
    : QMainWindow{parent}
    , session{session}
    , cloudSession{cloudSession}
    , applicationSettings{applicationSettings}
    , actionList{actionList}
    , isFirstRun{true}
{
    R_LOG_TRACE_IN;
    if (this->objectName().isEmpty())
    {
        this->setObjectName(QString::fromUtf8("MainWindow"));
    }

    this->readSettings();

    this->applyShortcutsToActions();

    this->setWindowTitle(RVendor::name() + " :: " + tr("Finite Element Analysis"));
    this->setWindowIcon(RIcon::fromFile(Application::windowIconFile()));

    int toolbarIconSize = this->applicationSettings->getToolbarIconSize();
    this->setIconSize(QSize(toolbarIconSize,toolbarIconSize));
    this->setToolButtonStyle(Qt::ToolButtonIconOnly);

    this->setDockOptions(QMainWindow::ForceTabbedDocks);

    this->createMenus();
    this->createToolBars();
    this->createRibbon();
    this->createStatusBar();

    // Shortcut of an action works only while a widget holding the action is visible,
    // which buttons on other pages of the ribbon are not.
    for (uint i=0;i<this->actionList->getNActions();i++)
    {
        RAction *action = this->actionList->getAction(i);
        if (action && !action->isSeparator())
        {
            this->addAction(action);
        }
    }

    this->createCentralWidget();

    QDockWidget *modelDock = new QDockWidget(this);
    modelDock->setObjectName(QString::fromUtf8("modelDock"));
    modelDock->setWindowTitle(tr("Models"));
    modelDock->setTitleBarWidget(new QWidget(modelDock));
    this->addDockWidget(Qt::LeftDockWidgetArea, modelDock);

    QTabWidget *modelDockTabWidget = new QTabWidget;
    modelDockTabWidget->setTabPosition(QTabWidget::West);
    modelDock->setWidget(modelDockTabWidget);

    this->modelsTab = this->createModelTab();
    modelDockTabWidget->addTab(this->modelsTab,tr("Model"));

    this->documentsTab = this->createDocumentTab();
    modelDockTabWidget->addTab(this->documentsTab,tr("Documents"));

    this->recordsTab = this->createRecordsTab();
    modelDockTabWidget->addTab(this->recordsTab,tr("Records"));

    ModelRecordsSelector *recordsSelector = static_cast<ModelRecordsSelector*>(this->recordsTab);
    auto updateRecordsTabVisibility = [this,modelDockTabWidget,recordsSelector]()
    {
        modelDockTabWidget->setTabVisible(modelDockTabWidget->indexOf(this->recordsTab),recordsSelector->hasRecords());
    };
    QObject::connect(recordsSelector,&ModelRecordsSelector::recordCountChanged,this,updateRecordsTabVisibility);
    updateRecordsTabVisibility();

    QDockWidget *solverDock = new QDockWidget(this);
    solverDock->setObjectName(QString::fromUtf8("solverDock"));
    solverDock->setWindowTitle(tr("Solver"));
    solverDock->setTitleBarWidget(new QWidget(solverDock));
    this->addDockWidget(Qt::RightDockWidgetArea, solverDock);

    this->solverDockTabWidget = new QTabWidget;
    this->solverDockTabWidget->setTabPosition(QTabWidget::East);
    solverDock->setWidget(this->solverDockTabWidget);

    this->problemTab = this->createProblemTab();
    this->solverDockTabWidget->addTab(this->problemTab,tr("Problem"));

    this->bcTab = this->createBcTab();
    this->solverDockTabWidget->addTab(this->bcTab,tr("Boundary conditions"));

    this->icTab = this->createIcTab();
    this->solverDockTabWidget->addTab(this->icTab,tr("Initial conditions"));

    this->ecTab = this->createEcTab();
    this->solverDockTabWidget->addTab(this->ecTab,tr("Environment conditions"));

    this->materialTab = this->createMaterialTab();
    this->solverDockTabWidget->addTab(this->materialTab,tr("Material"));

    this->resultsTab = this->createResultsTab();
    this->solverDockTabWidget->addTab(this->resultsTab,tr("Results"));

    if (this->isFirstRun || !this->restoreGeometry(this->applicationSettings->value("mainWindow/geometry").toByteArray()))
    {
        // First run of this version or no stored geometry: use 70 % of the screen and center the window.
        if (QScreen *screen = QGuiApplication::primaryScreen())
        {
            QRect screenRect = screen->availableGeometry();
            QSize windowSize(qRound(screenRect.width() * 0.7), qRound(screenRect.height() * 0.7));
            this->resize(windowSize);
            this->move(screenRect.center() - QPoint(windowSize.width() / 2, windowSize.height() / 2));
        }
    }
    this->restoreState(this->applicationSettings->value("mainWindow/windowState").toByteArray());

    // Stored state knows only tool bars of the time it was saved. Docks are left as they were.
    if (this->applicationSettings->value("mainWindow/toolBarsVersion",1).toInt() != toolBarsVersion)
    {
        this->placeToolBars();
        this->applicationSettings->setValue("mainWindow/toolBarsVersion",toolBarsVersion);
    }

    for (RSplitToolButton *button : std::as_const(this->toolBarSplitButtons))
    {
        button->setLastUsedPosition(this->applicationSettings->value("mainWindow/" + button->objectName(),0).toInt());
    }

    // Must follow restoring of the state, which brings back visibility of tool bars.
    const bool ribbonLayout = this->applicationSettings->value("mainWindow/ribbonLayout",false).toBool();
    {
        const QSignalBlocker ribbonLayoutActionBlocker(this->actionList->getAction(Action::ACTION_APPLICATION_RIBBON_LAYOUT));
        this->actionList->getAction(Action::ACTION_APPLICATION_RIBBON_LAYOUT)->setChecked(ribbonLayout);
    }
    this->setRibbonLayout(ribbonLayout);

    const bool ribbonCompact = this->applicationSettings->value("mainWindow/ribbonCompact",false).toBool();
    {
        const QSignalBlocker ribbonCompactActionBlocker(this->actionList->getAction(Action::ACTION_APPLICATION_RIBBON_COMPACT));
        this->actionList->getAction(Action::ACTION_APPLICATION_RIBBON_COMPACT)->setChecked(ribbonCompact);
    }
    this->ribbonBar->setCompact(ribbonCompact);

    this->ribbonBar->setCollapsed(this->applicationSettings->value("mainWindow/ribbonCollapsed",false).toBool());
    this->ribbonBar->setCurrentPage(this->applicationSettings->value("mainWindow/ribbonPage",0).toInt());

    // Toolbar icon size changed signal
    QObject::connect(this->applicationSettings,&RApplicationSettings::toolbarIconSizeChanged,this,&MainWindow::onToolbarIconSizeChanged);

    // Main progress signals
    QObject::connect(&RProgressHandler::getInstance(),&RProgressHandler::progress,this,&MainWindow::onMainProgress);
    QObject::connect(&RProgressHandler::getInstance(),&RProgressHandler::progressInitialize,this,&MainWindow::onMainProgressInitialize);
    QObject::connect(&RProgressHandler::getInstance(),&RProgressHandler::progressFinalize,this,&MainWindow::onMainProgressFinalize);

    // Solver manager signals.
    QObject::connect(&SolverManager::getInstance(),&SolverManager::jobBlocking,this,&MainWindow::onJobBlocking);
    QObject::connect(&SolverManager::getInstance(),&SolverManager::jobStarted,this,&MainWindow::onSolverJobStateChanged);
    QObject::connect(&SolverManager::getInstance(),&SolverManager::jobFinished,this,&MainWindow::onSolverJobStateChanged);
    QObject::connect(&SolverManager::getInstance(),&SolverManager::jobFailed,this,&MainWindow::onSolverJobStateChanged);

    // Draw engine signals.
    QObject::connect(this->session->getDrawEngine(),&DrawEngine::objectAdded,this,&MainWindow::onDrawObjectAdded);
    QObject::connect(this->session->getDrawEngine(),&DrawEngine::objectRemoved,this,&MainWindow::onDrawObjectRemoved);
    QObject::connect(this->session->getDrawEngine(),&DrawEngine::objectsRemoved,this,&MainWindow::onDrawObjectRemoved);

    // Job manager signals
    QObject::connect(&RJobManager::getInstance(),&RJobManager::jobBlocking,this,&MainWindow::onJobBlocking);
    QObject::connect(&RJobManager::getInstance(),&RJobManager::jobStarted,this,&MainWindow::onJobStarted);
    QObject::connect(&RJobManager::getInstance(),&RJobManager::jobFinished,this,&MainWindow::onJobEnded);

    // Connect model modification signals
    QObject::connect(this->session,&Session::modelAdded,this,&MainWindow::onModelAdded);
    QObject::connect(this->session,&Session::modelRemoved,this,&MainWindow::onModelRemoved);
    QObject::connect(this->session,&Session::modelChanged,this,&MainWindow::onModelChanged);

    // Connect problem modification signals
    QObject::connect(this->session,&Session::problemChanged,this,&MainWindow::onProblemChanged);

    // Connect results modification signals
    QObject::connect(this->session,&Session::resultsChanged,this,&MainWindow::onResultsChanged);

    // Model selection signals
    QObject::connect(this->session,&Session::entityVisibilityChanged,this,&MainWindow::onEntityVisibilityChanged);

    // Number of history records has changed.
    QObject::connect(this->session,&Session::nHistoryRecordsChanged,this,&MainWindow::onNHistoryRecordsChanged);

    if (this->isFirstRun)
    {
        // Dialog executed before the main window is shown makes macOS discard the size set above.
        QTimer::singleShot(0,this,[this]()
        {
            RFirstRunDialog(this).exec();
        });
    }

    R_LOG_TRACE_OUT;
}

void MainWindow::showTransformGeometryWidget()
{
    R_LOG_TRACE;
    QString dockName = QString::fromUtf8("dockTransform");
    GeometryTransformWidget *transformGeometryWidget = nullptr;

    if (!this->findCustomDockWidget(dockName))
    {
        transformGeometryWidget = new GeometryTransformWidget;

        QObject::connect(transformGeometryWidget,
                         &GeometryTransformWidget::finalize,
                         this,
                         &MainWindow::onGeometryTransformFinalize);
    }
    this->showCustomDockWidget(transformGeometryWidget,dockName,tr("Transform geometry"));
}

void MainWindow::hideTransformGeometryWidget()
{
    R_LOG_TRACE;
    this->hideCustomDockWidget(QString::fromUtf8("dockTransform"));
}

void MainWindow::showDrawWidget()
{
    R_LOG_TRACE;
    QString dockName = QString::fromUtf8("dockDraw");
    DrawInputWidget *drawInputWidget = nullptr;

    if (!this->findCustomDockWidget(dockName))
    {
        drawInputWidget = new DrawInputWidget;
    }
    this->showCustomDockWidget(drawInputWidget,dockName,tr("Draw object"));
}

void MainWindow::hideDrawWidget()
{
    R_LOG_TRACE;
    this->hideCustomDockWidget(QString::fromUtf8("dockDraw"));
}

void MainWindow::progressAutoHideEnable()
{
    R_LOG_TRACE;
    this->mainProgressBar->setAutoHide(true);
}

void MainWindow::progressAutoHideDisable()
{
    R_LOG_TRACE;
    this->mainProgressBar->setAutoHide(false);
}

QDockWidget *MainWindow::findCustomDockWidget(const QString &name)
{
    R_LOG_TRACE;
    return this->findChild<QDockWidget*>(name);
}

void MainWindow::showCustomDockWidget(QWidget *widget, const QString &name, const QString &title)
{
    R_LOG_TRACE;
    QDockWidget *dockWidget = this->findCustomDockWidget(name);

    if (!dockWidget)
    {
        QWidget *layoutWidget;
        QGridLayout *gridLayout;

        layoutWidget = new QWidget();

        gridLayout = new QGridLayout(layoutWidget);

        gridLayout->addWidget(widget);

        dockWidget = new QDockWidget(this);
        dockWidget->setObjectName(name);
        dockWidget->setWindowTitle(title);
        dockWidget->setWidget(layoutWidget);

        this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);

        QDockWidget *firstDockWidget = this->findFirstDockWidget(Qt::RightDockWidgetArea);
        if (firstDockWidget)
        {
            this->tabifyDockWidget(firstDockWidget,dockWidget);
        }
    }

    dockWidget->show();
    dockWidget->raise();
}

void MainWindow::hideCustomDockWidget(const QString &name)
{
    R_LOG_TRACE;
    QDockWidget *dockWidget = this->findCustomDockWidget(name);
    if (dockWidget)
    {
        delete dockWidget;
    }
}

QMenu *MainWindow::createFileMenu(QWidget *parent)
{
    R_LOG_TRACE_IN;
    QMenu *menuFile = new QMenu(parent);
    menuFile->setTitle(tr("File"));
    QMenu *menuFileSession = new QMenu(menuFile);
    menuFileSession->setTitle(tr("Session"));
    menuFileSession->setIcon(RIcon::fromFile(":/icons/file/pixmaps/range-session.svg"));
    menuFileSession->addAction(this->actionList->getAction(Action::ACTION_SESSION_NEW));
    menuFileSession->addAction(this->actionList->getAction(Action::ACTION_SESSION_OPEN));
    menuFileSession->addAction(this->actionList->getAction(Action::ACTION_SESSION_SAVE));
    menuFileSession->addAction(this->actionList->getAction(Action::ACTION_SESSION_SAVE_AS));
    menuFileSession->addAction(this->actionList->getAction(Action::ACTION_SESSION_CLOSE));
    menuFile->addAction(menuFileSession->menuAction());
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_NEW));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_OPEN));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_SAVE));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_SAVE_AS));
    QMenu *menuFileExport = new QMenu(menuFile);
    menuFileExport->setTitle(tr("Export"));
    menuFileExport->setIcon(RIcon::fromFile(":/icons/file/pixmaps/range-model_save_as.svg"));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_MSH));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_RAW));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_STL_ASCII));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_STL_BINARY));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_STEP));
    menuFile->addAction(menuFileExport->menuAction());
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_RENAME));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_CLOSE));
    menuFile->addSeparator();
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_RELOAD_RESULTS));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_DROP_RESULTS));
    menuFile->addSeparator();
    QMenu *menuFileFolders = new QMenu(menuFile);
    menuFileFolders->setTitle(tr("Folders"));
    menuFileFolders->setIcon(RIcon::fromFile(":/icons/file/pixmaps/range-open.svg"));
    menuFileFolders->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_FOLDER_CERT));
    menuFileFolders->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_FOLDER_DATA));
    menuFileFolders->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_FOLDER_DOC));
    menuFileFolders->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_FOLDER_LOG));
    menuFileFolders->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_FOLDER_MATERIALS));
    menuFileFolders->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_FOLDER_SESSION));
    menuFileFolders->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_FOLDER_TMP));
    menuFile->addAction(menuFileFolders->menuAction());
    menuFile->addSeparator();
    menuFile->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_RIBBON_LAYOUT));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_RIBBON_COMPACT));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_SETTINGS));
    menuFile->addSeparator();
    menuFile->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_QUIT));
    R_LOG_TRACE_OUT;
    return menuFile;
}

QMenu *MainWindow::createSpecialToolsMenu(QWidget *parent)
{
    R_LOG_TRACE_IN;
    QMenu *menuGeometryDevelopement = new QMenu(parent);
    menuGeometryDevelopement->setTitle(tr("Special tools"));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_EXPORT_SLIVER_ELEMENTS));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_EXPORT_INTERSECTED_ELEMENTS));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_PURGE_UNUSED_NODES));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_PURGE_UNUSED_ELEMENTS));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_NODES));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_ELEMENTS));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_POINT_INSIDE_SURFACE));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_TETRAHEDRALIZE_SURFACE));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_CONSOLIDATE));
    R_LOG_TRACE_OUT;
    return menuGeometryDevelopement;
}

QMenu *MainWindow::createCloudMenu(QWidget *parent)
{
    R_LOG_TRACE_IN;
    QMenu *menuCloud = new QMenu(parent);
    menuCloud->setTitle(tr("Cloud"));
    menuCloud->addAction(this->actionList->getAction(Action::ACTION_CLOUD_SESSION_MANAGER));
    menuCloud->addAction(this->actionList->getAction(Action::ACTION_CLOUD_FILE_MANAGER));
    R_LOG_TRACE_OUT;
    return menuCloud;
}

QMenu *MainWindow::createHelpMenu(QWidget *parent)
{
    R_LOG_TRACE_IN;
    QMenu *menuHelp = new QMenu(parent);
    menuHelp->setTitle(tr("Help"));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_HELP));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_UPDATE));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_ABOUT));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_ABOUT_QT));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_LICENSE));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_PRIVACY_POLICY));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_RELEASE_NOTES));
    R_LOG_TRACE_OUT;
    return menuHelp;
}

void MainWindow::createMenus()
{
    R_LOG_TRACE_IN;
    // Menu bar and ribbon share the place above the tool bars, only one of them is visible at a time.
    QWidget *menuWidget = new QWidget(this);
    QVBoxLayout *menuWidgetLayout = new QVBoxLayout(menuWidget);
    menuWidgetLayout->setContentsMargins(0,0,0,0);
    menuWidgetLayout->setSpacing(0);
    this->setMenuWidget(menuWidget);

    QMenuBar *menuBar = new QMenuBar(menuWidget);
    menuBar->setNativeMenuBar(!this->applicationSettings->getDontUseNativeMenuBar());
    menuWidgetLayout->addWidget(menuBar);
    this->classicMenuBar = menuBar;

    menuBar->addAction(this->createFileMenu(menuBar)->menuAction());

    QMenu *menuGeometry = new QMenu(menuBar);
    menuGeometry->setTitle(tr("Geometry"));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_UNDO));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REDO));
    menuGeometry->addSeparator();
    QMenu *menuGeometryDraw = new QMenu(menuGeometry);
    menuGeometryDraw->setTitle(tr("Draw"));
    menuGeometryDraw->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-draw_geometry.svg"));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_POINT));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_LINE));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_TRIANGLE));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_QUADRILATERAL));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_CIRCLE));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_ELLIPSE));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_TETRAHEDRON));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_HEXAHEDRON));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_CYLINDER));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_SPHERE));
    menuGeometryDraw->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_RAW));
    menuGeometry->addAction(menuGeometryDraw->menuAction());
    menuGeometry->addSeparator();
//    QMenu *menuGeometryPoint = new QMenu(menuGeometry);
//    menuGeometryPoint->setTitle(tr("Point"));
//    menuGeometryPoint->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_point.svg"));
//    menuGeometry->addAction(menuGeometryPoint->menuAction());
    QMenu *menuGeometryLine = new QMenu(menuGeometry);
    menuGeometryLine->setTitle(tr("Line"));
    menuGeometryLine->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_line.svg"));
    menuGeometryLine->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_LINE_GENERATE_FROM_EDGES));
    menuGeometry->addAction(menuGeometryLine->menuAction());
    QMenu *menuGeometrySurface = new QMenu(menuGeometry);
    menuGeometrySurface->setTitle(tr("Surface"));
    menuGeometrySurface->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_surface.svg"));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_MARK));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SWAP_ELEMENT_NORMAL));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SWAP_NORMALS));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SYNC_NORMALS));
    menuGeometrySurface->addSeparator();
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_CLOSE_HOLE));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_COARSEN));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIND_INTERSECTED_ELEMENTS));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BREAK_INTERSECTED_ELEMENTS));
    QMenu *menuGeometryBoolOperations = new QMenu(menuGeometrySurface);
    menuGeometryBoolOperations->setTitle(tr("Boolean operations"));
    menuGeometryBoolOperations->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_surface_bool_operation.svg"));
    menuGeometryBoolOperations->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_UNION));
    menuGeometryBoolOperations->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_DIFFERENCE));
    menuGeometryBoolOperations->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_INTERSECTION));
    menuGeometrySurface->addAction(menuGeometryBoolOperations->menuAction());
    menuGeometry->addAction(menuGeometrySurface->menuAction());
    QMenu *menuGeometryVolume = new QMenu(menuGeometry);
    menuGeometryVolume->setTitle(tr("Volume"));
    menuGeometryVolume->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_volume.svg"));
    menuGeometryVolume->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_VOLUME_GENERATE_TETRAHEDRA));
    menuGeometry->addAction(menuGeometryVolume->menuAction());
    menuGeometry->addSeparator();
    QMenu *menuGeometryCut = new QMenu(menuGeometry);
    menuGeometryCut->setTitle(tr("Cut"));
    menuGeometryCut->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_cut.svg"));
    menuGeometryCut->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_CUT_CREATE));
    menuGeometryCut->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_CUT_EDIT));
    menuGeometry->addAction(menuGeometryCut->menuAction());
    QMenu *menuGeometryIso = new QMenu(menuGeometry);
    menuGeometryIso->setTitle(tr("Iso"));
    menuGeometryIso->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_iso.svg"));
    menuGeometryIso->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ISO_CREATE));
    menuGeometryIso->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ISO_EDIT));
    menuGeometry->addAction(menuGeometryIso->menuAction());
    QMenu *menuGeometryStreamLine = new QMenu(menuGeometry);
    menuGeometryStreamLine->setTitle(tr("Stream line"));
    menuGeometryStreamLine->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_stream_line.svg"));
    menuGeometryStreamLine->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_STREAM_LINE_CREATE));
    menuGeometryStreamLine->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_STREAM_LINE_EDIT));
    menuGeometry->addAction(menuGeometryStreamLine->menuAction());
    QMenu *menuGeometryScalarField = new QMenu(menuGeometry);
    menuGeometryScalarField->setTitle(tr("Scalar field"));
    menuGeometryScalarField->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_scalar_field.svg"));
    menuGeometryScalarField->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SCALAR_FIELD_CREATE));
    menuGeometryScalarField->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SCALAR_FIELD_EDIT));
    menuGeometry->addAction(menuGeometryScalarField->menuAction());
    QMenu *menuGeometryVectorField = new QMenu(menuGeometry);
    menuGeometryVectorField->setTitle(tr("Vector field"));
    menuGeometryVectorField->setIcon(RIcon::fromFile(":/icons/geometry/pixmaps/range-entity_vector_field.svg"));
    menuGeometryVectorField->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_VECTOR_FIELD_CREATE));
    menuGeometryVectorField->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_VECTOR_FIELD_EDIT));
    menuGeometry->addAction(menuGeometryVectorField->menuAction());
    menuGeometry->addSeparator();
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ENTITY_MERGE));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ENTITY_REMOVE));
    menuGeometry->addSeparator();
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIND_SLIVER_ELEMENTS));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIX_SLIVER_ELEMENTS));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_MERGE_NEAR_NODES));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_MERGE_NODES));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_MOVE_NODE));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REMOVE_NODE));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REMOVE_NODE_AND_CLOSE_HOLE));
    menuGeometry->addSeparator();
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_CREATE_ELEMENT));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REMOVE_ELEMENT));
    menuGeometry->addSeparator();
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_TRANSFORM));
    menuGeometry->addSeparator();
    menuGeometry->addAction(this->createSpecialToolsMenu(menuGeometry)->menuAction());
    menuBar->addAction(menuGeometry->menuAction());

    QMenu *menuProblem = new QMenu(menuBar);
    menuProblem->setTitle(tr("Problem"));
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_TASK_FLOW));
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_SOLVER_SETUP));
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_DEFINE_MONITORING_POINTS));
    menuProblem->addSeparator();
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_RESET));
    menuBar->addAction(menuProblem->menuAction());

    QMenu *menuSolver = new QMenu(menuBar);
    menuSolver->setTitle(tr("Solution"));
    menuSolver->addAction(this->actionList->getAction(Action::ACTION_SOLVER_START));
    menuSolver->addAction(this->actionList->getAction(Action::ACTION_SOLVER_STOP));
    menuSolver->addAction(this->actionList->getAction(Action::ACTION_SOLVER_KILL));
    menuSolver->addSeparator();
    menuBar->addAction(menuSolver->menuAction());

    QMenu *menuReport = new QMenu(menuBar);
    menuReport->setTitle(tr("Report"));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_MODEL_STATISTICS));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_SOLVER_LOG));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_CONVERGENCE_GRAPH));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_MONITORING_POINT_GRAPH));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_CREATE_REPORT));
    menuBar->addAction(menuReport->menuAction());

    menuBar->addAction(this->createCloudMenu(menuBar)->menuAction());
    menuBar->addAction(this->createHelpMenu(menuBar)->menuAction());
    R_LOG_TRACE_OUT;
}

void MainWindow::createToolBars()
{
    R_LOG_TRACE_IN;

    auto action = [this](Action::Type type) -> QAction* { return this->actionList->getAction(type); };

    QToolBar *fileToolBar = this->createToolBar("mainToolBar",tr("File and edit toolbar"));
    fileToolBar->addAction(action(Action::ACTION_MODEL_NEW));
    fileToolBar->addAction(action(Action::ACTION_MODEL_OPEN));
    fileToolBar->addAction(action(Action::ACTION_MODEL_SAVE));
    fileToolBar->addSeparator();
    fileToolBar->addAction(action(Action::ACTION_GEOMETRY_UNDO));
    fileToolBar->addAction(action(Action::ACTION_GEOMETRY_REDO));

    QToolBar *workflowToolBar = this->createToolBar("workflowToolBar",tr("Workflow toolbar"));
    workflowToolBar->addAction(action(Action::ACTION_GEOMETRY_VOLUME_GENERATE_TETRAHEDRA));
    workflowToolBar->addAction(action(Action::ACTION_PROBLEM_TASK_FLOW));
    workflowToolBar->addSeparator();
    workflowToolBar->addAction(action(Action::ACTION_SOLVER_START));
    this->addToolBarSplitButton(workflowToolBar,"solverStopSplitButton",{action(Action::ACTION_SOLVER_STOP),action(Action::ACTION_SOLVER_KILL)},false);
    workflowToolBar->addSeparator();
    workflowToolBar->addAction(action(Action::ACTION_REPORT_CONVERGENCE_GRAPH));
    workflowToolBar->addAction(action(Action::ACTION_MODEL_RELOAD_RESULTS));
    workflowToolBar->addAction(action(Action::ACTION_REPORT_CREATE_REPORT));

    QToolBar *applicationToolBar = this->createToolBar("applicationToolBar",tr("Application toolbar"));

    // Last tool bar of a row takes the remaining width, this pushes its buttons to the right edge of the window.
    QWidget *spacer = new QWidget(applicationToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    applicationToolBar->addWidget(spacer);

    applicationToolBar->addAction(action(Action::ACTION_CLOUD_FILE_MANAGER));
    applicationToolBar->addSeparator();
    applicationToolBar->addAction(action(Action::ACTION_APPLICATION_RIBBON_LAYOUT));
    applicationToolBar->addAction(action(Action::ACTION_APPLICATION_SETTINGS));
    applicationToolBar->addSeparator();
    applicationToolBar->addAction(action(Action::ACTION_APPLICATION_HELP));

    QToolBar *geometryToolBar = this->createToolBar("geometryToolBar",tr("Geometry toolbar"));
    this->addToolBarSplitButton(geometryToolBar,"drawSplitButton",{action(Action::ACTION_GEOMETRY_DRAW_POINT),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_LINE),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_TRIANGLE),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_QUADRILATERAL),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_CIRCLE),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_ELLIPSE),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_TETRAHEDRON),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_HEXAHEDRON),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_CYLINDER),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_SPHERE),
                                                                   action(Action::ACTION_GEOMETRY_DRAW_RAW)},true);
    this->addToolBarSplitButton(geometryToolBar,"booleanSplitButton",{action(Action::ACTION_GEOMETRY_BOOL_UNION),
                                                                      action(Action::ACTION_GEOMETRY_BOOL_DIFFERENCE),
                                                                      action(Action::ACTION_GEOMETRY_BOOL_INTERSECTION)},true);
    geometryToolBar->addAction(action(Action::ACTION_GEOMETRY_TRANSFORM));

    QToolBar *meshToolBar = this->createToolBar("meshToolBar",tr("Mesh toolbar"));
    meshToolBar->addAction(action(Action::ACTION_GEOMETRY_SURFACE_MARK));
    this->addToolBarSplitButton(meshToolBar,"normalsSplitButton",{action(Action::ACTION_GEOMETRY_SURFACE_SWAP_ELEMENT_NORMAL),
                                                                  action(Action::ACTION_GEOMETRY_SURFACE_SWAP_NORMALS),
                                                                  action(Action::ACTION_GEOMETRY_SURFACE_SYNC_NORMALS)},true);
    meshToolBar->addAction(action(Action::ACTION_GEOMETRY_SURFACE_CLOSE_HOLE));
    meshToolBar->addAction(action(Action::ACTION_GEOMETRY_SURFACE_COARSEN));
    meshToolBar->addSeparator();
    // Finding has to come before repairing, so these always start with finding.
    this->addToolBarSplitButton(meshToolBar,"intersectedSplitButton",{action(Action::ACTION_GEOMETRY_FIND_INTERSECTED_ELEMENTS),
                                                                      action(Action::ACTION_GEOMETRY_BREAK_INTERSECTED_ELEMENTS)},false);
    this->addToolBarSplitButton(meshToolBar,"sliversSplitButton",{action(Action::ACTION_GEOMETRY_FIND_SLIVER_ELEMENTS),
                                                                  action(Action::ACTION_GEOMETRY_FIX_SLIVER_ELEMENTS)},false);

    QToolBar *resultsToolBar = this->createToolBar("resultsToolBar",tr("Results toolbar"));
    this->addToolBarSplitButton(resultsToolBar,"cutSplitButton",{action(Action::ACTION_GEOMETRY_CUT_CREATE),action(Action::ACTION_GEOMETRY_CUT_EDIT)},false);
    this->addToolBarSplitButton(resultsToolBar,"isoSplitButton",{action(Action::ACTION_GEOMETRY_ISO_CREATE),action(Action::ACTION_GEOMETRY_ISO_EDIT)},false);
    this->addToolBarSplitButton(resultsToolBar,"streamLineSplitButton",{action(Action::ACTION_GEOMETRY_STREAM_LINE_CREATE),action(Action::ACTION_GEOMETRY_STREAM_LINE_EDIT)},false);
    this->addToolBarSplitButton(resultsToolBar,"scalarFieldSplitButton",{action(Action::ACTION_GEOMETRY_SCALAR_FIELD_CREATE),action(Action::ACTION_GEOMETRY_SCALAR_FIELD_EDIT)},false);
    this->addToolBarSplitButton(resultsToolBar,"vectorFieldSplitButton",{action(Action::ACTION_GEOMETRY_VECTOR_FIELD_CREATE),action(Action::ACTION_GEOMETRY_VECTOR_FIELD_EDIT)},false);

    // Tool bars are placed in this order.
    this->toolBars = { fileToolBar, geometryToolBar, meshToolBar, workflowToolBar, resultsToolBar, applicationToolBar };
    this->placeToolBars();

    R_LOG_TRACE_OUT;
}

QToolBar *MainWindow::createToolBar(const QString &name, const QString &title)
{
    R_LOG_TRACE;
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setObjectName(name);
    toolBar->setWindowTitle(title);
    return toolBar;
}

void MainWindow::addToolBarSplitButton(QToolBar *toolBar, const QString &name, const QList<QAction*> &actions, bool rememberLastUsed)
{
    R_LOG_TRACE_IN;
    RSplitToolButton *button = new RSplitToolButton(actions,rememberLastUsed,toolBar);
    button->setObjectName(name);
    button->setIconSize(toolBar->iconSize());
    // Tool bar keeps in sync only buttons it has created itself.
    QObject::connect(toolBar,&QToolBar::iconSizeChanged,button,&QToolButton::setIconSize);
    toolBar->addWidget(button);
    this->toolBarSplitButtons.append(button);
    R_LOG_TRACE_OUT;
}

void MainWindow::placeToolBars()
{
    R_LOG_TRACE_IN;
    for (QToolBar *toolBar : std::as_const(this->toolBars))
    {
        this->removeToolBar(toolBar);
    }
    for (QToolBar *toolBar : std::as_const(this->toolBars))
    {
        this->addToolBar(Qt::TopToolBarArea,toolBar);
        // Removing a tool bar hides it.
        toolBar->show();
    }
    R_LOG_TRACE_OUT;
}

void MainWindow::createRibbon()
{
    R_LOG_TRACE_IN;
    this->ribbonBar = new RRibbonBar(this->menuWidget());
    this->menuWidget()->layout()->addWidget(this->ribbonBar);
    this->ribbonBar->setIconSize(this->applicationSettings->getToolbarIconSize());

    auto action = [this](Action::Type type) -> QAction* { return this->actionList->getAction(type); };

    // Line with tabs
    this->ribbonBar->addLeadingMenu(tr("File"),this->createFileMenu(this->ribbonBar));
    this->ribbonBar->addLeadingAction(action(Action::ACTION_MODEL_SAVE));
    this->ribbonBar->addLeadingAction(action(Action::ACTION_GEOMETRY_UNDO));
    this->ribbonBar->addLeadingAction(action(Action::ACTION_GEOMETRY_REDO));

    this->ribbonBar->addTrailingMenu(tr("Cloud"),action(Action::ACTION_CLOUD_FILE_MANAGER)->icon(),this->createCloudMenu(this->ribbonBar));
    this->ribbonBar->addTrailingAction(action(Action::ACTION_APPLICATION_RIBBON_LAYOUT));
    this->ribbonBar->addTrailingAction(action(Action::ACTION_APPLICATION_RIBBON_COMPACT));
    this->ribbonBar->addTrailingAction(action(Action::ACTION_APPLICATION_SETTINGS));
    this->ribbonBar->addTrailingMenu(tr("Help"),action(Action::ACTION_APPLICATION_HELP)->icon(),this->createHelpMenu(this->ribbonBar));

    RRibbonGroup *group = nullptr;

    // Geometry
    const int geometryPage = this->ribbonBar->addPage(tr("Geometry"));

    group = this->ribbonBar->addGroup(geometryPage,tr("Draw"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_POINT),tr("Point"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_LINE),tr("Line"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_TRIANGLE),tr("Triangle"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_QUADRILATERAL),tr("Quadrilateral"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_CIRCLE),tr("Circle"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_ELLIPSE),tr("Ellipse"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_TETRAHEDRON),tr("Tetrahedron"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_HEXAHEDRON),tr("Hexahedron"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_CYLINDER),tr("Cylinder"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_SPHERE),tr("Sphere"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_DRAW_RAW),tr("RAW object"));

    group = this->ribbonBar->addGroup(geometryPage,tr("Boolean"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_BOOL_UNION));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_BOOL_DIFFERENCE));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_BOOL_INTERSECTION));

    group = this->ribbonBar->addGroup(geometryPage,tr("Modify"));
    group->addLargeAction(action(Action::ACTION_GEOMETRY_TRANSFORM),tr("Transform"));

    group = this->ribbonBar->addGroup(geometryPage,tr("Entities"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_LINE_GENERATE_FROM_EDGES),tr("Line from edges"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_ENTITY_MERGE),tr("Merge"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_ENTITY_REMOVE),tr("Remove"));

    // Mesh
    const int meshPage = this->ribbonBar->addPage(tr("Mesh"));

    group = this->ribbonBar->addGroup(meshPage,tr("Generate"));
    group->addLargeAction(action(Action::ACTION_GEOMETRY_VOLUME_GENERATE_TETRAHEDRA),tr("Tetrahedra"));
    group->addLargeAction(action(Action::ACTION_GEOMETRY_SURFACE_COARSEN),tr("Coarsen"));

    group = this->ribbonBar->addGroup(meshPage,tr("Surface"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_SURFACE_MARK),tr("Mark"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_SURFACE_SWAP_ELEMENT_NORMAL));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_SURFACE_SWAP_NORMALS));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_SURFACE_SYNC_NORMALS),tr("Sync normals"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_SURFACE_CLOSE_HOLE));

    group = this->ribbonBar->addGroup(meshPage,tr("Check and repair"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_FIND_INTERSECTED_ELEMENTS),tr("Find intersected"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_BREAK_INTERSECTED_ELEMENTS),tr("Break intersected"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_FIND_SLIVER_ELEMENTS),tr("Find slivers"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_FIX_SLIVER_ELEMENTS),tr("Fix slivers"));

    group = this->ribbonBar->addGroup(meshPage,tr("Nodes"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_MERGE_NEAR_NODES),tr("Merge near"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_MERGE_NODES),tr("Merge"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_MOVE_NODE),tr("Move"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_REMOVE_NODE),tr("Remove"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_REMOVE_NODE_AND_CLOSE_HOLE),tr("Remove and close hole"));

    group = this->ribbonBar->addGroup(meshPage,tr("Elements"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_CREATE_ELEMENT),tr("Create"));
    group->addSmallAction(action(Action::ACTION_GEOMETRY_REMOVE_ELEMENT),tr("Remove"));
    group->addSmallMenu(tr("Special tools"),QIcon(),this->createSpecialToolsMenu(this->ribbonBar));

    // Problem
    const int problemPage = this->ribbonBar->addPage(tr("Problem"));

    group = this->ribbonBar->addGroup(problemPage,tr("Setup"));
    group->addLargeAction(action(Action::ACTION_PROBLEM_TASK_FLOW));
    group->addLargeAction(action(Action::ACTION_PROBLEM_SOLVER_SETUP),tr("Matrix solver"));
    group->addLargeAction(action(Action::ACTION_PROBLEM_DEFINE_MONITORING_POINTS),tr("Monitoring points"));

    auto addSolverDockTabAction = [this](RRibbonGroup *group, const QString &iconFile, const QString &label, const QString &toolTip, QWidget *MainWindow::*tab)
    {
        QAction *tabAction = new QAction(RIcon::fromFile(iconFile),label,this->ribbonBar);
        tabAction->setToolTip(toolTip);
        QObject::connect(tabAction,&QAction::triggered,this,[this,tab]() { this->showSolverDockTab(this->*tab); });
        group->addLargeAction(tabAction);
    };

    group = this->ribbonBar->addGroup(problemPage,tr("Conditions"));
    addSolverDockTabAction(group,":/icons/application/pixmaps/range-condition_boundary.svg",tr("Boundary"),tr("Show boundary conditions."),&MainWindow::bcTab);
    addSolverDockTabAction(group,":/icons/application/pixmaps/range-condition_initial.svg",tr("Initial"),tr("Show initial conditions."),&MainWindow::icTab);
    addSolverDockTabAction(group,":/icons/application/pixmaps/range-condition_environment.svg",tr("Environment"),tr("Show environment conditions."),&MainWindow::ecTab);
    addSolverDockTabAction(group,":/icons/application/pixmaps/range-condition_material.svg",tr("Material"),tr("Show materials."),&MainWindow::materialTab);

    group = this->ribbonBar->addGroup(problemPage,tr("Reset"));
    group->addLargeAction(action(Action::ACTION_PROBLEM_RESET));

    // Solve
    const int solvePage = this->ribbonBar->addPage(tr("Solve"));

    group = this->ribbonBar->addGroup(solvePage,tr("Run"));
    group->addLargeAction(action(Action::ACTION_SOLVER_START),tr("Start"));
    group->addLargeAction(action(Action::ACTION_SOLVER_STOP),tr("Stop"));
    group->addLargeAction(action(Action::ACTION_SOLVER_KILL),tr("Kill"));

    group = this->ribbonBar->addGroup(solvePage,tr("Monitor"));
    group->addLargeAction(action(Action::ACTION_REPORT_CONVERGENCE_GRAPH),tr("Convergence"));
    group->addLargeAction(action(Action::ACTION_REPORT_SOLVER_LOG),tr("Solver log"));

    group = this->ribbonBar->addGroup(solvePage,tr("Data"));
    group->addLargeAction(action(Action::ACTION_MODEL_RELOAD_RESULTS));
    group->addLargeAction(action(Action::ACTION_MODEL_DROP_RESULTS));

    // Results
    const int resultsPage = this->ribbonBar->addPage(tr("Results"));

    group = this->ribbonBar->addGroup(resultsPage,tr("Visualize"));
    group->addLargeSplitAction(action(Action::ACTION_GEOMETRY_CUT_CREATE),{action(Action::ACTION_GEOMETRY_CUT_CREATE),action(Action::ACTION_GEOMETRY_CUT_EDIT)},tr("Cut"));
    group->addLargeSplitAction(action(Action::ACTION_GEOMETRY_ISO_CREATE),{action(Action::ACTION_GEOMETRY_ISO_CREATE),action(Action::ACTION_GEOMETRY_ISO_EDIT)},tr("Iso"));
    group->addLargeSplitAction(action(Action::ACTION_GEOMETRY_STREAM_LINE_CREATE),{action(Action::ACTION_GEOMETRY_STREAM_LINE_CREATE),action(Action::ACTION_GEOMETRY_STREAM_LINE_EDIT)},tr("Stream line"));
    group->addLargeSplitAction(action(Action::ACTION_GEOMETRY_SCALAR_FIELD_CREATE),{action(Action::ACTION_GEOMETRY_SCALAR_FIELD_CREATE),action(Action::ACTION_GEOMETRY_SCALAR_FIELD_EDIT)},tr("Scalar field"));
    group->addLargeSplitAction(action(Action::ACTION_GEOMETRY_VECTOR_FIELD_CREATE),{action(Action::ACTION_GEOMETRY_VECTOR_FIELD_CREATE),action(Action::ACTION_GEOMETRY_VECTOR_FIELD_EDIT)},tr("Vector field"));

    group = this->ribbonBar->addGroup(resultsPage,tr("Graphs"));
    group->addLargeAction(action(Action::ACTION_REPORT_MONITORING_POINT_GRAPH));

    group = this->ribbonBar->addGroup(resultsPage,tr("Report"));
    group->addLargeAction(action(Action::ACTION_REPORT_MODEL_STATISTICS),tr("Statistics"));
    group->addLargeAction(action(Action::ACTION_REPORT_CREATE_REPORT));

    R_LOG_TRACE_OUT;
}

void MainWindow::showSolverDockTab(QWidget *tab)
{
    R_LOG_TRACE;
    QDockWidget *solverDock = this->findCustomDockWidget(QString::fromUtf8("solverDock"));
    solverDock->show();
    solverDock->raise();
    this->solverDockTabWidget->setCurrentWidget(tab);
}

void MainWindow::setRibbonLayout(bool ribbonLayout)
{
    R_LOG_TRACE_IN;
    this->classicMenuBar->setVisible(!ribbonLayout);
    this->ribbonBar->setVisible(ribbonLayout);

    for (QToolBar *toolBar : std::as_const(this->toolBars))
    {
        toolBar->setVisible(!ribbonLayout);
        // Otherwise context menu could bring tool bars back to the ribbon layout.
        toolBar->toggleViewAction()->setVisible(!ribbonLayout);
    }

    this->applicationSettings->setValue("mainWindow/ribbonLayout",ribbonLayout);
    R_LOG_TRACE_OUT;
}

void MainWindow::setRibbonCompact(bool ribbonCompact)
{
    R_LOG_TRACE_IN;
    this->ribbonBar->setCompact(ribbonCompact);
    this->applicationSettings->setValue("mainWindow/ribbonCompact",ribbonCompact);

    // Asking for compact ribbon from the classic layout means asking for the ribbon as well.
    if (ribbonCompact)
    {
        this->actionList->getAction(Action::ACTION_APPLICATION_RIBBON_LAYOUT)->setChecked(true);
    }
    R_LOG_TRACE_OUT;
}

void MainWindow::createStatusBar()
{
    R_LOG_TRACE_IN;
    QStatusBar *statusbar;

    statusbar = new QStatusBar(this);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    this->setStatusBar(statusbar);

    QWidget *progressBarWidget = new QWidget;
    statusbar->addWidget(progressBarWidget,1);

    QVBoxLayout *progressBarLayout = new QVBoxLayout;
    progressBarLayout->setSpacing(0);
    progressBarLayout->setContentsMargins(0,0,0,0);
    progressBarWidget->setLayout(progressBarLayout);

    this->mainProgressBar = new RProgressBar(this);
    this->mainProgressBar->hide();
    progressBarLayout->addWidget(this->mainProgressBar);
    R_LOG_TRACE_OUT;
}

void MainWindow::createCentralWidget()
{
    R_LOG_TRACE;
    this->centralTabWidget = new CentralTabWidget(this);
    this->setCentralWidget(this->centralTabWidget);
    this->modelSubWindows = new SubWindowManager(this->centralTabWidget->getMdiArea(), this);
    this->modelSubWindows->readLayout(this->applicationSettings);
    QObject::connect(this->modelSubWindows,&SubWindowManager::tilingAvailableChanged,this->centralTabWidget,&CentralTabWidget::setTileWindowsAvailable);
    QObject::connect(this->modelSubWindows,&SubWindowManager::windowListAvailableChanged,this->centralTabWidget,&CentralTabWidget::setWindowListAvailable);
    QObject::connect(this->centralTabWidget,&CentralTabWidget::tileWindowsClicked,this->modelSubWindows,&SubWindowManager::tileWindows);
    QObject::connect(this->centralTabWidget,&CentralTabWidget::windowsMenuAboutToShow,this->modelSubWindows,&SubWindowManager::fillWindowsMenu);
}

QWidget *MainWindow::createModelTab()
{
    R_LOG_TRACE;

    QSplitter *splitter = new QSplitter;
    splitter->setOpaqueResize(false);
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(false);

    //! Model tree.
    this->modelManagerTree = new ModelTree(this->actionList,splitter);

    //! Model display properties tree.
    ModelEntityDisplayPropertiesTree *treeModelDisplayProperties = new ModelEntityDisplayPropertiesTree;
    splitter->addWidget(treeModelDisplayProperties);

    //! Model geometric properties tree.
    ModelEntityGeometricPropertiesTree *treeModelGeometricProperties = new ModelEntityGeometricPropertiesTree;
    splitter->addWidget(treeModelGeometricProperties);

    splitter->setStretchFactor(0,10);
    splitter->setStretchFactor(1,4);
    splitter->setStretchFactor(2,1);

    return splitter;
}

QWidget *MainWindow::createDocumentTab()
{
    R_LOG_TRACE;

    DocumentTree *treeDocuments = new DocumentTree;

    return treeDocuments;
}

QWidget *MainWindow::createRecordsTab()
{
    R_LOG_TRACE;

    ModelRecordsSelector *treeRecords = new ModelRecordsSelector;

    return treeRecords;
}

QWidget *MainWindow::createProblemTab()
{
    R_LOG_TRACE;

    ProblemTree *treeProblem = new ProblemTree;

    return treeProblem;
}

QWidget *MainWindow::createBcTab()
{
    R_LOG_TRACE;
    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(false);

    BCManagerTree *treeBcList = new BCManagerTree(splitter);

    BCTree *treeBc = new BCTree(splitter);

    QObject::connect(treeBcList,
                     &BCManagerTree::bcSelected,
                     treeBc,
                     &BCTree::onBcSelected);

    return splitter;
}

QWidget *MainWindow::createIcTab()
{
    R_LOG_TRACE;
    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(false);

    ICManagerTree *treeIcList = new ICManagerTree(splitter);

    ICTree *treeIc = new ICTree(splitter);

    QObject::connect(treeIcList,
                     &ICManagerTree::icSelected,
                     treeIc,
                     &ICTree::onIcSelected);

    return splitter;
}

QWidget *MainWindow::createEcTab()
{
    R_LOG_TRACE;
    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(false);

    ECManagerTree *treeEcList = new ECManagerTree(splitter);

    ECTree *treeEc = new ECTree(splitter);

    QObject::connect(treeEcList,
                     &ECManagerTree::ecSelected,
                     treeEc,
                     &ECTree::onEcSelected);

    return splitter;
}

QWidget *MainWindow::createMaterialTab()
{
    R_LOG_TRACE;
    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(false);

    MaterialManagerTree *treeMaterialList = new MaterialManagerTree(splitter);

    MaterialTree *treeMaterial = new MaterialTree(splitter);

    QObject::connect(treeMaterialList,&MaterialManagerTree::materialSelected,treeMaterial,&MaterialTree::onMaterialSelected);
    QObject::connect(treeMaterial,&MaterialTree::materialChanged,treeMaterialList,&MaterialManagerTree::onMaterialChanged);

    return splitter;
}

QWidget *MainWindow::createResultsTab()
{
    R_LOG_TRACE;
    QWidget *layoutResults = new QWidget();
    QGridLayout *gridLayout = new QGridLayout(layoutResults);

    ResultsVariableSelector *resultsComboBox = new ResultsVariableSelector;

    gridLayout->addWidget(resultsComboBox,0,0,1,1);

    ResultsVariableTree *treeResultsVariable = new ResultsVariableTree;
    gridLayout->addWidget(treeResultsVariable,1,0,1,1);

    QObject::connect(resultsComboBox,
                     &ResultsVariableSelector::resultsVariableSelected,
                     treeResultsVariable,
                     &ResultsVariableTree::onResultsVariableSelected);

    return layoutResults;
}

QMenu * MainWindow::createPopupMenu ()
{
    R_LOG_TRACE;
    QMenu *popupMenu = this->QMainWindow::createPopupMenu();
//    Disabled untill toolbar modification is not implemented.
//    popupMenu->addSeparator();
//    popupMenu->addAction(this->actionList->getAction(Action::ACTION_SETTINGS_RESTORE));
    return popupMenu;
}

void MainWindow::applyShortcutsToActions()
{
    R_LOG_TRACE_IN;
    for (uint i=0;i<this->actionList->getNActions();i++)
    {
        RAction *action = this->actionList->getAction(i);
        if (!action || action->isSeparator())
        {
            continue;
        }
        QString key = "shortcuts/shortcut_" + action->getName();
        if (this->applicationSettings->contains(key))
        {
            action->setShortcut(this->applicationSettings->value(key).toString());
        }
    }
    R_LOG_TRACE_OUT;
}

void MainWindow::readSettings()
{
    R_LOG_TRACE_IN;
    this->isFirstRun = (this->applicationSettings->getStoredVersion() != RVendor::version());
    this->applicationSettings->sync();
    R_LOG_TRACE_OUT;
}

void MainWindow::writeSettings() const
{
    R_LOG_TRACE_IN;
    // Main window
    this->applicationSettings->setValue("mainWindow/geometry", this->saveGeometry());
    this->applicationSettings->setValue("mainWindow/windowState", this->saveState());
    this->applicationSettings->setValue("mainWindow/ribbonCollapsed", this->ribbonBar->isCollapsed());
    this->applicationSettings->setValue("mainWindow/ribbonPage", this->ribbonBar->getCurrentPage());
    for (const RSplitToolButton *button : std::as_const(this->toolBarSplitButtons))
    {
        if (button->getRememberLastUsed())
        {
            this->applicationSettings->setValue("mainWindow/" + button->objectName(), button->getLastUsedPosition());
        }
    }
    // Model windows
    this->modelSubWindows->writeLayout(this->applicationSettings);
    // Session
    QString sessionFileName = this->session->getFileName();
    if (sessionFileName.isEmpty())
    {
        sessionFileName = this->session->getDefaultFileName();
    }
    this->applicationSettings->setValue("session/fileName", sessionFileName.toUtf8().constData());

    // Shortcuts
    for (uint i=0;i<this->actionList->getNActions();i++)
    {
        const RAction *action = this->actionList->getAction(i);
        if (!action || action->isSeparator())
        {
            continue;
        }
        QString key = "shortcuts/shortcut_" + action->getName();
        this->applicationSettings->setValue(key, action->shortcut().toString());
    }

    this->applicationSettings->sync();
    R_LOG_TRACE_OUT;
}

QList<uint> MainWindow::getSelectedModelIDs() const
{
    R_LOG_TRACE;
    return this->modelManagerTree->getSelectedModelIDs();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    R_LOG_TRACE_IN;

    QString quitQuestion;
    if (SolverManager::getInstance().getNRunning() > 0)
    {
        quitQuestion = "<b>" + tr("If you quit now solver will continue executing in the background.") + "</b><br/>";
    }

    if (RMessageBox::quit(this,quitQuestion) == RMessageBox::Yes)
    {
        this->writeSettings();
        // Accept event -> close window.
        event->accept();
    }
    else
    {
        // Ignore event -> leave window open.
        event->ignore();
    }
    R_LOG_TRACE_OUT;
}

QDockWidget *MainWindow::findFirstDockWidget(Qt::DockWidgetArea area)
{
    R_LOG_TRACE;
    QList<QDockWidget *> dockWidgets = this->findChildren<QDockWidget *>();

    for (int i=0;i<dockWidgets.size();i++)
    {
        if (this->dockWidgetArea(dockWidgets[i]) == area)
        {
            return dockWidgets[i];
        }
    }
    return nullptr;
}

void MainWindow::progressBar(RProgressBar *progressBar, double fraction)
{
    R_LOG_TRACE;
    progressBar->setValue(qRound(fraction*100));
//    progressBar->show();
}

void MainWindow::progressBarInitialize(RProgressBar *progressBar, const QString &message, bool pulseType)
{
    R_LOG_TRACE_IN;
    if (pulseType)
    {
        progressBar->startPulse();
    }
    else
    {
        progressBar->setRange(0,100);
    }
    progressBar->setMessage(message);
    progressBar->setValue(0);
    progressBar->show();
    this->statusBar()->clearMask();
    R_LOG_TRACE_OUT;
}

void MainWindow::progressBarFinalize(RProgressBar *progressBar, const QString &message)
{
    R_LOG_TRACE_IN;
    progressBar->stopPulse();
    progressBar->setRange(0,100);
    progressBar->setValue(100);
    progressBar->setMessage("");
    progressBar->hide();
    this->statusBar()->showMessage(message,5000);
    R_LOG_TRACE_OUT;
}

void MainWindow::setEnabled(bool enabled)
{
    R_LOG_TRACE;
    this->modelsTab->setEnabled(enabled);
    this->documentsTab->setEnabled(enabled);
//    this->dockRecords->setEnabled(enabled);
    this->problemTab->setEnabled(enabled);
    this->bcTab->setEnabled(enabled);
    this->icTab->setEnabled(enabled);
    this->ecTab->setEnabled(enabled);
    this->materialTab->setEnabled(enabled);
    this->resultsTab->setEnabled(enabled);
    this->actionList->setEnabled(enabled);
    if (enabled)
    {
        this->actionList->processAvailability();
    }
}

void MainWindow::enable()
{
    R_LOG_TRACE;
    this->setEnabled(true);
}

void MainWindow::disable()
{
    R_LOG_TRACE;
    this->setEnabled(false);
}

void MainWindow::onMainProgress(double fraction)
{
    R_LOG_TRACE;
    this->progressBar(this->mainProgressBar,fraction);
}

void MainWindow::onMainProgressInitialize(const QString &message, bool pulseType)
{
    R_LOG_TRACE;
    this->progressBarInitialize(this->mainProgressBar,message,pulseType);
}

void MainWindow::onMainProgressFinalize(const QString &message)
{
    R_LOG_TRACE;
    this->progressBarFinalize(this->mainProgressBar,message);
}

void MainWindow::onDrawObjectAdded()
{
    R_LOG_TRACE;
    this->showDrawWidget();
}

void MainWindow::onDrawObjectRemoved()
{
    R_LOG_TRACE;
    if (this->session->getDrawEngine()->getNObjects() == 0)
    {
        this->hideDrawWidget();
    }
}

void MainWindow::onJobBlocking(bool blocking)
{
    R_LOG_TRACE;
    if (blocking)
    {
        this->disable();
    }
    else
    {
        this->enable();
    }
}

void MainWindow::onJobStarted()
{
    R_LOG_TRACE;
}

void MainWindow::showProblemSetupTab(RProblemTypeMask problemTypeMask)
{
    R_LOG_TRACE;
    this->solverDockTabWidget->setCurrentWidget(ProblemTree::hasSettings(problemTypeMask) ? this->problemTab : this->bcTab);
}

void MainWindow::onJobEnded()
{
    R_LOG_TRACE;
}

void MainWindow::onSolverJobStateChanged()
{
    R_LOG_TRACE;
    this->actionList->processAvailability();
}

void MainWindow::onModelAdded(uint position)
{
    R_LOG_TRACE;
    this->modelSubWindows->onModelAdded(position);
    this->modelManagerTree->onModelAdded(position);
    if (this->session->getModel(position).isSelected(false))
    {
        this->session->setModelSelected(position,true);
    }
    if (this->session->getModel(position).getProblemTaskTree().getProblemTypeMask() == R_PROBLEM_NONE)
    {
        ProblemTaskDialog problemTaskDialog(position,this);
        problemTaskDialog.exec();
    }
}

void MainWindow::onModelRemoved(uint position)
{
    R_LOG_TRACE;
    this->modelSubWindows->onModelRemoved(position);
    this->modelManagerTree->onModelRemoved(position);
    this->actionList->processAvailability();
}

void MainWindow::onModelChanged(uint position)
{
    R_LOG_TRACE;
    this->modelSubWindows->onModelChanged(position);
    this->modelManagerTree->onModelChanged(position);
    this->actionList->processAvailability();
}

void MainWindow::onProblemChanged(uint)
{
    R_LOG_TRACE;
    this->actionList->processAvailability();
}

void MainWindow::onResultsChanged(uint)
{
    R_LOG_TRACE;
    this->actionList->processAvailability();
}

void MainWindow::onEntityVisibilityChanged(uint modelID, REntityGroupType, uint, bool visible)
{
    R_LOG_TRACE;
    bool modelIsVisible = false;
    if (visible)
    {
        modelIsVisible = true;
    }
    else
    {
        if (!this->session->getModel(modelID).isVisible(false))
        {
            modelIsVisible = true;
        }
    }
    if (modelIsVisible)
    {
        if (!this->modelSubWindows->windowExists(modelID))
        {
            this->modelSubWindows->windowCreate(modelID);
        }
    }
}

void MainWindow::onGeometryTransformFinalize()
{
    R_LOG_TRACE;
    this->hideTransformGeometryWidget();
}

void MainWindow::onNHistoryRecordsChanged(uint)
{
    R_LOG_TRACE;
    this->actionList->processAvailability();
}

void MainWindow::onToolbarIconSizeChanged(int toolbarIconSize)
{
    R_LOG_TRACE;
    this->setIconSize(QSize(toolbarIconSize,toolbarIconSize));
    this->ribbonBar->setIconSize(toolbarIconSize);
}
