#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

#include <QString>
#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QVideoFrameInput>
#include <QQueue>

#include "video_settings.h"

class VideoEncoder : public QObject
{

    Q_OBJECT

    protected:

        VideoSettings videoSettings;

        QQueue<QImage> imageQueue;

        QMediaCaptureSession *session;
        QVideoFrameInput *frameInput;

    public:

        //! Constructor.
        explicit VideoEncoder(const QString &fileName, const QQueue<QImage> &imageStack, const VideoSettings &videoSettings, QObject *parent = nullptr);

        //! Add frame to video.
        void newFrame(const QImage &image);

    private slots:

        //! Actual location has chenged.
        void onActualLocationChanged(const QUrl &location);

        //! Video frame input is ready send frames.
        void onReadyToSendVideoFrame();

        //! Media recorder error occured.
        void onRecorderErrorOccurred(QMediaRecorder::Error, const QString &errorString);

    signals:

        //! Output file has changed.
        void outputFileNameChanged(QString fileName);

        //! Encoding has finished.
        void finished(QString fileName);

        //! Encoding has failed.
        void failed();

};

#endif // VIDEO_ENCODER_H
