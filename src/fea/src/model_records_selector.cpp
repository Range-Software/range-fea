#include <QAction>
#include <QIcon>
#include <QVBoxLayout>
#include <QToolBar>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>

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
    // Start playback at the selected "From" record (jumpToFirst = true); pausing ignores it.
    QObject::connect(actions.last(), &QAction::triggered, this, [this](){ this->playToggle(true); });
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

    QToolBar *rangeToolBar = new QToolBar;
    layout->addWidget(rangeToolBar);

    this->startSpin = new QSpinBox;
    this->startSpin->setMinimum(1);
    this->startSpin->setMaximum(1);
    this->startSpin->setPrefix(tr("From") + " ");
    this->startSpin->setToolTip(tr("First record to play / record"));
    rangeToolBar->addWidget(this->startSpin);

    this->endSpin = new QSpinBox;
    this->endSpin->setMinimum(1);
    this->endSpin->setMaximum(1);
    this->endSpin->setPrefix(tr("To") + " ");
    this->endSpin->setToolTip(tr("Last record to play / record"));
    rangeToolBar->addWidget(this->endSpin);

    // Keep the end record at or after the start record.
    QObject::connect(this->startSpin,
                     QOverload<int>::of(&QSpinBox::valueChanged),
                     this->endSpin,
                     &QSpinBox::setMinimum);

    // Highlight the records inside the from-to range as it changes.
    auto highlightRange = [this](){
        this->tree->setHighlightRange(uint(this->startSpin->value()),uint(this->endSpin->value()));
    };
    QObject::connect(this->startSpin,QOverload<int>::of(&QSpinBox::valueChanged),this,highlightRange);
    QObject::connect(this->endSpin,QOverload<int>::of(&QSpinBox::valueChanged),this,highlightRange);

    // Range set from the tree's context menu.
    QObject::connect(this->tree,
                     &ModelRecordsTree::setFromRequested,
                     this,
                     [this](uint recordNumber){ this->startSpin->setValue(int(recordNumber)); });
    QObject::connect(this->tree,
                     &ModelRecordsTree::setToRequested,
                     this,
                     [this](uint recordNumber){ this->endSpin->setValue(int(recordNumber)); });
    QObject::connect(this->tree,
                     &ModelRecordsTree::setRangeRequested,
                     this,
                     [this](uint first, uint last){
                         this->startSpin->setValue(int(first));
                         this->endSpin->setValue(int(last));
                     });

    QObject::connect(this->tree,
                     &ModelRecordsTree::recordCountChanged,
                     this,
                     &ModelRecordsSelector::updateRecordRange);
    this->updateRecordRange();
    highlightRange();

    QObject::connect(this,&ModelRecordsSelector::recordFinished,this,&ModelRecordsSelector::onRecordFinished);
}

void ModelRecordsSelector::createAnimation(uint modelID)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelID);
    std::vector<double> &rTimes = rModel.getTimeSolver().getTimes();
    QList<QString> imageFileNames;

    uint firstRecord = uint(this->startSpin->value());
    uint lastRecord = uint(this->endSpin->value());

    for (uint j=0;j<rTimes.size();j++)
    {
        uint recordNumber = j+1;
        // Only encode the records that were just played/recorded, i.e. those
        // inside the selected from-to range.  Screen-shots left over from earlier
        // recordings also exist on disk and must be skipped.
        if (recordNumber < firstRecord || recordNumber > lastRecord)
        {
            continue;
        }

        QString recordFileName(RFileManager::getFileNameWithTimeStep(rModel.getFileName(),recordNumber));
        QString imageFileName(RFileManager::getFileNameWithTimeStep(rModel.buildScreenShotFileName(),recordNumber));
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

void ModelRecordsSelector::updateRecordRange()
{
    // Record numbers can be sparse (e.g. 2,10,...,102), so the spin boxes range
    // over the actual first/last record numbers rather than a 1..N count.
    uint first = this->tree->getFirstRecordNumber();
    uint last = this->tree->getLastRecordNumber();
    if (last < first || last < 1)
    {
        first = 1;
        last = 1;
    }

    // Preserve a full-range end selection so the default keeps playing through
    // to the last record as records are added.
    bool endWasFull = (this->endSpin->value() >= this->endSpin->maximum());

    this->startSpin->setRange(int(first),int(last));
    this->endSpin->setMaximum(int(last));
    this->endSpin->setMinimum(this->startSpin->value());

    if (endWasFull)
    {
        this->endSpin->setValue(int(last));
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
    // Stop when there is no next record, or advancing would step past the "To"
    // record.  Checking the next record (rather than the current one) avoids
    // loading the record just beyond the end of the range.
    uint nextRecordNumber = this->tree->getNextRecordNumber();
    if (!jumpToFirst && (nextRecordNumber == 0 ||
                         nextRecordNumber > uint(this->endSpin->value()) ||
                         !this->markNextIndicator))
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
            this->tree->markRecord(uint(this->startSpin->value()));
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
