#ifndef VIDEO_SETTINGS_H
#define VIDEO_SETTINGS_H

#include <QList>
#include <QString>
#include <QMediaFormat>

class VideoSettings
{

    protected:

        uint fps;
        uint fpi;
        QMediaFormat::VideoCodec videoCodec;
        QMediaFormat::FileFormat fileFormat;

    public:

        static const uint minFps;
        static const uint maxFps;
        static const uint minFpi;
        static const uint maxFpp;

    private:

        //! Internal initialization function.
        void _init(const VideoSettings *pVideoSettings = nullptr);

    public:

        //! Constructor.
        VideoSettings();

        //! Copy constructor.
        VideoSettings(const VideoSettings &videoSettings);

        //! Destructor.
        ~VideoSettings();

        //! Assignment operator.
        VideoSettings &operator =(const VideoSettings &videoSettings);

        //! Return frames per second.
        uint getFps() const;

        //! Set frames per second.
        void setFps(uint value);

        //! Return frames per image.
        uint getFpi() const;

        //! Set frames per image.
        void setFpi(uint value);

        //! Return video codec.
        QMediaFormat::VideoCodec getVideoCodec() const;

        //! Set video codec.
        void setVideoCodec(const QMediaFormat::VideoCodec &videoCodec);

        //! Return file format.
        QMediaFormat::FileFormat getFileFormat() const;

        //! Set file format.
        void setFileFormat(const QMediaFormat::FileFormat &fileFormat);

};

#endif // VIDEO_SETTINGS_H
