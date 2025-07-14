#include <rbl_logger.h>

#include <rml_mesh_generator.h>

#include "application.h"
#include "mesh_generator.h"

MeshGenerator::MeshGenerator(uint modelID)
    : modelID(modelID)
{
}

int MeshGenerator::perform()
{
    Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Generate 3D mesh"));

    Model model = Application::instance()->getSession()->getModel(this->modelID);
    RLogger::info("Generating 3D mesh for model \'%s\'\n",model.getName().toUtf8().constData());
    RLogger::indent();

    try
    {
        if (model.getMeshInput().getSurfaceIntegrityCheck())
        {
            uint nIntersected = model.breakIntersectedElements(10);
            if (nIntersected > 0)
            {
                RLogger::warning("Number of intersected elements found = %u.\n",nIntersected);
            }
        }
        RMeshGenerator::generate(model.getMeshInput(),model);
        model.clearEdgeNodes();
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to generate 3D mesh: %s\n",error.getMessage().toUtf8().constData());
        RLogger::unindent();
        return 1;
    }

    RLogger::unindent();

    RLogger::info("Consolidating mesh internal structures.\n");
    RLogger::indent();

    try
    {
        model.consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to consolidate internal structures: %s\n",error.getMessage().toUtf8().constData());
        RLogger::unindent();
        return 1;
    }

    RLogger::unindent();

    Application::instance()->getSession()->setModel(this->modelID,model);
    return 0;
}
