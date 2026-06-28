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
    , nextFrameTimeUs{0}
    , encodedFrameCount{0}
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
    uint encoded=0;

    // Microseconds each frame is shown for; explicit per-frame timestamps make the
    // resulting video length deterministic (frameCount / fps) rather than depending
    // on encoder/muxer defaults.
    const qint64 frameDurationUs = (this->videoSettings.getFps() > 0)
                                 ? qint64(1000000.0 / double(this->videoSettings.getFps()) + 0.5)
                                 : 0;

    while (!this->imageQueue.isEmpty())
    {
        QVideoFrame videoFrame(this->imageQueue.head());
        if (!videoFrame.isValid())
        {
            this->imageQueue.dequeue();
            RLogger::warning("Invalid video frame\n");
            continue;
        }
        if (this->encodedFrameCount == 0)
        {
            videoFrame.setStreamFrameRate(qreal(this->videoSettings.getFps()));
        }
        videoFrame.setStartTime(this->nextFrameTimeUs);
        videoFrame.setEndTime(this->nextFrameTimeUs + frameDurationUs);

        if (!this->frameInput->sendVideoFrame(videoFrame))
        {
            // Input not ready: keep the frame queued and retry on the next signal.
            break;
        }

        this->imageQueue.dequeue();
        this->nextFrameTimeUs += frameDurationUs;
        this->encodedFrameCount++;
        encoded++;
    }
    RLogger::info("%llu (+%u) frames encoded\n",this->encodedFrameCount,encoded);

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
