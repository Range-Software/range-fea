#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include "mesh_generator_dialog.h"
#include "mesh_generator.h"
#include "application.h"

MeshGeneratorDialog::MeshGeneratorDialog(uint modelID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID)
{
    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    this->meshInput = rModel.getMeshInput();

    this->setWindowTitle(tr("Genenerate 3D mesh"));

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    this->surfaceIntegrityCheck = new QCheckBox(tr("Check surface integrity"));
    mainLayout->addWidget(this->surfaceIntegrityCheck);
    this->surfaceIntegrityCheck->setChecked(rModel.getNVolumes() == 0);

    this->reconstructCheck = new QCheckBox(tr("Reconstruct mesh (reuse current nodes)"));
    mainLayout->addWidget(this->reconstructCheck);
    this->reconstructCheck->setChecked(this->meshInput.getReconstruct());
    this->reconstructCheck->setEnabled(rModel.getNVolumes() > 0);

    this->keepResultsCheck = new QCheckBox(tr("Keep computed results"));
    mainLayout->addWidget(this->keepResultsCheck);
    this->keepResultsCheck->setChecked(this->meshInput.getKeepResults());
    this->keepResultsCheck->setEnabled(rModel.getNVariables() > 0);

    this->qualityMeshGroupBox = new QGroupBox(tr("Quality mesh"));
    mainLayout->addWidget(this->qualityMeshGroupBox);
    this->qualityMeshGroupBox->setCheckable(true);
    this->qualityMeshGroupBox->setChecked(this->meshInput.getQualityMesh());

    QGridLayout *qualityMeshLayout = new QGridLayout;
    this->qualityMeshGroupBox->setLayout(qualityMeshLayout);

    QLabel *label = new QLabel(tr("Maximum element volume") + ":");
    qualityMeshLayout->addWidget(label,0,0,1,1);

    this->volumeConstraintEdit = new ValueLineEdit(0.0,1e99);
    qualityMeshLayout->addWidget(this->volumeConstraintEdit,0,1,1,1);
    this->volumeConstraintEdit->setValue(this->meshInput.getVolumeConstraint());

    QLabel *volumeConstraintUnitsLabel = new QLabel("[m^3]");
    qualityMeshLayout->addWidget(volumeConstraintUnitsLabel,0,2,1,1);

    this->meshSizeFunctionGroupBox = new QGroupBox(tr("Generate mesh size function"));
    qualityMeshLayout->addWidget(this->meshSizeFunctionGroupBox,1,0,1,3);
    this->meshSizeFunctionGroupBox->setCheckable(true);
    this->meshSizeFunctionGroupBox->setChecked(false);
    this->meshSizeFunctionGroupBox->setEnabled(rModel.getNVariables() > 0);

    QGridLayout *meshSizeFunctionLayout = new QGridLayout;
    this->meshSizeFunctionGroupBox->setLayout(meshSizeFunctionLayout);

    QLabel *meshSizeFunctionLabel = new QLabel(tr("Source variable") + ":");
    meshSizeFunctionLayout->addWidget(meshSizeFunctionLabel,0,0,1,1);

    this->meshSizeFunctionSourceComboBox = new QComboBox;
    meshSizeFunctionLayout->addWidget(this->meshSizeFunctionSourceComboBox,0,1,1,2);

    for (uint i=0;i<rModel.getNVariables();i++)
    {
        this->meshSizeFunctionSourceComboBox->addItem(rModel.getVariable(i).getName(),
                                                      QVariant(int(rModel.getVariable(i).getType())));
    }

    double initEdgeLenght = std::cbrt(12.0 * this->meshInput.getVolumeConstraint() / std::sqrt(2.0));

    QLabel *meshSizeFunctionMinValueLabel = new QLabel(tr("Minimum edge length") + ":");
    meshSizeFunctionLayout->addWidget(meshSizeFunctionMinValueLabel,1,0,1,1);

    this->meshSizeFunctionMinValueEdit = new ValueLineEdit(0.0,1e10);
    this->meshSizeFunctionMinValueEdit->setValue(initEdgeLenght*0.1);
    meshSizeFunctionLayout->addWidget(this->meshSizeFunctionMinValueEdit,1,1,1,1);

    QLabel *meshSizeFunctionMinValueUnitsLabel = new QLabel("[m]");
    meshSizeFunctionLayout->addWidget(meshSizeFunctionMinValueUnitsLabel,1,2,1,1);

    QLabel *meshSizeFunctionMaxValueLabel = new QLabel(tr("Maximum edge length") + ":");
    meshSizeFunctionLayout->addWidget(meshSizeFunctionMaxValueLabel,2,0,1,1);

    this->meshSizeFunctionMaxValueEdit = new ValueLineEdit(0.0,1e10);
    this->meshSizeFunctionMaxValueEdit->setValue(initEdgeLenght);
    meshSizeFunctionLayout->addWidget(this->meshSizeFunctionMaxValueEdit,2,1,1,1);

    QLabel *meshSizeFunctionMaxValueUnitsLabel = new QLabel("[m]");
    meshSizeFunctionLayout->addWidget(meshSizeFunctionMaxValueUnitsLabel,2,2,1,1);

    this->tetgenParamsGroupBox = new QGroupBox(tr("TetGen parameters"));
    this->tetgenParamsGroupBox->setCheckable(true);
    this->tetgenParamsGroupBox->setChecked(false);
    mainLayout->addWidget(this->tetgenParamsGroupBox);

    QVBoxLayout *tetgenParamsLayout = new QVBoxLayout;
    this->tetgenParamsGroupBox->setLayout(tetgenParamsLayout);

    this->tetgenParamsEdit = new QLineEdit(Application::instance()->getSession()->getModel(this->modelID).generateMeshTetGenInputParams(this->meshInput));
    tetgenParamsLayout->addWidget(this->tetgenParamsEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->surfaceIntegrityCheck,&QCheckBox::checkStateChanged,this,&MeshGeneratorDialog::onSurfaceIntegrityStateChanged);
    QObject::connect(this->qualityMeshGroupBox,&QGroupBox::clicked,this,&MeshGeneratorDialog::onQualityMeshGroupBoxClicked);
    QObject::connect(this->volumeConstraintEdit,&ValueLineEdit::valueChanged,this,&MeshGeneratorDialog::onVolumeConstraintValueChanged);
    QObject::connect(this->meshSizeFunctionMinValueEdit,&ValueLineEdit::valueChanged,this,&MeshGeneratorDialog::onVolumeConstraintValueChanged);
    QObject::connect(this->meshSizeFunctionMaxValueEdit,&ValueLineEdit::valueChanged,this,&MeshGeneratorDialog::onVolumeConstraintValueChanged);
    QObject::connect(this->reconstructCheck,&QCheckBox::checkStateChanged,this,&MeshGeneratorDialog::onReconstructStateChanged);
    QObject::connect(this->keepResultsCheck,&QCheckBox::checkStateChanged,this,&MeshGeneratorDialog::onKeepResultsStateChanged);
    QObject::connect(this->tetgenParamsGroupBox,&QGroupBox::clicked,this,&MeshGeneratorDialog::onTetgenParamsGroupBoxClicked);
}

int MeshGeneratorDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        this->updateMeshInput();

        if (this->meshInput.getUseSizeFunction())
        {
            RVariableType meshSizeFunctionVariableType = RVariableType(this->meshSizeFunctionSourceComboBox->currentData().toInt());
            double meshSizeFunctionMinValue = this->meshSizeFunctionMinValueEdit->getValue();
            double meshSizeFunctionMaxValue = this->meshSizeFunctionMaxValueEdit->getValue();

            Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

            QSet<RVariableType> variableList;
            variableList.insert(meshSizeFunctionVariableType);
            RRVector meshSizeFunction = rModel.generateMeshSizeFunction(variableList,
                                                                        meshSizeFunctionMinValue,
                                                                        meshSizeFunctionMaxValue,
                                                                        0.5);
            this->meshInput.setSizeFunctionValues(meshSizeFunction);
        }

        Application::instance()->getSession()->getModel(this->modelID).setMeshInput(this->meshInput);

        MeshGenerator *meshGenerator = new MeshGenerator(this->modelID);
        RJobManager::getInstance().submit(meshGenerator);
    }
    return retVal;
}

void MeshGeneratorDialog::updateMeshInput()
{
    this->meshInput.setSurfaceIntegrityCheck(this->surfaceIntegrityCheck->isChecked());
    this->meshInput.setQualityMesh(this->qualityMeshGroupBox->isChecked());
    this->meshInput.setUseSizeFunction(this->qualityMeshGroupBox->isChecked() && this->meshSizeFunctionGroupBox->isChecked());
    this->meshInput.setVolumeConstraint(this->volumeConstraintEdit->getValue());
    this->meshInput.setReconstruct(this->reconstructCheck->isChecked());
    this->meshInput.setKeepResults(this->keepResultsCheck->isChecked());

    if (this->meshSizeFunctionMaxValueEdit->getValue() > this->meshSizeFunctionMaxValueEdit->getMaximum())
    {
        this->meshSizeFunctionMaxValueEdit->setValue(this->meshSizeFunctionMaxValueEdit->getMaximum());
    }

    this->meshSizeFunctionMinValueEdit->setRange(this->meshSizeFunctionMaxValueEdit->getMinimum(),
                                                 this->meshSizeFunctionMaxValueEdit->getValue());
    if (this->meshSizeFunctionMinValueEdit->getValue() > this->meshSizeFunctionMaxValueEdit->getValue())
    {
        this->meshSizeFunctionMinValueEdit->setValue(this->meshSizeFunctionMinValueEdit->getMaximum());
    }


    this->meshInput.setUseTetGenInputParams(this->tetgenParamsGroupBox->isChecked());
    if (!this->tetgenParamsGroupBox->isChecked())
    {
        // Update TetGen parameters line edit.
        bool oldState = this->tetgenParamsEdit->blockSignals(true);
        this->tetgenParamsEdit->setText(Application::instance()->getSession()->getModel(this->modelID).generateMeshTetGenInputParams(this->meshInput));
        this->tetgenParamsEdit->blockSignals(oldState);
    }
    this->meshInput.setTetGenInputParams(this->tetgenParamsEdit->text());
}

void MeshGeneratorDialog::onSurfaceIntegrityStateChanged(Qt::CheckState)
{
    this->updateMeshInput();
}

void MeshGeneratorDialog::onQualityMeshGroupBoxClicked(bool)
{
    this->updateMeshInput();
}

void MeshGeneratorDialog::onVolumeConstraintValueChanged(double)
{
    this->updateMeshInput();
}

void MeshGeneratorDialog::onMeshSizeFunctionMinValueChanged(double)
{
    this->updateMeshInput();
}

void MeshGeneratorDialog::onMeshSizeFunctionMaxValueChanged(double)
{
    this->updateMeshInput();
}

void MeshGeneratorDialog::onReconstructStateChanged(Qt::CheckState)
{
    this->updateMeshInput();
}

void MeshGeneratorDialog::onKeepResultsStateChanged(Qt::CheckState)
{
    this->updateMeshInput();
}

void MeshGeneratorDialog::onTetgenParamsGroupBoxClicked(bool)
{
    this->updateMeshInput();
}
