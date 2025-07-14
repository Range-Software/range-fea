#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include <rgl_message_box.h>

#include "application.h"
#include "mark_entity_dialog.h"
#include "model_action.h"

double MarkEntityDialog::separationAngle = 45.0;

MarkEntityDialog::MarkEntityDialog(REntityGroupType entityType, QWidget *parent)
    : QDialog(parent)
    , entityType(entityType)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QString windowTitleStr = tr("Mark entity") + " (" + RElementGroup::getTypeName(entityType) + ")";
    this->setWindowTitle(tr(windowTitleStr.toUtf8().constData()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    this->autoMarkRadio = new QRadioButton("Auto-mark all entities");
    mainLayout->addWidget(this->autoMarkRadio);

    QVector<QString> keyMouseCombinations = GLActionEvent::findKeyMouseCombinations(GLActionEvent::PickElement);
    QString markPickMessage = tr("To mark elements at least one element must be picked.") + " " + tr("Use") + " ";
    bool isFirst = true;
    foreach (auto keyMouseCombination,keyMouseCombinations)
    {
        if (!isFirst)
        {
            markPickMessage += " " + tr("or") + " ";
        }
        markPickMessage += "<strong>" + keyMouseCombination + "</strong>";
        isFirst = true;
    }
    markPickMessage += " " + tr("to pick element") + ".";
    this->pickedMarkRadio = new QRadioButton("Mark only selected and related elements");
    this->pickedMarkRadio->setToolTip(markPickMessage);
    mainLayout->addWidget(this->pickedMarkRadio);

    if (Application::instance()->getSession()->getPickList().isEmpty())
    {
        this->autoMarkRadio->setChecked(true);
        this->pickedMarkRadio->setDisabled(true);
    }
    else
    {
        this->pickedMarkRadio->setChecked(true);
    }

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    this->separationAngleSpin = new QSpinBox;
    this->separationAngleSpin->setRange(0,360);
    this->separationAngleSpin->setValue(MarkEntityDialog::separationAngle);
    formLayout->addRow(tr("Separation angle") + ":",this->separationAngleSpin);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int MarkEntityDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        this->separationAngle = double(this->separationAngleSpin->value());
        bool markSuccess = false;
        if (this->autoMarkRadio->isChecked() || Application::instance()->getSession()->getPickList().isEmpty())
        {
            // Automark all selected models.
            QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
            for (int i=0;i<modelIDs.size();i++)
            {
                RLogger::info("Auto-marking model \'%s\'\n",Application::instance()->getSession()->getModel(modelIDs[i]).getName().toUtf8().constData());
                if (this->autoMarkEntities(modelIDs[i]))
                {
                    markSuccess = true;
                }
            }
        }
        else
        {
            // Mark each model at a time.
            QList<uint> modelIDs = Application::instance()->getSession()->getPickList().getModelIDs();
            for (int i=0;i<modelIDs.size();i++)
            {
                RLogger::info("Marking model \'%s\'\n",Application::instance()->getSession()->getModel(modelIDs[i]).getName().toUtf8().constData());
                if (this->markEntities(modelIDs[i],Application::instance()->getSession()->getPickList().getItems(modelIDs[i])))
                {
                    markSuccess = true;
                }
            }
        }
        if (markSuccess)
        {
            Application::instance()->getSession()->getPickList().clear();
        }
    }

    return retVal;
}

bool MarkEntityDialog::autoMarkEntities(uint modelID)
{
    ModelActionInput modelActionInput(modelID);

    switch (this->entityType)
    {
        case R_ENTITY_GROUP_POINT:
            break;
        case R_ENTITY_GROUP_LINE:
            break;
        case R_ENTITY_GROUP_SURFACE:
            modelActionInput.setAutoMarkSurfaces(this->separationAngle);
            break;
        case R_ENTITY_GROUP_VOLUME:
            break;
        default:
            RLogger::error("Unknown element group \'%d\'",int(this->entityType));
            return false;
    }

    ModelAction *modelAction = new ModelAction;
    modelAction->setAutoDelete(true);
    modelAction->addAction(modelActionInput);
    RJobManager::getInstance().submit(modelAction);

    return true;
}

bool MarkEntityDialog::markEntities(uint modelID, const QVector<PickItem> &pickList)
{
    QList<uint> elementIDs;
    for (int j=0;j<pickList.size();j++)
    {
        const SessionEntityID &pickEntityID = pickList[j].getEntityID();
        if (pickEntityID.getMid() == modelID && pickEntityID.getType() == this->entityType)
        {
            uint elementID = pickList[j].getElementID();
            if (elementID < Application::instance()->getSession()->getModel(modelID).getNElements() &&
                R_ELEMENT_TYPE_IS_SURFACE(Application::instance()->getSession()->getModel(modelID).getElement(elementID).getType()))
            {
                elementIDs.append(elementID);
            }
        }
    }

    if (elementIDs.size() == 0)
    {
        QVector<QString> keyMouseCombinations = GLActionEvent::findKeyMouseCombinations(GLActionEvent::PickElement);
        QString markPickMessage = tr("To mark elements at least one element must be picked.") + " " + tr("Use") + " ";
        bool isFirst = true;
        foreach (auto keyMouseCombination,keyMouseCombinations)
        {
            if (!isFirst)
            {
                markPickMessage += " " + tr("or") + " ";
            }
            markPickMessage += "<strong>" + keyMouseCombination + "</strong>";
            isFirst = true;
        }
        markPickMessage += " " + tr("to pick element") + ".";

        RMessageBox::warning(this,tr("No elements were picked."),markPickMessage);
        return false;
    }

    ModelActionInput modelActionInput(modelID);

    switch (this->entityType)
    {
        case R_ENTITY_GROUP_POINT:
            break;
        case R_ENTITY_GROUP_LINE:
            break;
        case R_ENTITY_GROUP_SURFACE:
            modelActionInput.setMarkSurfaces(this->separationAngle,elementIDs);
            break;
        case R_ENTITY_GROUP_VOLUME:
            break;
        default:
            RLogger::error("Unknown element group \'%d\'",int(this->entityType));
            return false;
    }

    ModelAction *modelAction = new ModelAction;
    modelAction->setAutoDelete(true);
    modelAction->addAction(modelActionInput);
    RJobManager::getInstance().submit(modelAction);

    return true;
}
