#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include "merge_entity_dialog.h"
#include "application.h"
#include "model_action.h"

MergeEntityDialog::MergeEntityDialog(REntityGroupTypeMask entityTypeMask, QWidget *parent) :
    QDialog(parent),
    entityTypeMask(entityTypeMask)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Merge entities"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    this->setLayout (mainLayout);

    QLabel *listLabel = new QLabel(tr("Following entities will be merged") + ":");
    mainLayout->addWidget(listLabel);

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QList<SessionEntityID> selectedEntities = this->findEntities(modelIDs[i],this->entityTypeMask);

        QList<SessionEntityID> selectedPoints = SessionEntityID::filterList(selectedEntities,R_ENTITY_GROUP_POINT);
        QList<SessionEntityID> selectedLines = SessionEntityID::filterList(selectedEntities,R_ENTITY_GROUP_LINE);
        QList<SessionEntityID> selectedSurfaces = SessionEntityID::filterList(selectedEntities,R_ENTITY_GROUP_SURFACE);
        QList<SessionEntityID> selectedVolumes = SessionEntityID::filterList(selectedEntities,R_ENTITY_GROUP_VOLUME);

        Model &rModel = Application::instance()->getSession()->getModel(modelIDs[i]);

        QGroupBox *modelGroupBox= new QGroupBox;
        modelGroupBox->setTitle(rModel.getName());
        mainLayout->addWidget(modelGroupBox);

        QGridLayout *modelGroupLayout = new QGridLayout;
        modelGroupBox->setLayout(modelGroupLayout);

        QString text = "<dl>";
        if (selectedPoints.size() >= 2)
        {
            text += "<dt>" + tr("Points") + ":";
            text += "<dd>";
            for (int j=0;j<selectedPoints.size();j++)
            {
                text += rModel.getPoint(selectedPoints[j].getEid()).getName() + QString(",&nbsp;");
            }
        }
        if (selectedLines.size() >= 2)
        {
            text += "<dt>" + tr("Lines") + ":";
            text += "<dd>";
            for (int j=0;j<selectedLines.size();j++)
            {
                text += rModel.getLine(selectedLines[j].getEid()).getName() + QString(",&nbsp;");
            }
        }
        if (selectedSurfaces.size() >= 2)
        {
            text += "<dt>" + tr("Surfaces") + ":";
            text += "<dd>";
            for (int j=0;j<selectedSurfaces.size();j++)
            {
                text += rModel.getSurface(selectedSurfaces[j].getEid()).getName() + QString(",&nbsp;");
            }
        }
        if (selectedVolumes.size() >= 2)
        {
            text += "<dt>" + tr("Volumes") + ":";
            text += "<dd>";
            for (int j=0;j<selectedVolumes.size();j++)
            {
                text += rModel.getVolume(selectedVolumes[j].getEid()).getName() + QString(",&nbsp;");
            }
        }
        text += "</dl>";

        QLabel *modelLabel = new QLabel(text);
        modelGroupLayout->addWidget(modelLabel,1,0,1,1);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int MergeEntityDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
        for (int i=0;i<modelIDs.size();i++)
        {
            QList<SessionEntityID> selectedEntities = SessionEntityID::filterList(this->findEntities(modelIDs[i],this->entityTypeMask));

            if (selectedEntities.size() < 2)
            {
                continue;
            }
            ModelActionInput modelActionInput(modelIDs[i]);
            modelActionInput.setMergeEntities(selectedEntities);

            ModelAction *modelAction = new ModelAction;
            modelAction->setAutoDelete(true);
            modelAction->addAction(modelActionInput);
            RJobManager::getInstance().submit(modelAction);
        }
    }

    return retVal;
}

bool MergeEntityDialog::entitiesSelected(REntityGroupTypeMask entityTypeMask)
{
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QList<SessionEntityID> selectedEntities = Session::filterSelectedEntityIDs(Application::instance()->getSession()->getSelectedEntityIDs(),modelIDs[i],entityTypeMask);

        QList<SessionEntityID> selectedPoints = SessionEntityID::filterList(selectedEntities,R_ENTITY_GROUP_POINT);
        QList<SessionEntityID> selectedLines = SessionEntityID::filterList(selectedEntities,R_ENTITY_GROUP_LINE);
        QList<SessionEntityID> selectedSurfaces = SessionEntityID::filterList(selectedEntities,R_ENTITY_GROUP_SURFACE);
        QList<SessionEntityID> selectedVolumes = SessionEntityID::filterList(selectedEntities,R_ENTITY_GROUP_VOLUME);
        if (selectedPoints.size() >= 2 || selectedLines.size() >= 2 || selectedSurfaces.size() >= 2 || selectedVolumes.size() >= 2)
        {
            return true;
        }
    }
    return false;
}

QList<SessionEntityID> MergeEntityDialog::findEntities(uint mid, REntityGroupTypeMask entityTypeMask)
{
    return Session::filterSelectedEntityIDs(Application::instance()->getSession()->getSelectedEntityIDs(),mid,entityTypeMask);
}
