#include <QAction>
#include <QIcon>
#include <QVBoxLayout>
#include <QToolBar>
#include <QCheckBox>

#include <rbl_job_manager.h>
#include <rbl_progress.h>

#include <rml_file_manager.h>

#include "model_records_selector.h"
#include "model_io.h"
#include "application.h"
#include "video_settings_dialog.h"
#include "video_encoder.h"

ModelRecordsSelector::ModelRecordsSelector(QWidget *parent)
    : QWidget(parent)
    , markNextIndicator(false)
    , recordIndicator(false)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    this->setLayout(layout);

    this->tree = new ModelRecordsTree;
    layout->addWidget(this->tree);

    QObject::connect(this->tree,
                     &ModelRecordsTree::recordMarked,
                     this,
                     &ModelRecordsSelector::onRecordMarked);

    QToolBar *toolBar = new QToolBar;
    layout->addWidget(toolBar);

    QList<QAction*> actions;

    actions.append(new QAction(QIcon(":/icons/media/pixmaps/range-play_first.svg"),"First",this));
    QObject::connect(actions.last(), &QAction::triggered, this->tree, &ModelRecordsTree::markFirst);

    actions.append(new QAction(QIcon(":/icons/media/pixmaps/range-play_backward.svg"),"Backward",this));
    QObject::connect(actions.last(), &QAction::triggered, this->tree, &ModelRecordsTree::markPrevious);

    actions.append(new QAction(QIcon(":/icons/media/pixmaps/range-play_play.svg"),"Play",this));
    QObject::connect(actions.last(), &QAction::triggered, this, &ModelRecordsSelector::playToggle);
    this->playAction = actions.last();

    actions.append(new QAction(QIcon(":/icons/media/pixmaps/range-play_forward.svg"),"Forward",this));
    QObject::connect(actions.last(), &QAction::triggered, this->tree, &ModelRecordsTree::markNext);

    actions.append(new QAction(QIcon(":/icons/media/pixmaps/range-play_last.svg"),"Last",this));
    QObject::connect(actions.last(), &QAction::triggered, this->tree, &ModelRecordsTree::markLast);

    toolBar->addActions(actions);

    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    toolBar->addWidget(empty);

    actions.clear();

    actions.append(new QAction(QIcon(":/icons/media/pixmaps/range-play_record.svg"),"Record",this));
    QObject::connect(actions.last(), &QAction::triggered, this, &ModelRecordsSelector::onRecordVideo);
    this->recordAction = actions.last();

    toolBar->addActions(actions);

    QObject::connect(this,&ModelRecordsSelector::recordFinished,this,&ModelRecordsSelector::onRecordFinished);
}

void ModelRecordsSelector::createAnimation(bool modelID)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelID);
    std::vector<double> &rTimes = rModel.getTimeSolver().getTimes();
    QList<QString> imageFileNames;

    for (uint j=0;j<rTimes.size();j++)
    {
        QString recordFileName(RFileManager::getFileNameWithTimeStep(rModel.getFileName(),j+1));
        QString imageFileName(RFileManager::getFileNameWithTimeStep(rModel.buildScreenShotFileName(),j+1));
        if (RFileManager::fileExists(recordFileName) && RFileManager::fileExists(imageFileName))
        {
            imageFileNames.append(imageFileName);
        }
    }

    if (imageFileNames.size() <= 0)
    {
        return;
    }

    try
    {
        QQueue<QImage> imageQueue;

        uint cnt=0;
        foreach (const QString &imageFileName, imageFileNames)
        {
            RLogger::info("Adding image file \'%s\'\n",imageFileName.toUtf8().constData());

            QImage image;
            if (!image.load(imageFileName))
            {
                RLogger::error("Failed to load image file \'%s\'\n",imageFileName.toUtf8().constData());
                return;
            }

            for (uint i=0;i<this->videoSettings.getFpi();i++)
            {
                imageQueue.enqueue(image);
            }
            RProgressPrint(cnt++,imageFileNames.size());
        }

        VideoEncoder *videoEncoder = new VideoEncoder(rModel.buildAnimationFileName(),imageQueue,this->videoSettings,this);
        QObject::connect(videoEncoder,&VideoEncoder::finished,this,&ModelRecordsSelector::onVideoEncodeFinished);
        QObject::connect(videoEncoder,&VideoEncoder::finished,videoEncoder,&QObject::deleteLater);
        QObject::connect(videoEncoder,&VideoEncoder::failed,this,&ModelRecordsSelector::onVideoEncodeFailed);
        QObject::connect(videoEncoder,&VideoEncoder::failed,videoEncoder,&QObject::deleteLater);
    }
    catch (const RError &rError)
    {
        RLogger::error("Failed to create animation file:  %s\n", rError.getMessage().toUtf8().constData());
        return;
    }
}

void ModelRecordsSelector::onRecordMarked(uint modelID, const QString &recordFileName)
{
    RLogger::info("Selected record: \'%s\'\n", recordFileName.toUtf8().constData());

    RJob *updateJob = new ModelIO(MODEL_IO_UPDATE, recordFileName, &Application::instance()->getSession()->getModel(modelID));
    this->updateModelID = modelID;

    QObject::connect(updateJob,
                     &RJob::finished,
                     this,
                     &ModelRecordsSelector::onUpdateJobFinished);

    RJobManager::getInstance().submit(updateJob);
}

void ModelRecordsSelector::onRecordFinished()
{
    QTimer::singleShot(this->recordIndicator?0:1000, this, SLOT(loadNextRecord()));
}

void ModelRecordsSelector::onUpdateJobFinished()
{
    if (this->recordIndicator)
    {
        // Take screen-shot
        Application::instance()->getSession()->setTakeScreenShot(this->updateModelID,Application::instance()->getSession()->getModel(this->updateModelID).buildScreenShotFileName(),false);
    }
    if (this->markNextIndicator)
    {
        emit this->recordFinished();
    }
}

void ModelRecordsSelector::playToggle(bool jumpToFirst)
{
    this->markNextIndicator = !this->markNextIndicator;

    if (this->markNextIndicator)
    {
        Application::instance()->getMainWindow()->progressAutoHideDisable();
        this->playAction->setIcon(QIcon(":/icons/media/pixmaps/range-play_pause.svg"));
        this->playAction->setText("Pause");

        this->loadNextRecord(jumpToFirst);
    }
    else
    {
        this->playAction->setIcon(QIcon(":/icons/media/pixmaps/range-play_play.svg"));
        this->playAction->setText("Play");
        this->recordAction->setEnabled(true);
        Application::instance()->getMainWindow()->progressAutoHideEnable();
    }
}

void ModelRecordsSelector::loadNextRecord(bool jumpToFirst)
{
    if (!jumpToFirst && (this->tree->isLast() || !this->markNextIndicator))
    {
        if (this->recordIndicator)
        {
            QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
            for (int i=0;i<selectedModelIDs.size();i++)
            {
                RProgressInitialize("Creating a video file");
                this->createAnimation(selectedModelIDs.at(i));
                RProgressFinalize();
            }
        }
        this->markNextIndicator = false;
        this->recordIndicator = false;
        this->playAction->setIcon(QIcon(":/icons/media/pixmaps/range-play_play.svg"));
        this->playAction->setText("Play");
        this->recordAction->setEnabled(true);
        Application::instance()->getMainWindow()->progressAutoHideEnable();
    }
    else
    {
        if (jumpToFirst)
        {
            if (this->tree->isFirst())
            {
                this->tree->markCurrent();
            }
            else
            {
                this->tree->markFirst();
            }
        }
        else
        {
            this->tree->markNext();
        }
    }
}

void ModelRecordsSelector::onRecordVideo()
{
    VideoSettingsDialog videoSettingsDialog(this->videoSettings);
    if (videoSettingsDialog.exec() == QDialog::Accepted)
    {
        this->videoSettings = videoSettingsDialog.getVideoSettings();
        this->recordIndicator = true;
        this->recordAction->setDisabled(true);
        this->playToggle(true);
    }
}

void ModelRecordsSelector::onVideoEncodeFinished(QString fileName)
{
    RLogger::info("Video has been successfully encoded (file: \"%s\").\n",fileName.toUtf8().constData());
}

void ModelRecordsSelector::onVideoEncodeFailed()
{
    RLogger::error("Video encoding has failed.\n");
}
