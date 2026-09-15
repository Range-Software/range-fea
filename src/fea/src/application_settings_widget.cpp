#include <QVBoxLayout>

#include "application_settings_widget.h"
#include "render_backend.h"

ApplicationSettingsWidget::ApplicationSettingsWidget(ApplicationSettings *applicationSettings, QWidget *parent)
    : RApplicationSettingsWidget{applicationSettings,parent}
    , applicationSettings{applicationSettings}
{
    QWidget *generalWidget = this->createGeneralTab();
    this->tabWidget->addTab(generalWidget,tr("General"));
}

QWidget *ApplicationSettingsWidget::createGeneralTab()
{
    QWidget *widget = new QWidget;

    QGridLayout *layout = new QGridLayout;
    widget->setLayout(layout);

    this->solverFileChooserButton = new RFileChooserButton(tr("Solver executable") + ":",
                                                           RFileChooserButton::OpenFile,
                                                           tr("Select Range solver executable"),
                                                           this->applicationSettings->getSolverPath(),
                                                           tr("All files") + " (*)");
    layout->addWidget(this->solverFileChooserButton,0,0,1,2);

    QLabel *ncpusLabel = new QLabel(tr("Number of threads") + ":");
    layout->addWidget(ncpusLabel,1,0,1,1);

    this->nThreadsSpin = new QSpinBox;
    this->nThreadsSpin->setRange(1,ApplicationSettings::getMaxThreads());
    this->nThreadsSpin->setValue(this->applicationSettings->getNThreads());
    this->nThreadsSpin->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout->addWidget(this->nThreadsSpin,1,1,1,1);

    QLabel *nHistoryRecordsLabel = new QLabel(tr("Number of history (undo) records") + ":");
    layout->addWidget(nHistoryRecordsLabel,2,0,1,1);

    this->nHistoryRecordsSpin = new QSpinBox;
    this->nHistoryRecordsSpin->setRange(1,INT_MAX);
    this->nHistoryRecordsSpin->setValue(this->applicationSettings->getNHistoryRecords());
    this->nHistoryRecordsSpin->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout->addWidget(this->nHistoryRecordsSpin,2,1,1,1);

    QLabel *renderBackendLabel = new QLabel(tr("Rendering backend") + ":");
    layout->addWidget(renderBackendLabel,3,0,1,1);

    this->renderBackendCombo = new QComboBox;
    foreach (RenderBackend::Type renderBackend, RenderBackend::getTypes())
    {
        this->renderBackendCombo->addItem(RenderBackend::toDisplayString(renderBackend),int(renderBackend));
    }
    ApplicationSettingsWidget::selectComboValue(this->renderBackendCombo,int(this->applicationSettings->getRenderBackend()));
    this->renderBackendCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout->addWidget(this->renderBackendCombo,3,1,1,1);

    QLabel *rhiApiLabel = new QLabel(tr("Graphics API") + ":");
    layout->addWidget(rhiApiLabel,4,0,1,1);

    this->rhiApiCombo = new QComboBox;
    foreach (RenderBackend::RhiApi rhiApi, RenderBackend::getAvailableRhiApis())
    {
        this->rhiApiCombo->addItem(RenderBackend::toDisplayString(rhiApi),int(rhiApi));
    }
    ApplicationSettingsWidget::selectComboValue(this->rhiApiCombo,int(this->applicationSettings->getRhiApi()));
    this->rhiApiCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    // The graphics API only matters for the QRhi backend.
    this->rhiApiCombo->setEnabled(this->applicationSettings->getRenderBackend() == RenderBackend::Type::Rhi);
    layout->addWidget(this->rhiApiCombo,4,1,1,1);

    QLabel *renderBackendNoteLabel = new QLabel(tr("The rendering backend is applied the next time the application starts."));
    renderBackendNoteLabel->setWordWrap(true);
    renderBackendNoteLabel->setEnabled(false);
    layout->addWidget(renderBackendNoteLabel,5,0,1,2);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(spacer,6,0,1,2);

    QObject::connect(this->solverFileChooserButton,&RFileChooserButton::fileNameChanged,this,&ApplicationSettingsWidget::onSolverPathChanged);
    QObject::connect(this->nThreadsSpin,&QSpinBox::valueChanged,this,&ApplicationSettingsWidget::onNThreadsChanged);
    QObject::connect(this->nHistoryRecordsSpin,&QSpinBox::valueChanged,this,&ApplicationSettingsWidget::onNHistoryRecordsChanged);
    QObject::connect(this->renderBackendCombo,&QComboBox::currentIndexChanged,this,&ApplicationSettingsWidget::onRenderBackendChanged);
    QObject::connect(this->rhiApiCombo,&QComboBox::currentIndexChanged,this,&ApplicationSettingsWidget::onRhiApiChanged);

    return widget;
}

void ApplicationSettingsWidget::selectComboValue(QComboBox *comboBox, int value)
{
    const int index = comboBox->findData(value);
    if (index >= 0)
    {
        comboBox->setCurrentIndex(index);
    }
}

void ApplicationSettingsWidget::setDefaultValues()
{
    this->RApplicationSettingsWidget::setDefaultValues();

    this->solverFileChooserButton->setFileName(ApplicationSettings::getDefaultSolverPath());
    this->nThreadsSpin->setValue(ApplicationSettings::getDefaultNThreads());
    this->nHistoryRecordsSpin->setValue(ApplicationSettings::getDefaultNHistoryRecords());
    ApplicationSettingsWidget::selectComboValue(this->renderBackendCombo,int(ApplicationSettings::getDefaultRenderBackend()));
    ApplicationSettingsWidget::selectComboValue(this->rhiApiCombo,int(ApplicationSettings::getDefaultRhiApi()));
}

void ApplicationSettingsWidget::onSolverPathChanged(const QString &solverPath)
{
    this->applicationSettings->setSolverPath(solverPath);
}

void ApplicationSettingsWidget::onNThreadsChanged(int nThreads)
{
    this->applicationSettings->setNThreads(nThreads);
}

void ApplicationSettingsWidget::onNHistoryRecordsChanged(int nHistoryRecords)
{
    this->applicationSettings->setNHistoryRecords(nHistoryRecords);
}

void ApplicationSettingsWidget::onRenderBackendChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    RenderBackend::Type renderBackend = RenderBackend::Type(this->renderBackendCombo->itemData(index).toInt());

    this->applicationSettings->setRenderBackend(renderBackend);
    this->rhiApiCombo->setEnabled(renderBackend == RenderBackend::Type::Rhi);
}

void ApplicationSettingsWidget::onRhiApiChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    this->applicationSettings->setRhiApi(RenderBackend::RhiApi(this->rhiApiCombo->itemData(index).toInt()));
}

