#ifndef MODEL_RECORDS_SELECTOR_H
#define MODEL_RECORDS_SELECTOR_H

#include <QWidget>
#include <QAction>

#include "model_records_tree.h"
#include "video_settings.h"

class ModelRecordsSelector : public QWidget
{

    Q_OBJECT

    protected:

        //! Play action.
        QAction *playAction;
        //! Record action.
        QAction *recordAction;
        //! Active model id (while updating).
        uint updateModelID;
        //! Mark next record.
        bool markNextIndicator;
        //! Record screenshot indicator.
        bool recordIndicator;
        //! Model records tree.
        ModelRecordsTree *tree;
        //! Video settings.
        VideoSettings videoSettings;

    public:

        //! Constructor.
        explicit ModelRecordsSelector(QWidget *parent = nullptr);

    protected:

        //! Create animation from screenshots.
        void createAnimation(bool modelID);

    protected slots:

        //! Record markeded.
        void onRecordMarked(uint modelID, const QString &recordFileName);

        //! On record has finished.
        void onRecordFinished();

        //! On update job has finished.
        void onUpdateJobFinished();

        //! Play/Pause.
        void playToggle(bool jumpToFirst = false);

        //! Load next record.
        void loadNextRecord(bool jumpToFirst = false);

        //! On record video toggled.
        void onRecordVideo();

        //! Video recording has finished.
        void onVideoEncodeFinished(QString fileName);

        //! Video recording has failed.
        void onVideoEncodeFailed();

    signals:

        //! Signal to load next record.
        void recordFinished();
    
};

#endif // MODEL_RECORDS_SELECTOR_H
