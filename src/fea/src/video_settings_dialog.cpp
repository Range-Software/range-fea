#include <QVBoxLayout>
#include <QFormLayout>
#include <QIcon>
#include <QPushButton>
#include <QDialogButtonBox>

#include <QMediaFormat>
#include <QMediaRecorder>

#include "video_settings_dialog.h"

VideoSettingsDialog::VideoSettingsDialog(const VideoSettings &videoSettings, QWidget *parent)
    : QDialog{parent}
    , videoSettings{videoSettings}
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Video settings"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    this->videoCodecCombo = new QComboBox;
    QList<QMediaFormat::VideoCodec> videoCodecs = VideoSettingsDialog::findSupportedVideoCodecs();
    for (qsizetype i=0;i<videoCodecs.size();i++)
    {
        QMediaFormat::VideoCodec videoCodec = videoCodecs.at(i);

        this->videoCodecCombo->addItem(QMediaFormat::videoCodecDescription(videoCodec),int(videoCodec));

        if (videoCodec == this->videoSettings.getVideoCodec())
        {
            this->videoCodecCombo->setCurrentIndex(i);
        }
    }
    formLayout->addRow(tr("Video codec"),this->videoCodecCombo);

    this->fileFormatCombo = new QComboBox;
    QList<QMediaFormat::FileFormat> fileFormats = VideoSettingsDialog::findSupportedFileFormats();
    for (qsizetype i=0;i<fileFormats.size();i++)
    {
        QMediaFormat::FileFormat fileFormat = fileFormats.at(i);

        this->fileFormatCombo->addItem(QMediaFormat::fileFormatDescription(fileFormat),int(fileFormat));

        if (fileFormat == this->videoSettings.getFileFormat())
        {
            this->fileFormatCombo->setCurrentIndex(i);
        }
    }
    formLayout->addRow(tr("File format"),this->fileFormatCombo);

    this->fpsSpin = new QSpinBox;
    this->fpsSpin->setValue(int(this->videoSettings.getFps()));
    this->fpsSpin->setRange(int(VideoSettings::minFps),int(VideoSettings::maxFps));
    formLayout->addRow(tr("Frames per second"),this->fpsSpin);

    this->fprSpin = new QSpinBox;
    this->fprSpin->setValue(int(this->videoSettings.getFpi()));
    this->fprSpin->setRange(int(VideoSettings::minFpi),int(VideoSettings::maxFpp));
    formLayout->addRow(tr("Frames per record"),this->fprSpin);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(cancelButton,&QPushButton::clicked,this,&VideoSettingsDialog::reject);
    QObject::connect(okButton,&QPushButton::clicked,this,&VideoSettingsDialog::accept);

    QObject::connect(this->videoCodecCombo,&QComboBox::currentIndexChanged,this,&VideoSettingsDialog::onVideoCodecIndexChanged);
    QObject::connect(this->fileFormatCombo,&QComboBox::currentIndexChanged,this,&VideoSettingsDialog::onFileFormatIndexChanged);
    QObject::connect(this->fpsSpin,&QSpinBox::valueChanged,this,&VideoSettingsDialog::onFpsValueChanged);
    QObject::connect(this->fprSpin,&QSpinBox::valueChanged,this,&VideoSettingsDialog::onFprValueChanged);
}

const VideoSettings &VideoSettingsDialog::getVideoSettings() const
{
    return this->videoSettings;
}

QList<QMediaFormat::VideoCodec> VideoSettingsDialog::findSupportedVideoCodecs()
{
    QMediaRecorder *recorder = new QMediaRecorder;
    QMediaFormat mediaFormat = recorder->mediaFormat();
    QList<QMediaFormat::VideoCodec> videoCodecs = mediaFormat.supportedVideoCodecs(QMediaFormat::Encode);
    recorder->deleteLater();
    return videoCodecs;
}

QList<QMediaFormat::FileFormat> VideoSettingsDialog::findSupportedFileFormats()
{
    QMediaRecorder *recorder = new QMediaRecorder;
    QMediaFormat mediaFormat = recorder->mediaFormat();
    QList<QMediaFormat::FileFormat> fileFormats;
    QList<QMediaFormat::FileFormat> supportedFileFormats = mediaFormat.supportedFileFormats(QMediaFormat::Encode);
    for (auto fileFormat : std::as_const(supportedFileFormats))
    {
        if (fileFormat != QMediaFormat::WMA &&
            fileFormat != QMediaFormat::AAC &&
            fileFormat != QMediaFormat::MP3 &&
            fileFormat != QMediaFormat::Wave &&
            fileFormat != QMediaFormat::Mpeg4Audio &&
            fileFormat != QMediaFormat::FLAC)
        {
            fileFormats.append(fileFormat);
        }
    }
    recorder->deleteLater();

    return fileFormats;
}

void VideoSettingsDialog::onVideoCodecIndexChanged(int index)
{
    this->videoSettings.setVideoCodec(QMediaFormat::VideoCodec(this->videoCodecCombo->itemData(index).toInt()));
}

void VideoSettingsDialog::onFileFormatIndexChanged(int index)
{
    this->videoSettings.setFileFormat(QMediaFormat::FileFormat(this->fileFormatCombo->itemData(index).toInt()));
}

void VideoSettingsDialog::onFpsValueChanged(int value)
{
    this->videoSettings.setFps(value);
}

void VideoSettingsDialog::onFprValueChanged(int value)
{
    this->videoSettings.setFpi(value);
}
