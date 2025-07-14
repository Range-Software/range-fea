#include <QDir>
#include <QDesktopServices>

#include <rbl_job_manager.h>

#include <rml_file_io.h>

#include "session.h"
#include "application.h"
#include "model_io.h"

Session::Session(QObject *parent)
    : QObject{parent}
{
    R_LOG_TRACE_IN;
    this->sessionID = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
    this->drawEngine = new DrawEngine(this);

    QObject::connect(&this->pickList,&PickList::pickListChanged,this,&Session::onPickListChanged);
    QObject::connect(this,&Session::modelAdded,this,&Session::onModelAdded);
    QObject::connect(this,&Session::modelChanged,this,&Session::onModelChanged);
    QObject::connect(Application::instance()->getApplicationSettings(),
                     &ApplicationSettings::nHistoryRecordsChanged,
                     this,
                     &Session::onNHistoryRecordsChanged);
    R_LOG_TRACE_OUT;
}

const QString &Session::getID() const
{
    return this->sessionID;
}

void Session::lock()
{
    this->mutex.lock();
}

bool Session::trylock(int timeout)
{
    return this->mutex.tryLock(timeout);
}

void Session::unlock()
{
    this->mutex.unlock();
}

DrawEngine *Session::getDrawEngine()
{
    return this->drawEngine;
}

QMutex *Session::getMutex()
{
    return &this->mutex;
}

const QString &Session::getFileName() const
{
    return this->fileName;
}

uint Session::getNModels() const
{
    return uint(this->models.size());
}

const Model & Session::getModel(uint position) const
{
    return this->models[int(position)];
}

Model & Session::getModel(uint position)
{
    return this->models[int(position)];
}

const Model * Session::getModelPtr(uint position) const
{
    return &this->models[int(position)];
}

Model * Session::getModelPtr(uint position)
{
    return &this->models[int(position)];
}

void Session::addModel(const Model &model)
{
    R_LOG_TRACE_IN;
    bool locked = this->trylock();
    this->models.push_back(model);
    this->models.last().initializeMeshInput();
    if (locked)
    {
        this->unlock();
    }
    emit this->modelAdded(uint(this->models.size()-1));
    R_LOG_TRACE_OUT;
}

void Session::setModel(uint position, const Model &model)
{
    R_LOG_TRACE_IN;
    bool locked = this->trylock();
    this->models[int(position)] = model;
    if (locked)
    {
        this->unlock();
    }
    this->setModelChanged(position);
    R_LOG_TRACE_OUT;
}

void Session::removeModel(uint position)
{
    R_LOG_TRACE_IN;
    bool locked = this->trylock();
    this->models.removeAt(int(position));
    this->pickList.removeItems(position);
    if (locked)
    {
        this->unlock();
    }
    emit this->modelRemoved(position);
    R_LOG_TRACE_OUT;
}

void Session::setModelSelected(uint modelID, bool selected)
{
    R_LOG_TRACE_IN;
    for (uint i=0;i<this->getModel(modelID).getNPoints();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_POINT,i,selected,false);
    }
    for (uint i=0;i<this->getModel(modelID).getNLines();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_LINE,i,selected,false);
    }
    for (uint i=0;i<this->getModel(modelID).getNSurfaces();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_SURFACE,i,selected,false);
    }
    for (uint i=0;i<this->getModel(modelID).getNVolumes();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_VOLUME,i,selected,false);
    }
    for (uint i=0;i<this->getModel(modelID).getNVectorFields();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_VECTOR_FIELD,i,selected,false);
    }
    for (uint i=0;i<this->getModel(modelID).getNScalarFields();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_SCALAR_FIELD,i,selected,false);
    }
    for (uint i=0;i<this->getModel(modelID).getNStreamLines();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_STREAM_LINE,i,selected,false);
    }
    for (uint i=0;i<this->getModel(modelID).getNCuts();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_CUT,i,selected,false);
    }
    for (uint i=0;i<this->getModel(modelID).getNIsos();i++)
    {
        this->setEntitySelected(modelID,R_ENTITY_GROUP_ISO,i,selected,false);
    }
    this->getModel(modelID).setSelected(selected);
    this->setModelSelectionChanged(modelID);
    R_LOG_TRACE_OUT;
}

void Session::setEntitySelected(uint modelID, REntityGroupType elementGrpType, uint entityID, bool selected, bool notifyModel)
{
    R_LOG_TRACE_IN;
    if (this->getModel(modelID).getSelected(elementGrpType,entityID) != selected)
    {
        this->getModel(modelID).setSelected(elementGrpType,entityID,selected);
        emit this->entitySelectionChanged(modelID,elementGrpType,entityID,selected);
        if (notifyModel)
        {
            this->setModelSelectionChanged(modelID);
        }
    }
    R_LOG_TRACE_OUT;
}

void Session::setEntityVisible(uint modelID, REntityGroupType elementGrpType, uint entityID, bool visible)
{
    R_LOG_TRACE_IN;
    if (this->getModel(modelID).getVisible(elementGrpType,entityID) != visible)
    {
        this->getModel(modelID).setVisible(elementGrpType,entityID,visible);
        emit this->entityVisibilityChanged(modelID,elementGrpType,entityID,visible);
        emit this->modelVisibilityChanged(modelID);
    }
    R_LOG_TRACE_OUT;
}

void Session::setModelVisible(uint modelID, bool visible)
{
    R_LOG_TRACE_IN;
    for (uint i=0;i<this->getModel(modelID).getNPoints();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_POINT,i,visible);
    }
    for (uint i=0;i<this->getModel(modelID).getNLines();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_LINE,i,visible);
    }
    for (uint i=0;i<this->getModel(modelID).getNSurfaces();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_SURFACE,i,visible);
    }
    for (uint i=0;i<this->getModel(modelID).getNVolumes();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_VOLUME,i,visible);
    }
    for (uint i=0;i<this->getModel(modelID).getNVectorFields();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_VECTOR_FIELD,i,visible);
    }
    for (uint i=0;i<this->getModel(modelID).getNScalarFields();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_SCALAR_FIELD,i,visible);
    }
    for (uint i=0;i<this->getModel(modelID).getNStreamLines();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_STREAM_LINE,i,visible);
    }
    for (uint i=0;i<this->getModel(modelID).getNCuts();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_CUT,i,visible);
    }
    for (uint i=0;i<this->getModel(modelID).getNIsos();i++)
    {
        this->setEntityVisible(modelID,R_ENTITY_GROUP_ISO,i,visible);
    }
    R_LOG_TRACE_OUT;
}

QList<uint> Session::getSelectedModelIDs() const
{
//    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs;

    for (uint i=0;i<this->getNModels();i++)
    {
        // Check if model is partialy selected (NOT completely unselected)
        if (!this->getModel(i).isSelected(false))
        {
            selectedModelIDs.push_back(i);
        }
    }

//    R_LOG_TRACE_OUT;
    return selectedModelIDs;
}

QList<uint> Session::getVisibleModelIDs() const
{
    R_LOG_TRACE_IN;
    QList<uint> visibleModelIDs;

    for (uint i=0;i<this->getNModels();i++)
    {
        // Check if model is partialy visible (NOT completely unselected)
        if (!this->getModel(i).isVisible(false))
        {
            visibleModelIDs.push_back(i);
        }
    }

    R_LOG_TRACE_OUT;
    return visibleModelIDs;
}

bool Session::isModelSelected(uint modelID)
{
//    R_LOG_TRACE_IN;
    return (!this->getModel(modelID).isSelected(false));
}

QList<SessionEntityID> Session::getAllEntityIDs() const
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> allEntityIDs;

    for (uint i=0;i<this->getNModels();i++)
    {
        allEntityIDs += this->getAllEntityIDs(i);
    }
    R_LOG_TRACE_OUT;
    return allEntityIDs;
}

QList<SessionEntityID> Session::getAllEntityIDs(uint modelID) const
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> allEntityIDs;
    SessionEntityID entityID;

    entityID.setMid(modelID);

    entityID.setType(R_ENTITY_GROUP_POINT);
    for (uint j=0;j<this->getModel(modelID).getNPoints();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    entityID.setType(R_ENTITY_GROUP_LINE);
    for (uint j=0;j<this->getModel(modelID).getNLines();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    entityID.setType(R_ENTITY_GROUP_SURFACE);
    for (uint j=0;j<this->getModel(modelID).getNSurfaces();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    entityID.setType(R_ENTITY_GROUP_VOLUME);
    for (uint j=0;j<this->getModel(modelID).getNVolumes();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    entityID.setType(R_ENTITY_GROUP_VECTOR_FIELD);
    for (uint j=0;j<this->getModel(modelID).getNVectorFields();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    entityID.setType(R_ENTITY_GROUP_SCALAR_FIELD);
    for (uint j=0;j<this->getModel(modelID).getNScalarFields();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    entityID.setType(R_ENTITY_GROUP_STREAM_LINE);
    for (uint j=0;j<this->getModel(modelID).getNStreamLines();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    entityID.setType(R_ENTITY_GROUP_CUT);
    for (uint j=0;j<this->getModel(modelID).getNCuts();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    entityID.setType(R_ENTITY_GROUP_ISO);
    for (uint j=0;j<this->getModel(modelID).getNIsos();j++)
    {
        entityID.setEid(j);
        allEntityIDs.push_back(entityID);
    }

    R_LOG_TRACE_OUT;
    return allEntityIDs;
}

QList<SessionEntityID> Session::getSelectedEntityIDs() const
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs;

    for (uint i=0;i<this->getNModels();i++)
    {
        selectedEntityIDs += this->getModel(i).getSelectedEntityIDs(i);
    }
    R_LOG_TRACE_OUT;
    return selectedEntityIDs;
}

QList<SessionEntityID> Session::getVisibleEntityIDs() const
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> visibleEntityIDs;

    for (uint i=0;i<this->getNModels();i++)
    {
        visibleEntityIDs += this->getModel(i).getVisibleEntityIDs(i);
    }
    R_LOG_TRACE_OUT;
    return visibleEntityIDs;
}

QList<SessionEntityID> Session::filterSelectedEntityIDs(const QList<SessionEntityID> selectedEntityIDs, uint modelID, REntityGroupTypeMask entityTypeMask)
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntities(selectedEntityIDs);

    for (int i=selectedEntities.size()-1;i>=0;i--)
    {
        if (selectedEntities[i].getMid() == modelID
            &&
            selectedEntities[i].getType() & entityTypeMask
            &&
            R_ENTITY_GROUP_TYPE_IS_ELEMENT(selectedEntities[i].getType()))
        {
            continue;
        }
        selectedEntities.erase(selectedEntities.begin()+i);
    }

    R_LOG_TRACE_OUT;
    return selectedEntities;
}

uint Session::findModelByName(const QString &modelName)
{
    R_LOG_TRACE_IN;
    for (uint i=0;i<this->getNModels();i++)
    {
        if (this->getModel(i).getName() == modelName)
        {
            R_LOG_TRACE_OUT;
            return i;
        }
    }
    R_LOG_TRACE_OUT;
    return RConstants::eod;
}

uint Session::findModelByPtr(const Model *pModel)
{
    R_LOG_TRACE_IN;
    for (uint i=0;i<this->getNModels();i++)
    {
        if (this->getModelPtr(i) == pModel)
        {
            R_LOG_TRACE_OUT;
            return i;
        }
    }
    R_LOG_TRACE_OUT;
    return RConstants::eod;
}

void Session::setModelSelectionChanged(uint modelID)
{
    R_LOG_TRACE_IN;
    emit this->modelSelectionChanged(modelID);
    R_LOG_TRACE_OUT;
}

void Session::setDisplayPropertiesChanged(uint modelID, REntityGroupType elementGrpType, uint entityID)
{
    R_LOG_TRACE_IN;
    emit this->displayPropertiesChanged(modelID,elementGrpType,entityID);
    R_LOG_TRACE_OUT;
}

void Session::setModelChanged(uint modelID)
{
    R_LOG_TRACE_IN;
    emit this->modelChanged(modelID);
    R_LOG_TRACE_OUT;
}

void Session::setModelRenamed(uint modelID)
{
    R_LOG_TRACE_IN;
    emit this->modelRenamed(modelID);
    R_LOG_TRACE_OUT;
}

void Session::setProblemChanged(uint modelID)
{
    R_LOG_TRACE_IN;
    emit this->problemChanged(modelID);
    R_LOG_TRACE_OUT;
}

void Session::setProblemSelectionChanged(uint modelID)
{
    R_LOG_TRACE_IN;
    emit this->problemSelectionChanged(modelID);
    R_LOG_TRACE_OUT;
}

void Session::setResultsChanged(uint modelID)
{
    R_LOG_TRACE_IN;
    emit this->resultsChanged(modelID);
    R_LOG_TRACE_OUT;
}

void Session::setBoundaryConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID)
{
    R_LOG_TRACE_IN;
    emit this->boundaryConditionChanged(modelID,elementGrpType,entityID);
    R_LOG_TRACE_OUT;
}

void Session::setInitialConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID)
{
    R_LOG_TRACE_IN;
    emit this->initialConditionChanged(modelID,elementGrpType,entityID);
    R_LOG_TRACE_OUT;
}

void Session::setEnvironmentConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID)
{
    R_LOG_TRACE_IN;
    emit this->environmentConditionChanged(modelID,elementGrpType,entityID);
    R_LOG_TRACE_OUT;
}

void Session::setMaterialChanged(uint modelID, REntityGroupType elementGrpType, uint entityID)
{
    R_LOG_TRACE_IN;
    emit this->materialChanged(modelID,elementGrpType,entityID);
    R_LOG_TRACE_OUT;
}

void Session::setVariableDataChanged(uint modelID, RVariableType variableType)
{
    R_LOG_TRACE_IN;
    emit this->variableDataChanged(modelID,variableType);
    R_LOG_TRACE_OUT;
}

void Session::setVariableDataChanged(const SessionEntityID &entityID, RVariableType variableType)
{
    R_LOG_TRACE_IN;
    emit this->variableDataChanged(entityID,variableType);
    R_LOG_TRACE_OUT;
}

const PickList &Session::getPickList() const
{
    R_LOG_TRACE;
    return this->pickList;
}

PickList &Session::getPickList()
{
    R_LOG_TRACE;
    return this->pickList;
}

void Session::setBeginDrawStreamLinePosition(const RR3Vector &position)
{
    R_LOG_TRACE_IN;
    emit this->beginDrawStreamLinePosition(position);
    R_LOG_TRACE_OUT;
}

void Session::setEndDrawStreamLinePosition()
{
    R_LOG_TRACE_IN;
    emit this->endDrawStreamLinePosition();
    R_LOG_TRACE_OUT;
}

void Session::setBeginDrawScaleOrigin(const RR3Vector &position)
{
    R_LOG_TRACE_IN;
    emit this->beginDrawScaleOrigin(position);
    R_LOG_TRACE_OUT;
}

void Session::setEndDrawScaleOrigin()
{
    R_LOG_TRACE_IN;
    emit this->endDrawScaleOrigin();
    R_LOG_TRACE_OUT;
}

void Session::setBeginDrawRotationOrigin(const RR3Vector &position)
{
    R_LOG_TRACE_IN;
    emit this->beginDrawRotationOrigin(position);
    R_LOG_TRACE_OUT;
}

void Session::setEndDrawRotationOrigin()
{
    R_LOG_TRACE_IN;
    emit this->endDrawRotationOrigin();
    R_LOG_TRACE_OUT;
}

void Session::setBeginDrawLocalDirections(const QList<RLocalDirection> &localDirections)
{
    R_LOG_TRACE_IN;
    emit this->beginDrawLocalDirections(localDirections);
    R_LOG_TRACE_OUT;
}

void Session::setEndDrawLocalDirections()
{
    R_LOG_TRACE_IN;
    emit this->endDrawLocalDirections();
    R_LOG_TRACE_OUT;
}

void Session::setBeginDrawCutPlane(const RPlane &plane)
{
    R_LOG_TRACE_IN;
    emit this->beginDrawCutPlane(plane);
    R_LOG_TRACE_OUT;
}

void Session::setEndDrawCutPlane()
{
    R_LOG_TRACE_IN;
    emit this->endDrawCutPlane();
    R_LOG_TRACE_OUT;
}

void Session::setBeginDrawMoveNodes(const QMap<SessionNodeID, RR3Vector> &nodesToMove)
{
    R_LOG_TRACE_IN;
    emit this->beginDrawMoveNodes(nodesToMove);
    R_LOG_TRACE_OUT;
}

void Session::setEndDrawMoveNodes()
{
    R_LOG_TRACE_IN;
    emit this->endDrawMoveNodes();
    R_LOG_TRACE_OUT;
}

void Session::setTakeScreenShot(uint modelID, const QString &screenShotFileName, bool blocking)
{
    R_LOG_TRACE_IN;
    if (blocking)
    {
        emit this->takeScreenShotBlocking(modelID, screenShotFileName);
    }
    else
    {
        emit this->takeScreenShot(modelID, screenShotFileName);
    }
    R_LOG_TRACE_OUT;
}

void Session::readModels(const QStringList &fileNames)
{
    R_LOG_TRACE_IN;
    RLogger::info("Loading %u files\n",fileNames.size());
    RLogger::indent();
    for (int i=0;i<fileNames.size();i++)
    {
        RLogger::info("Loading %u: \'%s\'\n",i+1,fileNames.at(i).toUtf8().constData());

        if (!fileNames.at(i).isEmpty())
        {
            // Read model.
            ModelIO *modelIO = new ModelIO(MODEL_IO_OPEN, fileNames.at(i));
            modelIO->setAutoDelete(true);

            RJobManager::getInstance().submit(modelIO);
        }
    }
    RLogger::unindent();
    R_LOG_TRACE_OUT;
}

void Session::read(const QString &fileName)
{
    R_LOG_TRACE_IN;
    if (fileName.isEmpty())
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"No file name was provided.");
    }

    RLogger::info("Reading session file \'%s\'.\n",fileName.toUtf8().constData());
    RLogger::indent();

    this->fileName = fileName;

    RFile sessionFile(this->fileName,RFile::ASCII);

    if (!sessionFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        RLogger::unindent();
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::OpenFile,R_ERROR_REF,"Failed to open the file \'%s\'.",this->fileName.toUtf8().constData());
    }

    RLogger::info("Reading session file records.\n");
    RLogger::indent();

    uint nModels = 0;
    RFileIO::readAscii(sessionFile,nModels);
    RLogger::info("Detected %u file records.\n",nModels);
    QStringList modelFileNames;
    for (uint i=0;i<nModels;i++)
    {
        QString modelFileName;
        RFileIO::readAscii(sessionFile,modelFileName);

        if (modelFileName.isEmpty())
        {
            RLogger::info("File record %u is empty.\n",i+1);
        }
        else
        {
            RLogger::info("File record %u \'%s\'\n",i+1,modelFileName.toUtf8().constData());
            modelFileNames.append(modelFileName);
        }
    }
    RLogger::unindent();

    sessionFile.close();

    this->readModels(modelFileNames);

    RLogger::unindent();
    R_LOG_TRACE_OUT;
}

void Session::write(const QString &fileName, bool writeModels)
{
    R_LOG_TRACE_IN;
    RLogger::info("Writing session file \'%s\'.\n",fileName.toUtf8().constData());

    if (fileName.isEmpty())
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"No file name was provided.");
    }

    this->fileName = fileName;

    RSaveFile sessionFile(this->fileName,RSaveFile::ASCII);

    if (!sessionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::OpenFile,R_ERROR_REF,"Failed to open the file \'%s\'.",this->fileName.toUtf8().constData());
    }

    RFileIO::writeAscii(sessionFile,this->getNModels(),true);
    for (uint i=0;i<this->getNModels();i++)
    {
        RFileIO::writeAscii(sessionFile,"\"" + this->getModel(i).getFileName() + "\"",true);

        if (writeModels)
        {
            if (this->getModel(i).getFileName().isEmpty())
            {
                QDir dataDir(Application::instance()->getApplicationSettings()->getDataDir());
                QString binaryExtension = RModel::getDefaultFileExtension(true);
                QString modelName = this->getModel(i).getName();
                if (modelName.length() == 0)
                {
                    modelName = "Model_" + QString::number(i+1);
                }
                QString modelFileName = dataDir.filePath(modelName + "." + binaryExtension);
                this->getModel(i).setFileName(modelFileName);
            }

            // Write model.
            ModelIO *modelIO = new ModelIO(MODEL_IO_SAVE, this->getModel(i).getFileName(), this->getModelPtr(i));
            modelIO->setAutoDelete(true);

            RJobManager::getInstance().submit(modelIO);
        }
    }

    sessionFile.commit();
    R_LOG_TRACE_OUT;
}

void Session::clear()
{
    R_LOG_TRACE_IN;
    RLogger::info("Closing session\n");
    this->fileName.clear();

    while (this->getNModels() > 0)
    {
        RLogger::indent();
        RLogger::info("Closing model \'%s\'\n",this->getModel(this->getNModels()-1).getName().toUtf8().constData());

        Application::instance()->getSession()->removeModel(this->getNModels()-1);

        RLogger::unindent();
    }
    R_LOG_TRACE_OUT;
}

void Session::storeCurentModelVersion(uint modelID, const QString &message)
{
    R_LOG_TRACE_IN;
    this->getModel(modelID).storeCurentVersion(Application::instance()->getApplicationSettings()->getNHistoryRecords(),message);
    R_LOG_TRACE_OUT;
}

bool Session::isUndoAvailable() const
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = this->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        if (this->getModel(selectedModelIDs[i]).getUndoStackSize() > 0)
        {
            R_LOG_TRACE_OUT;
            return true;
        }
    }
    R_LOG_TRACE_OUT;
    return false;
}

bool Session::isRedoAvailable() const
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = this->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        if (this->getModel(selectedModelIDs[i]).getRedoStackSize() > 0)
        {
            R_LOG_TRACE_OUT;
            return true;
        }
    }
    R_LOG_TRACE_OUT;
    return false;
}

QString Session::getUndoTooltip() const
{
    R_LOG_TRACE_IN;
    QString undoMessage;

    QList<uint> selectedModelIDs = this->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        if (!undoMessage.isEmpty())
        {
            undoMessage += "; ";
        }
        undoMessage += this->getModel(selectedModelIDs[i]).getUndoActionMessage();
    }
    undoMessage = tr("Undo") + (undoMessage.isEmpty() ? QString() : ": " + undoMessage);
    R_LOG_TRACE_OUT;
    return undoMessage;
}

QString Session::getRedoTooltip() const
{
    R_LOG_TRACE_IN;
    QString redoMessage;

    QList<uint> selectedModelIDs = this->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        if (!redoMessage.isEmpty())
        {
            redoMessage += "; ";
        }
        redoMessage += this->getModel(selectedModelIDs[i]).getRedoActionMessage();
    }
    redoMessage = tr("Redo") + (redoMessage.isEmpty() ? QString() : ": " + redoMessage);
    R_LOG_TRACE_OUT;
    return redoMessage;
}

QString Session::getDefaultFileExtension()
{
    R_LOG_TRACE;
    return "ras";
}

QString Session::getDefaultFileName()
{
    R_LOG_TRACE_IN;
    QDir sesionDir = Application::instance()->getApplicationSettings()->getSessionDir();
    R_LOG_TRACE_OUT;
    return sesionDir.filePath(QString("default.") + Session::getDefaultFileExtension());
}

bool Session::selectedModelsHasEntities(REntityGroupTypeMask entityTypeMask)
{
//    R_LOG_TRACE_IN;
    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        if (((entityTypeMask & R_ENTITY_GROUP_POINT) && Application::instance()->getSession()->getModel(modelID).getNPoints())
            ||
            ((entityTypeMask & R_ENTITY_GROUP_LINE) && Application::instance()->getSession()->getModel(modelID).getNLines())
            ||
            ((entityTypeMask & R_ENTITY_GROUP_SURFACE) && Application::instance()->getSession()->getModel(modelID).getNSurfaces())
            ||
            ((entityTypeMask & R_ENTITY_GROUP_VOLUME) && Application::instance()->getSession()->getModel(modelID).getNVolumes())
            ||
            ((entityTypeMask & R_ENTITY_GROUP_CUT) && Application::instance()->getSession()->getModel(modelID).getNCuts())
            ||
            ((entityTypeMask & R_ENTITY_GROUP_ISO) && Application::instance()->getSession()->getModel(modelID).getNIsos())
            ||
            ((entityTypeMask & R_ENTITY_GROUP_STREAM_LINE) && Application::instance()->getSession()->getModel(modelID).getNStreamLines())
            ||
            ((entityTypeMask & R_ENTITY_GROUP_SCALAR_FIELD) && Application::instance()->getSession()->getModel(modelID).getNScalarFields())
            ||
            ((entityTypeMask & R_ENTITY_GROUP_VECTOR_FIELD) && Application::instance()->getSession()->getModel(modelID).getNVectorFields()))
        {
//            R_LOG_TRACE_OUT;
            return true;
        }
    }
//    R_LOG_TRACE_OUT;
    return false;
}

bool Session::selectedModelsHasEntitySelected(REntityGroupType entityGroupType)
{
//    R_LOG_TRACE_IN;
    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        foreach (SessionEntityID sessionID, Application::instance()->getSession()->getModel(modelID).getSelectedEntityIDs(modelID))
        {
            if (sessionID.getType() == entityGroupType)
            {
//                R_LOG_TRACE_OUT;
                return true;
            }
        }
    }
//    R_LOG_TRACE_OUT;
    return false;
}

void Session::onPickListChanged()
{
    R_LOG_TRACE_IN;
    emit this->pickListChanged();
    R_LOG_TRACE_OUT;
}

void Session::onModelAdded(uint modelID)
{
    R_LOG_TRACE_IN;
    this->getModel(modelID).createDependentEntities();
    R_LOG_TRACE_OUT;
}

void Session::onModelChanged(uint modelID)
{
    R_LOG_TRACE_IN;
    this->getModel(modelID).createDependentEntities();
    R_LOG_TRACE_OUT;
}

void Session::onNHistoryRecordsChanged(uint nHistoryRecords)
{
    R_LOG_TRACE_IN;
    for (uint i=0;i<this->getNModels();i++)
    {
        this->getModel(i).updateHistoryStackSize(nHistoryRecords);
    }
    emit this->nHistoryRecordsChanged(nHistoryRecords);
    R_LOG_TRACE_OUT;
}

void Session::onSoftwareUpdateFileSaved(uint downloadID, const QString &fileName)
{
    R_LOG_TRACE_IN;
    RLogger::info("Opening downloaded file \'%s\' (id=\'%u\')\n",fileName.toUtf8().constData(),downloadID);

    if (!QDesktopServices::openUrl(QUrl("file:///" + fileName)))
    {
        RLogger::error("Failed to open the file \'%s\'\n",fileName.toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}
