#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QList>

#include "draw_engine.h"
#include "pick_list.h"
#include "model.h"
#include "session_entity_id.h"
#include "session_node_id.h"

class Session : public QObject
{

    Q_OBJECT

    protected:

        //! Session ID.
        QString sessionID;
        //! Session filename.
        QString fileName;
        //! List of models.
        QList<Model> models;
        //! Pick list.
        PickList pickList;
        //! Draw mode engine.
        DrawEngine *drawEngine;
        //! Session mutex.
        QMutex mutex;

    public:

        //! Constructor.
        Session(QObject *parent = nullptr);

        //! Return session ID.
        const QString &getID() const;

        //! Lock session.
        void lock();

        //! Trylock session.
        bool trylock(int timeout = 0);

        //! Unlock session.
        void unlock();

        //! Return pointer to draw mode engine.
        DrawEngine *getDrawEngine();

        //! Return pointer to mutex;
        QMutex *getMutex();

        //! Return session filename.
        const QString &getFileName() const;

        //! Return number of models in session.
        uint getNModels() const;

        //! Return const reference to model at given position.
        const Model & getModel(uint position) const;

        //! Return reference to model at given position.
        Model & getModel(uint position);

        //! Return const pointer to model at given position.
        const Model * getModelPtr(uint position) const;

        //! Return pointer to model at given position.
        Model * getModelPtr(uint position);

        //! Add model to the list.
        void addModel(const Model &model);

        //! Set model to the list at given position.
        void setModel(uint position, const Model &model);

        //! Remove model from the list.
        void removeModel(uint position);

        //! Set model (all entities) as de/selected.
        void setModelSelected(uint modelID, bool selected);

        //! Set entity as de/selected.
        void setEntitySelected(uint modelID, REntityGroupType elementGrpType, uint entityID, bool selected, bool notifyModel = true);

        //! Set entity visibility on/off.
        void setEntityVisible(uint modelID, REntityGroupType elementGrpType, uint entityID, bool visible);

        //! Set model (all entities) visibility on/off.
        void setModelVisible(uint modelID, bool visible);

        //! Return list of positions of selected models.
        QList<uint> getSelectedModelIDs() const;

        //! Return list of positions of visible models.
        QList<uint> getVisibleModelIDs() const;

        //! Return true if given model ID is selected.
        bool isModelSelected(uint modelID);

        //! Return list of all entity IDs.
        QList<SessionEntityID> getAllEntityIDs() const;

        //! Return list of all entity IDs for given model ID.
        QList<SessionEntityID> getAllEntityIDs(uint modelID) const;

        //! Return list of all selected entity IDs.
        QList<SessionEntityID> getSelectedEntityIDs() const;

        //! Return list of all visible entity IDs.
        QList<SessionEntityID> getVisibleEntityIDs() const;

        //! Filter selected entity IDs
        static QList<SessionEntityID> filterSelectedEntityIDs(const QList<SessionEntityID> selectedEntityIDs,
                                                              uint modelID,
                                                              REntityGroupTypeMask entityTypeMask);

        //! Find model by its name and return its position.
        //! If no model with such name could be found position
        //! will be set to be equal to number of models in session.
        uint findModelByName(const QString &modelName);

        //! Find model by pointer to instance and return its position.
        //! If no model could be found position will be set to be equal to number of models in session.
        uint findModelByPtr(const Model *pModel);

        //! Emit signal that model selection with given model ID has changed.
        void setModelSelectionChanged(uint modelID);

        //! Emit signal that model entity display properties has changed.
        void setDisplayPropertiesChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Emit signal that model has changed.
        void setModelChanged(uint modelID);

        //! Emit signal that model has been renamed.
        void setModelRenamed(uint modelID);

        //! Emit signal that problem for given model ID has changed.
        void setProblemChanged(uint modelID);

        //! Emit signal that problem selection for given model ID has changed.
        void setProblemSelectionChanged(uint modelID);

        //! Emit signal that results for given model ID has changed.
        void setResultsChanged(uint modelID);

        //! Emit signal that boundary condition has changed.
        void setBoundaryConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Emit signal that initial condition has changed.
        void setInitialConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Emit signal that environment condition has changed.
        void setEnvironmentConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Emit signal that material has changed.
        void setMaterialChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Emit signal that variable data has changed.
        void setVariableDataChanged(uint modelID, RVariableType variableType);

        //! Emit signal that variable data has changed.
        void setVariableDataChanged(const SessionEntityID &entityID, RVariableType variableType);

        //! Return const reference to pick list.
        const PickList & getPickList() const;

        //! Return reference to pick list.
        PickList & getPickList();

        //! Emit signal that stream line position drawing should begin.
        void setBeginDrawStreamLinePosition(const RR3Vector &position);

        //! Emit signal that stream line position drawing should end.
        void setEndDrawStreamLinePosition();

        //! Emit signal that geometry scale origin drawing should begin.
        void setBeginDrawScaleOrigin(const RR3Vector &position);

        //! Emit signal that geometry scale origin drawing should end.
        void setEndDrawScaleOrigin();

        //! Emit signal that geometry rotation origin drawing should begin.
        void setBeginDrawRotationOrigin(const RR3Vector &position);

        //! Emit signal that geometry rotation origin drawing should end.
        void setEndDrawRotationOrigin();

        //! Emit signal that local rotation drawing should begin.
        void setBeginDrawLocalDirections(const QList<RLocalDirection> &localDirections);

        //! Emit signal that local rotation drawing should end.
        void setEndDrawLocalDirections();

        //! Emit signal that cut plane drawing should begin.
        void setBeginDrawCutPlane(const RPlane &plane);

        //! Emit signal that cut plane drawing should end.
        void setEndDrawCutPlane();

        //! Emit signal that move nodes drawing should begin.
        void setBeginDrawMoveNodes(const QMap<SessionNodeID, RR3Vector> &nodesToMove);

        //! Emit signal that move nodes drawing should end.
        void setEndDrawMoveNodes();

        //! Take screenshot of given model ID.
        void setTakeScreenShot(uint modelID, const QString &screenShotFileName, bool blocking);

        //! Read models.
        void readModels(const QStringList &fileNames);

        //! Read session from the file.
        void read(const QString &fileName);

        //! Write session to the file.
        void write(const QString &fileName, bool writeModels = false);

        //! Clear session content.
        void clear();

        //! Store current model to history.
        void storeCurentModelVersion(uint modelID, const QString &message);

        //! Check if undo is available.
        bool isUndoAvailable() const;

        //! Check if redo is available.
        bool isRedoAvailable() const;

        //! Return undo tooltip text.
        QString getUndoTooltip() const;

        //! Return redo tooltip text.
        QString getRedoTooltip() const;

        //! Return default file extension.
        static QString getDefaultFileExtension();

        //! Return default file name.
        static QString getDefaultFileName();

        //! Check if entity exist in selected model.
        static bool selectedModelsHasEntities(REntityGroupTypeMask entityTypeMask = R_ENTITY_GROUP_ELEMENT);

        //! Check if entity exist and is selected in selected model.
        static bool selectedModelsHasEntitySelected(REntityGroupType entityGroupType);

    signals:

        //! Emitted when new model has been added to the session.
        void modelAdded(uint modelID);

        //! Emitted when model has been changed in the session.
        void modelChanged(uint modelID);

        //! Emitted when model has been removed from the session.
        void modelRemoved(uint modelID);

        //! Emitted when model has been renamed.
        void modelRenamed(uint modelID);

        //! Problem has changed.
        void problemChanged(uint modelID);

        //! Problem selection has changed.
        void problemSelectionChanged(uint modelID);

        //! Problem has changed.
        void resultsChanged(uint modelID);

        //! Emitted when model selection has changed.
        void modelSelectionChanged(uint modelID);

        //! Emitted when entity selection has changed.
        void entitySelectionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID, bool selected);

        //! Emitted when model visibility has changed.
        void modelVisibilityChanged(uint modelID);

        //! Emitted when entity visibility has changed.
        void entityVisibilityChanged(uint modelID, REntityGroupType elementGrpType, uint entityID, bool visible);

        //! Emitted when entity display properties has changed.
        void displayPropertiesChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Boundary condition has changed.
        void boundaryConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Initial condition has changed.
        void initialConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Environment condition has changed.
        void environmentConditionChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Material has changed.
        void materialChanged(uint modelID, REntityGroupType elementGrpType, uint entityID);

        //! Variable data has changed.
        void variableDataChanged(uint modelID, RVariableType variableType);

        //! Variable data has changed.
        void variableDataChanged(const SessionEntityID &entityID, RVariableType variableType);

        //! Pick list has changed.
        void pickListChanged();

        //! Stream line position drawing should begin.
        void beginDrawStreamLinePosition(const RR3Vector &position);

        //! Stream line position drawing should end.
        void endDrawStreamLinePosition();

        //! Geometry scale origin drawing should begin.
        void beginDrawScaleOrigin(const RR3Vector &position);

        //! Geometry scale origin drawing should end.
        void endDrawScaleOrigin();

        //! Geometry rotation origin drawing should begin.
        void beginDrawRotationOrigin(const RR3Vector &position);

        //! Geometry rotation origin drawing should end.
        void endDrawRotationOrigin();

        //! Local direction drawing should begin.
        void beginDrawLocalDirections(const QList<RLocalDirection> &localDiretions);

        //! Local direction drawing should end.
        void endDrawLocalDirections();

        //! Cut plane drawing should begin.
        void beginDrawCutPlane(const RPlane &plane);

        //! Cut plane drawing should end.
        void endDrawCutPlane();

        //! Move nodes drawing should begin.
        void beginDrawMoveNodes(const QMap<SessionNodeID, RR3Vector> &nodesToMove);

        //! Move nodes drawing should end.
        void endDrawMoveNodes();

        //! Take screenshot of given model ID.
        void takeScreenShot(uint modelID, const QString &screenShotFileName);

        //! Take screenshot of given model ID.
        void takeScreenShotBlocking(uint modelID, const QString &screenShotFileName);

        //! Number of history records has changed.
        void nHistoryRecordsChanged(uint nHistoryRecords);

    public slots:

        //! Catch file saved signal for software update file.
        void onSoftwareUpdateFileSaved(uint downloadID, const QString &fileName);

    protected slots:

        //! Called when picListChanged signal is emitted.
        void onPickListChanged();

        //! Called when modelAdded signal is emitted.
        void onModelAdded(uint modelID);

        //! Called when modelChanged signal is emitted.
        void onModelChanged(uint modelID);

        //! Called when number of history records has changed.
        void onNHistoryRecordsChanged(uint nHistoryRecords);

};

#endif /* SESSION_H */
