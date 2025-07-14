#include <QVBoxLayout>

#include "application_settings_widget.h"

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

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(spacer,3,0,1,2);

    QObject::connect(this->solverFileChooserButton,&RFileChooserButton::fileNameChanged,this,&ApplicationSettingsWidget::onSolverPathChanged);
    QObject::connect(this->nThreadsSpin,&QSpinBox::valueChanged,this,&ApplicationSettingsWidget::onNThreadsChanged);
    QObject::connect(this->nHistoryRecordsSpin,&QSpinBox::valueChanged,this,&ApplicationSettingsWidget::onNHistoryRecordsChanged);

    return widget;
}

void ApplicationSettingsWidget::setDefaultValues()
{
    this->RApplicationSettingsWidget::setDefaultValues();

    this->solverFileChooserButton->setFileName(ApplicationSettings::getDefaultSolverPath());
    this->nThreadsSpin->setValue(ApplicationSettings::getDefaultNThreads());
    this->nHistoryRecordsSpin->setValue(ApplicationSettings::getDefaultNHistoryRecords());
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

