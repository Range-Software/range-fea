#include <rbl_error.h>

#include "draw_engine.h"
#include "application.h"

DrawEngine::DrawEngine(QObject *parent) :
    QObject(parent)
{
    R_LOG_TRACE;
}

void DrawEngine::start()
{
    R_LOG_TRACE_IN;
    if (this->isRunning)
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::Application,R_ERROR_REF,"Engine is already running");
    }
    R_LOG_TRACE_OUT;
}

void DrawEngine::stop()
{
    R_LOG_TRACE_IN;
    if (!this->isRunning)
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::Application,R_ERROR_REF,"Engine is not running");
    }
    R_LOG_TRACE_OUT;

}

void DrawEngine::processObjects(const QList<uint> &modelIDs, bool mergeNearNodes, double tolerance, bool findNearest)
{
    R_LOG_TRACE_IN;
    for (int i=0;i<modelIDs.size();i++)
    {
        Application::instance()->getSession()->storeCurentModelVersion(modelIDs[i],tr("Draw object"));
        for (int j=0;j<this->objects.size();j++)
        {
            // Change alpha of each entity to 255
            Model &model = this->objects[j]->getModel();
            for (uint i=0;i<model.getNEntityGroups();i++)
            {
                REntityGroup *rEntityGroup = model.getEntityGroupPtr(i);
                int r,g,b,a;
                rEntityGroup->getData().getColor(r,g,b,a);
                a = 255;
                rEntityGroup->getData().setColor(r,g,b,a);
            }
            Application::instance()->getSession()->getModel(modelIDs[i]).insertModel(this->objects[j]->getModel(),mergeNearNodes,tolerance,findNearest);
        }
        Application::instance()->getSession()->setModelChanged(modelIDs[i]);
    }
    qDeleteAll(this->objects);
    this->objects.clear();
    emit this->objectsRemoved();
    R_LOG_TRACE_OUT;
}

void DrawEngine::addObject(DrawEngineObject *object)
{
    R_LOG_TRACE_IN;
    object->setParent(this);
    this->objects.push_back(object);
    emit this->objectAdded();
    R_LOG_TRACE_OUT;
}

void DrawEngine::removeObject(uint position)
{
    R_LOG_TRACE_IN;
    DrawEngineObject *pObject = this->objects[position];
    this->objects.removeAt(position);
    delete pObject;
    emit this->objectRemoved(position);
    R_LOG_TRACE_OUT;
}

uint DrawEngine::getNObjects() const
{
    R_LOG_TRACE;
    return uint(this->objects.size());
}

const DrawEngineObject *DrawEngine::getObject(uint position) const
{
    R_LOG_TRACE;
    return this->objects[position];
}

DrawEngineObject *DrawEngine::getObject(uint position)
{
    R_LOG_TRACE;
    return this->objects[position];
}

void DrawEngine::setObjectChanged(uint position)
{
    R_LOG_TRACE;
    emit this->objectChanged(position);
}

bool DrawEngine::findLimits(RLimitBox &limitBox) const
{
    R_LOG_TRACE_IN;
    bool limitFound = false;
    foreach (const auto object, this->objects)
    {
        RLimitBox lb = object->getModel().findNodeLimits();
        if (!limitFound)
        {
            limitBox = lb;
        }
        limitBox.merge(lb);
        limitFound = true;
    }
    R_LOG_TRACE_OUT;
    return limitFound;
}
