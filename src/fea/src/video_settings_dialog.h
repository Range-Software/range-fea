#ifndef VIDEO_SETTINGS_DIALOG_H
#define VIDEO_SETTINGS_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>

#include "video_settings.h"

class VideoSettingsDialog : public QDialog
{

    Q_OBJECT

    protected:

        VideoSettings videoSettings;

        QComboBox *videoCodecCombo;
        QComboBox *fileFormatCombo;
        QSpinBox *fpsSpin;
        QSpinBox *fprSpin;

    public:

        //! Constructor.
        explicit VideoSettingsDialog(const VideoSettings &videoSettings, QWidget *parent = nullptr);

        const VideoSettings &getVideoSettings() const;

    protected:

        //! Get supported video codecs.
        static QList<QMediaFormat::VideoCodec> findSupportedVideoCodecs();

        //! Get supported file formats.
        static QList<QMediaFormat::FileFormat> findSupportedFileFormats();

    protected slots:

        //! Video codec has changed.
        void onVideoCodecIndexChanged(int index);

        //! File format has changed.
        void onFileFormatIndexChanged(int index);

        //! FPS value has changed.
        void onFpsValueChanged(int value);

        //! FPR value has changed.
        void onFprValueChanged(int value);

};

#endif // VIDEO_SETTINGS_DIALOG_H
