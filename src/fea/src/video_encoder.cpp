#include <rbl_error.h>
#include <rbl_logger.h>

#include <QMediaRecorder>
#include <QMediaFormat>
#include <QVideoFrame>
#include <QVideoFrameInput>
#include <QUrl>

#include "video_encoder.h"

VideoEncoder::VideoEncoder(const QString &fileName, const QQueue<QImage> &imageStack, const VideoSettings &videoSettings, QObject *parent)
    : QObject{parent}
    , videoSettings{videoSettings}
    , imageQueue{imageStack}
    , session{nullptr}
    , frameInput{nullptr}
{
    this->session = new QMediaCaptureSession(parent);

    QMediaRecorder *recorder = new QMediaRecorder;

    QMediaFormat mediaFormat = recorder->mediaFormat();
    mediaFormat.setFileFormat(this->videoSettings.getFileFormat());
    mediaFormat.setVideoCodec(this->videoSettings.getVideoCodec());

    recorder->setMediaFormat(mediaFormat);
    recorder->setQuality(QMediaRecorder::HighQuality);
    recorder->setVideoFrameRate(qreal(this->videoSettings.getFps()));

    this->session->setRecorder(recorder);

    this->frameInput = new QVideoFrameInput;
    this->session->setVideoFrameInput(this->frameInput);

    QObject::connect(recorder,&QMediaRecorder::actualLocationChanged,this,&VideoEncoder::onActualLocationChanged);
    QObject::connect(recorder,&QMediaRecorder::errorOccurred,this,&VideoEncoder::onRecorderErrorOccurred);
    QObject::connect(this->frameInput,&QVideoFrameInput::readyToSendVideoFrame,this,&VideoEncoder::onReadyToSendVideoFrame);

    this->session->recorder()->setOutputLocation(QUrl::fromLocalFile(fileName));

    RLogger::info("Start recording\n");
    this->session->recorder()->record();
}

void VideoEncoder::onActualLocationChanged(const QUrl &location)
{
    emit this->outputFileNameChanged(location.path());
}

void VideoEncoder::onReadyToSendVideoFrame()
{
    static uint encodedTotal=0;
    uint encoded=0;

    bool encodeNextImage = true;
    while (!this->imageQueue.isEmpty() && encodeNextImage)
    {
        QVideoFrame videoFrame(this->imageQueue.dequeue());
        if (!videoFrame.isValid())
        {
            RLogger::warning("Invalid video frame\n");
            continue;
        }
        if (encodedTotal == 0)
        {
            videoFrame.setStreamFrameRate(qreal(this->videoSettings.getFps()));
        }
        encodeNextImage = this->frameInput->sendVideoFrame(videoFrame);
        encoded++;
        encodedTotal++;
    }
    RLogger::info("%u (+%u) frames encoded\n",encodedTotal,encoded);

    if (this->imageQueue.isEmpty())
    {
        RLogger::info("Stop recording\n");
        this->session->recorder()->stop();
        emit this->finished(this->session->recorder()->actualLocation().path());
    }
}

void VideoEncoder::onRecorderErrorOccurred(QMediaRecorder::Error, const QString &errorString)
{
    RLogger::error("Recorder error: %s\n",errorString.toUtf8().constData());
    emit this->failed();
}
