#include <QtGui/QCloseEvent>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QGridLayout>
#include <QStatusBar>
#include <QSplitter>
#include <QScrollBar>
#include <QHBoxLayout>

#include <rbl_error.h>
#include <rbl_job_manager.h>

#include <rgl_first_run_dialog.h>
#include <rgl_progress_handler.h>
#include <rgl_message_box.h>

#include "action.h"
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
    this->setWindowIcon(QIcon(":/icons/logos/pixmaps/range-fea.svg"));

    int toolbarIconSize = this->applicationSettings->getToolbarIconSize();
    this->setIconSize(QSize(toolbarIconSize,toolbarIconSize));
    this->setToolButtonStyle(Qt::ToolButtonIconOnly);

    this->setDockOptions(QMainWindow::ForceTabbedDocks);

    this->createMenus();
    this->createToolBars();
    this->createStatusBar();
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

    QDockWidget *solverDock = new QDockWidget(this);
    solverDock->setObjectName(QString::fromUtf8("solverDock"));
    solverDock->setWindowTitle(tr("Solver"));
    solverDock->setTitleBarWidget(new QWidget(solverDock));
    this->addDockWidget(Qt::RightDockWidgetArea, solverDock);

    QTabWidget *solverDockTabWidget = new QTabWidget;
    solverDockTabWidget->setTabPosition(QTabWidget::East);
    solverDock->setWidget(solverDockTabWidget);

    this->problemTab = this->createProblemTab();
    solverDockTabWidget->addTab(this->problemTab,tr("Problem"));

    this->bcTab = this->createBcTab();
    solverDockTabWidget->addTab(this->bcTab,tr("Boundary conditions"));

    this->icTab = this->createIcTab();
    solverDockTabWidget->addTab(this->icTab,tr("Initial conditions"));

    this->ecTab = this->createEcTab();
    solverDockTabWidget->addTab(this->ecTab,tr("Environment conditions"));

    this->materialTab = this->createMaterialTab();
    solverDockTabWidget->addTab(this->materialTab,tr("Material"));

    this->resultsTab = this->createResultsTab();
    solverDockTabWidget->addTab(this->resultsTab,tr("Results"));

    this->restoreGeometry(this->applicationSettings->value("mainWindow/geometry").toByteArray());
    this->restoreState(this->applicationSettings->value("mainWindow/windowState").toByteArray());

    // Toolbar icon size changed signal
    QObject::connect(this->applicationSettings,&RApplicationSettings::toolbarIconSizeChanged,this,&MainWindow::onToolbarIconSizeChanged);

    // Main progress signals
    QObject::connect(&RProgressHandler::getInstance(),&RProgressHandler::progress,this,&MainWindow::onMainProgress);
    QObject::connect(&RProgressHandler::getInstance(),&RProgressHandler::progressInitialize,this,&MainWindow::onMainProgressInitialize);
    QObject::connect(&RProgressHandler::getInstance(),&RProgressHandler::progressFinalize,this,&MainWindow::onMainProgressFinalize);

    // Solver manager signals.
    QObject::connect(&SolverManager::getInstance(),&SolverManager::jobBlocking,this,&MainWindow::onJobBlocking);

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
        RFirstRunDialog(this).exec();
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

void MainWindow::createMenus()
{
    R_LOG_TRACE_IN;
    QMenuBar *menubar = new QMenuBar(this);
    this->setMenuBar(menubar);

    QMenu *menuFile = new QMenu(menubar);
    menuFile->setTitle(QApplication::translate("MainWindow", "File"));
    QMenu *menuFileSession = new QMenu(menuFile);
    menuFileSession->setTitle(QApplication::translate("MainWindow", "Session"));
    menuFileSession->setIcon(QIcon(":/icons/file/pixmaps/range-session->svg"));
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
    menuFileExport->setTitle(QApplication::translate("MainWindow", "Export"));
    menuFileExport->setIcon(QIcon(":/icons/file/pixmaps/range-model_save_as.svg"));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_MSH));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_RAW));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_STL_ASCII));
    menuFileExport->addAction(this->actionList->getAction(Action::ACTION_MODEL_EXPORT_STL_BINARY));
    menuFile->addAction(menuFileExport->menuAction());
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_RENAME));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_CLOSE));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_RELOAD_RESULTS));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_MODEL_DROP_RESULTS));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_SETTINGS));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuFile->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_QUIT));
    menubar->addAction(menuFile->menuAction());

    QMenu *menuGeometry = new QMenu(menubar);
    menuGeometry->setTitle(QApplication::translate("MainWindow", "Geometry"));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_UNDO));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REDO));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    QMenu *menuGeometryDraw = new QMenu(menuGeometry);
    menuGeometryDraw->setTitle(QApplication::translate("MainWindow", "Draw"));
    menuGeometryDraw->setIcon(QIcon(":/icons/geometry/pixmaps/range-draw_geometry.svg"));
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
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
//    QMenu *menuGeometryPoint = new QMenu(menuGeometry);
//    menuGeometryPoint->setTitle(QApplication::translate("MainWindow", "Point"));
//    menuGeometryPoint->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_point.svg"));
//    menuGeometry->addAction(menuGeometryPoint->menuAction());
    QMenu *menuGeometryLine = new QMenu(menuGeometry);
    menuGeometryLine->setTitle(QApplication::translate("MainWindow", "Line"));
    menuGeometryLine->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_line.svg"));
    menuGeometryLine->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_LINE_GENERATE_FROM_EDGES));
    menuGeometry->addAction(menuGeometryLine->menuAction());
    QMenu *menuGeometrySurface = new QMenu(menuGeometry);
    menuGeometrySurface->setTitle(QApplication::translate("MainWindow", "Surface"));
    menuGeometrySurface->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_surface.svg"));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_MARK));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SWAP_ELEMENT_NORMAL));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SWAP_NORMALS));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SYNC_NORMALS));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_CLOSE_HOLE));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_COARSEN));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIND_INTERSECTED_ELEMENTS));
    menuGeometrySurface->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BREAK_INTERSECTED_ELEMENTS));
    QMenu *menuGeometryBoolOperations = new QMenu(menuGeometrySurface);
    menuGeometryBoolOperations->setTitle(QApplication::translate("MainWindow", "Boolean operations"));
    menuGeometryBoolOperations->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_surface_bool_operation.svg"));
    menuGeometryBoolOperations->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_UNION));
    menuGeometryBoolOperations->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_DIFFERENCE));
    menuGeometryBoolOperations->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_INTERSECTION));
    menuGeometrySurface->addAction(menuGeometryBoolOperations->menuAction());
    menuGeometry->addAction(menuGeometrySurface->menuAction());
    QMenu *menuGeometryVolume = new QMenu(menuGeometry);
    menuGeometryVolume->setTitle(QApplication::translate("MainWindow", "Volume"));
    menuGeometryVolume->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_volume.svg"));
    menuGeometryVolume->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_VOLUME_GENERATE_TETRAHEDRA));
    menuGeometry->addAction(menuGeometryVolume->menuAction());
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    QMenu *menuGeometryCut = new QMenu(menuGeometry);
    menuGeometryCut->setTitle(QApplication::translate("MainWindow", "Cut"));
    menuGeometryCut->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_cut.svg"));
    menuGeometryCut->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_CUT_CREATE));
    menuGeometryCut->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_CUT_EDIT));
    menuGeometry->addAction(menuGeometryCut->menuAction());
    QMenu *menuGeometryIso = new QMenu(menuGeometry);
    menuGeometryIso->setTitle(QApplication::translate("MainWindow", "Iso"));
    menuGeometryIso->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_iso.svg"));
    menuGeometryIso->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ISO_CREATE));
    menuGeometryIso->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ISO_EDIT));
    menuGeometry->addAction(menuGeometryIso->menuAction());
    QMenu *menuGeometryStreamLine = new QMenu(menuGeometry);
    menuGeometryStreamLine->setTitle(QApplication::translate("MainWindow", "Stream line"));
    menuGeometryStreamLine->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_stream_line.svg"));
    menuGeometryStreamLine->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_STREAM_LINE_CREATE));
    menuGeometryStreamLine->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_STREAM_LINE_EDIT));
    menuGeometry->addAction(menuGeometryStreamLine->menuAction());
    QMenu *menuGeometryScalarField = new QMenu(menuGeometry);
    menuGeometryScalarField->setTitle(QApplication::translate("MainWindow", "Scalar field"));
    menuGeometryScalarField->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_scalar_field.svg"));
    menuGeometryScalarField->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SCALAR_FIELD_CREATE));
    menuGeometryScalarField->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SCALAR_FIELD_EDIT));
    menuGeometry->addAction(menuGeometryScalarField->menuAction());
    QMenu *menuGeometryVectorField = new QMenu(menuGeometry);
    menuGeometryVectorField->setTitle(QApplication::translate("MainWindow", "Vector field"));
    menuGeometryVectorField->setIcon(QIcon(":/icons/geometry/pixmaps/range-entity_vector_field.svg"));
    menuGeometryVectorField->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_VECTOR_FIELD_CREATE));
    menuGeometryVectorField->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_VECTOR_FIELD_EDIT));
    menuGeometry->addAction(menuGeometryVectorField->menuAction());
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ENTITY_MERGE));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_ENTITY_REMOVE));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIND_SLIVER_ELEMENTS));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIX_SLIVER_ELEMENTS));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_MERGE_NEAR_NODES));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_MOVE_NODE));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REMOVE_NODE));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REMOVE_NODE_AND_CLOSE_HOLE));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_CREATE_ELEMENT));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REMOVE_ELEMENT));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_TRANSFORM));
    menuGeometry->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    QMenu *menuGeometryDevelopement = new QMenu(menuGeometry);
    menuGeometryDevelopement->setTitle(QApplication::translate("MainWindow", "Special tools"));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_EXPORT_SLIVER_ELEMENTS));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_EXPORT_INTERSECTED_ELEMENTS));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_PURGE_UNUSED_NODES));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_PURGE_UNUSED_ELEMENTS));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_NODES));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_ELEMENTS));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_POINT_INSIDE_SURFACE));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_TETRAHEDRALIZE_SURFACE));
    menuGeometryDevelopement->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DEV_CONSOLIDATE));
    menuGeometry->addAction(menuGeometryDevelopement->menuAction());
    menubar->addAction(menuGeometry->menuAction());

    QMenu *menuProblem = new QMenu(menubar);
    menuProblem->setTitle(QApplication::translate("MainWindow", "Problem"));
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_TASK_FLOW));
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_SOLVER_SETUP));
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_DEFINE_MONITORING_POINTS));
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menuProblem->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_RESET));
    menubar->addAction(menuProblem->menuAction());

    QMenu *menuSolver = new QMenu(menubar);
    menuSolver->setTitle(QApplication::translate("MainWindow", "Solution"));
    menuSolver->addAction(this->actionList->getAction(Action::ACTION_SOLVER_START));
    menuSolver->addAction(this->actionList->getAction(Action::ACTION_SOLVER_STOP));
    menuSolver->addAction(this->actionList->getAction(Action::ACTION_SOLVER_KILL));
    menuSolver->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    menubar->addAction(menuSolver->menuAction());

    QMenu *menuReport = new QMenu(menubar);
    menuReport->setTitle(QApplication::translate("MainWindow", "Report"));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_MODEL_STATISTICS));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_SOLVER_LOG));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_CONVERGENCE_GRAPH));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_MONITORING_POINT_GRAPH));
    menuReport->addAction(this->actionList->getAction(Action::ACTION_REPORT_CREATE_REPORT));
    menubar->addAction(menuReport->menuAction());

    menubar->addSeparator();

    QMenu *menuHelp = new QMenu(menubar);
    menuHelp->setTitle(QApplication::translate("MainWindow", "Help"));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_HELP));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_UPDATE));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_ABOUT));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_ABOUT_QT));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_LICENSE));
    menuHelp->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_RELEASE_NOTES));
    menubar->addAction(menuHelp->menuAction());
    R_LOG_TRACE_OUT;
}

void MainWindow::createToolBars()
{
    R_LOG_TRACE_IN;

    QToolBar *mainToolBar = new QToolBar(this);
    mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
    mainToolBar->setWindowTitle(tr("Main toolbar"));
    this->addToolBar(Qt::TopToolBarArea, mainToolBar);

    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_MODEL_NEW));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_MODEL_OPEN));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_MODEL_SAVE));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_MODEL_CLOSE));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_QUIT));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_UNDO));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_REDO));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_TRANSFORM));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_VOLUME_GENERATE_TETRAHEDRA));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_PROBLEM_TASK_FLOW));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SOLVER_START));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SOLVER_STOP));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SOLVER_KILL));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_REPORT_CONVERGENCE_GRAPH));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_REPORT_CREATE_REPORT));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_MODEL_RELOAD_RESULTS));

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainToolBar->addWidget(spacer);

    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_SETTINGS));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_HELP));
    mainToolBar->addAction(this->actionList->getAction(Action::ACTION_APPLICATION_ABOUT));

    this->addToolBarBreak();

    QToolBar *drawToolBar = new QToolBar(this);
    drawToolBar->setObjectName(QString::fromUtf8("drawToolBar"));
    drawToolBar->setWindowTitle(tr("Draw toolbar"));
    this->addToolBar(Qt::TopToolBarArea, drawToolBar);

    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_POINT));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_LINE));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_TRIANGLE));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_QUADRILATERAL));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_CIRCLE));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_ELLIPSE));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_TETRAHEDRON));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_HEXAHEDRON));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_CYLINDER));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_SPHERE));
    drawToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_DRAW_RAW));

    QToolBar *surfaceToolBar = new QToolBar(this);
    surfaceToolBar->setObjectName(QString::fromUtf8("surfaceToolBar"));
    surfaceToolBar->setWindowTitle(tr("Surface toolbar"));
    this->addToolBar(Qt::TopToolBarArea, surfaceToolBar);

    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_MARK));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SWAP_ELEMENT_NORMAL));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SWAP_NORMALS));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_SYNC_NORMALS));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_CLOSE_HOLE));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_SURFACE_COARSEN));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIND_INTERSECTED_ELEMENTS));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BREAK_INTERSECTED_ELEMENTS));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIND_SLIVER_ELEMENTS));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_FIX_SLIVER_ELEMENTS));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_DIFFERENCE));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_INTERSECTION));
    surfaceToolBar->addAction(this->actionList->getAction(Action::ACTION_GEOMETRY_BOOL_UNION));

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
//    popupMenu->addAction(this->actionList->getAction(Action::ACTION_SEPARATOR));
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
    this->actionList->setEnabled(enabled);;
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

void MainWindow::onJobEnded()
{
    R_LOG_TRACE;
}

void MainWindow::onModelAdded(uint position)
{
    R_LOG_TRACE;
    this->modelSubWindows->onModelAdded(position);
    this->modelManagerTree->onModelAdded(position);
    if (!this->session->isModelSelected(position))
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
}
