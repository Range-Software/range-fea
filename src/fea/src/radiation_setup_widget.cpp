#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

#include <rbl_job_manager.h>
#include <rml_file_manager.h>
#include <rgl_message_box.h>

#include "application.h"
#include "model_action.h"
#include "radiation_setup_widget.h"

RadiationSetupWidget::RadiationSetupWidget(const RRadiationSetup &radiationSetup, const QString &defaultViewFactorFileName, QWidget *parent)
    : QWidget(parent)
    , radiationSetup(radiationSetup)
    , defaultViewFactorFileName(defaultViewFactorFileName)
{
    RRadiationResolution radiationResolution = this->radiationSetup.getResolution();

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    const Model &rModel = Application::instance()->getSession()->getModel(modelIDs[0]);

    QString viewFactorFile(this->radiationSetup.getViewFactorMatrixFile());
    if (viewFactorFile.isEmpty())
    {
        viewFactorFile = rModel.buildDataFileName(RViewFactorMatrix::getDefaultFileExtension(true),rModel.getTimeSolver().getEnabled());
        this->radiationSetup.setViewFactorMatrixFile(viewFactorFile);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QGroupBox *radiationGroupBox = new QGroupBox(tr("Radiation setup"));
    mainLayout->addWidget(radiationGroupBox);

    QGridLayout *radiationLayout = new QGridLayout;
    radiationGroupBox->setLayout(radiationLayout);

    uint crow = 0;

    QLabel *resolutionLabel = new QLabel(tr("Resolution"));
    resolutionLabel->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding));
    radiationLayout->addWidget(resolutionLabel,crow,0,1,1);

    QComboBox *resolutionCombo = new QComboBox;
    resolutionCombo->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    resolutionCombo->addItem(RRadiationSetup::getResolutionText(R_RADIATION_RESOLUTION_LOW));
    resolutionCombo->addItem(RRadiationSetup::getResolutionText(R_RADIATION_RESOLUTION_MEDIUM));
    resolutionCombo->addItem(RRadiationSetup::getResolutionText(R_RADIATION_RESOLUTION_HIGH));
    if (radiationResolution <= R_RADIATION_RESOLUTION_LOW)
    {
        resolutionCombo->setCurrentIndex(0);
    }
    else if (radiationResolution >= R_RADIATION_RESOLUTION_HIGH)
    {
        resolutionCombo->setCurrentIndex(2);
    }
    else
    {
        resolutionCombo->setCurrentIndex(1);
    }
    radiationLayout->addWidget(resolutionCombo,crow,1,1,1);
    crow++;

    this->connect(resolutionCombo,SIGNAL(currentIndexChanged(int)),SLOT(onResolutionChanged(int)));


    QGroupBox *customVfFile = new QGroupBox(tr("Custom view-factor file"));
    customVfFile->setCheckable(true);
    radiationLayout->addWidget(customVfFile,crow,0,1,2);
    crow++;

    QObject::connect(customVfFile,&QGroupBox::toggled,this,&RadiationSetupWidget::onCustomViewFactorFileToggled);

    QHBoxLayout *customVfLayout = new QHBoxLayout;
    customVfFile->setLayout(customVfLayout);

    this->viewFactorFileButton = new RFileChooserButton(QString(),
                                                        RFileChooserButton::OpenFile,
                                                        tr("Select view-factor file"),
                                                        viewFactorFile,
                                                        tr("View-factor files") + " (*." + RViewFactorMatrix::getDefaultFileExtension(true) + ");;" + tr("Any files") + " (*)");
    customVfLayout->addWidget(this->viewFactorFileButton);

    QObject::connect(this->viewFactorFileButton,
                     &RFileChooserButton::fileNameChanged,
                     this,
                     &RadiationSetupWidget::onViewFactorFileChanged);

    bool patchesOK = false, viewFactorsOK = false;

    RViewFactorMatrixHeader viewFactorMatrixHeader;

    rModel.generateViewFactorMatrixHeader(viewFactorMatrixHeader);

    if (viewFactorMatrixHeader.getPatchInput() == rModel.getViewFactorMatrix().getHeader().getPatchInput() &&
        viewFactorMatrixHeader.getPatchInput().size() == rModel.getNSurfaces() &&
        viewFactorMatrixHeader.getNElements() == rModel.getNElements())
    {
        patchesOK = true;
        if (viewFactorMatrixHeader.getHemicubeResolution() == rModel.getViewFactorMatrix().getHeader().getHemicubeResolution() &&
            rModel.getViewFactorMatrix().getPatchBook().getNPatches() == rModel.getViewFactorMatrix().size())
        {
            viewFactorsOK = true;
        }
    }

    if (!patchesOK)
    {
        QLabel *patchStatusLabel = new QLabel;
        patchStatusLabel->setText("<font color=#ff0000>" + tr("Surface patches need to be generated.") + "</font>");
        radiationLayout->addWidget(patchStatusLabel,crow++,0,1,2);

        QPushButton *regeneratePatches = new QPushButton(tr("Regenerate patches"));
        radiationLayout->addWidget(regeneratePatches,crow++,0,1,2);

        QObject::connect(regeneratePatches,
                         &QPushButton::clicked,
                         this,
                         &RadiationSetupWidget::regeneratePatches);
    }

    if (!viewFactorsOK)
    {
        QLabel *viewFactorsStatusLabel = new QLabel;
        viewFactorsStatusLabel->setText("<font color=#ff0000>" + tr("View-factors need to be calculated.") + "</font>");
        radiationLayout->addWidget(viewFactorsStatusLabel,crow++,0,1,2);

        QPushButton *recalculateViewFactors = new QPushButton(tr("Recalculate view-factors"));
        radiationLayout->addWidget(recalculateViewFactors,crow++,0,1,2);

        QObject::connect(recalculateViewFactors,
                         &QPushButton::clicked,
                         this,
                         &RadiationSetupWidget::recalculateViewFactors);
    }

    QPushButton *clearViewFactorMatrix = new QPushButton(tr("Clear view-factor matrix"));
    radiationLayout->addWidget(clearViewFactorMatrix,crow++,0,1,2);

    QObject::connect(clearViewFactorMatrix,
                     &QPushButton::clicked,
                     this,
                     &RadiationSetupWidget::clearViewFactorMatrix);
}

void RadiationSetupWidget::onResolutionChanged(int index)
{
    RRadiationResolution radiationResolution;
    if (index == 0)
    {
        radiationResolution = R_RADIATION_RESOLUTION_LOW;
    }
    else if (index == 1)
    {
        radiationResolution = R_RADIATION_RESOLUTION_MEDIUM;
    }
    else
    {
        radiationResolution = R_RADIATION_RESOLUTION_HIGH;
    }

    this->radiationSetup.setResolution(radiationResolution);
    emit this->changed(this->radiationSetup);
}

void RadiationSetupWidget::onCustomViewFactorFileToggled(bool checked)
{
    if (!checked)
    {
//        this->viewFactorFileButton->setFileName(this->defaultViewFactorFileName);
        this->radiationSetup.setViewFactorMatrixFile(this->defaultViewFactorFileName);
        emit this->changed(this->radiationSetup);
    }
}

void RadiationSetupWidget::onViewFactorFileChanged(const QString &fileName)
{
    if (fileName.isEmpty())
    {
        return;
    }

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        Model &rModel = Application::instance()->getSession()->getModel(modelIDs[i]);
        rModel.getProblemSetup().getRadiationSetup().setViewFactorMatrixFile(fileName);

        try
        {
            if (rModel.getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_RADIATIVE_HEAT)
            {
                rModel.loadViewFactorMatrix();
            }
            else
            {
                rModel.unloadViewFactorMatrix();
            }
            Application::instance()->getSession()->setProblemChanged(modelIDs[i]);
        }
        catch (const RError &error)
        {
            RLogger::error("Failed un/load view-factor matrix file. %s\n",error.getMessage().toUtf8().constData());
        }
    }
}

void RadiationSetupWidget::regeneratePatches()
{
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setGeneratePatches();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
}

void RadiationSetupWidget::recalculateViewFactors()
{
    if (RMessageBox::question(Application::instance()->getMainWindow(),
                              tr("Calculate view-factors"),
                              tr("Are you sure you want to calculate view-factors?") + "<br/>" + tr("This operation can take long time.")) != RMessageBox::Yes)
    {
        return;
    }

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setCalculateViewFactors();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
}

void RadiationSetupWidget::clearViewFactorMatrix()
{
    if (RMessageBox::question(Application::instance()->getMainWindow(),
                              tr("Clear view-factor matrix"),
                              tr("Are you sure you want to clear view-factor matrix?") + "<br/>"
                            + tr("This operation will clear all generated patches and calculated view-factors.") + " "
                            + tr("It will also delete all view-factor matrix file.")) != RMessageBox::Yes)
    {
        return;
    }

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        Application::instance()->getSession()->getModel(modelIDs[i]).getViewFactorMatrix().clear();
        try
        {
            const QString &fileName = Application::instance()->getSession()->getModel(modelIDs[i]).getProblemSetup().getRadiationSetup().getViewFactorMatrixFile();
            RLogger::info("Removing file \'%s\'\n",fileName.toUtf8().constData());
            RFileManager::remove(fileName);
        }
        catch (const RError &error)
        {
            RLogger::error("Failed to delete view-factor matrix file. %s",error.getMessage().toUtf8().constData());
        }

        Application::instance()->getSession()->setModelChanged(modelIDs[i]);
    }
}
