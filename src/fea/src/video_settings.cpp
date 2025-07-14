#include "video_settings.h"

const uint VideoSettings::minFps(24);
const uint VideoSettings::maxFps(60);
const uint VideoSettings::minFpi(1);
const uint VideoSettings::maxFpp(100);

void VideoSettings::_init(const VideoSettings *pVideoSettings)
{
    if (pVideoSettings)
    {
        this->fps = pVideoSettings->fps;
        this->fpi = pVideoSettings->fpi;
        this->videoCodec = pVideoSettings->videoCodec;
        this->fileFormat = pVideoSettings->fileFormat;
    }
}

VideoSettings::VideoSettings()
    : fps{24}
    , fpi{1}
    , videoCodec{QMediaFormat::VideoCodec::H264}
    , fileFormat{QMediaFormat::MPEG4}
{
    this->_init();
}

VideoSettings::VideoSettings(const VideoSettings &videoSettings)
{
    this->_init(&videoSettings);
}

VideoSettings::~VideoSettings()
{

}

VideoSettings &VideoSettings::operator =(const VideoSettings &videoSettings)
{
    this->_init(&videoSettings);
    return (*this);
}

uint VideoSettings::getFps() const
{
    return this->fps;
}

void VideoSettings::setFps(uint value)
{
    this->fps = value;
}

uint VideoSettings::getFpi() const
{
    return this->fpi;
}

void VideoSettings::setFpi(uint value)
{
    this->fpi = value;
}

QMediaFormat::VideoCodec VideoSettings::getVideoCodec() const
{
    return videoCodec;
}

void VideoSettings::setVideoCodec(const QMediaFormat::VideoCodec &videoCodec)
{
    this->videoCodec = videoCodec;
}

QMediaFormat::FileFormat VideoSettings::getFileFormat() const
{
    return fileFormat;
}

void VideoSettings::setFileFormat(const QMediaFormat::FileFormat &fileFormat)
{
    this->fileFormat = fileFormat;
}
