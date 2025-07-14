#include <QFileInfo>
#include <QFileDialog>

#include <rml_file_manager.h>

#include "model_io.h"
#include "application.h"

ModelIO::ModelIO(ModelIOType type, const QString &fileName, Model *pModel)
{
    R_LOG_TRACE_IN;
    this->type = type;
    this->fileName = fileName;
    this->pModel = pModel;
    R_LOG_TRACE_OUT;
}

QString ModelIO::findNewModelName(const QString &fileName)
{
    R_LOG_TRACE_IN;
    QString modelBaseName = QFileInfo(fileName).baseName();
    QString modelName = modelBaseName;
    uint nTries = 2;
    while (Application::instance()->getSession()->findModelByName(modelName) != RConstants::eod)
    {
        modelName = modelBaseName + " [" + QString::number(nTries) + "]";
        nTries++;
    }
    R_LOG_TRACE_OUT;
    return modelName;
}

QString ModelIO::getModelSaveName(const ApplicationSettings *mainSettings, QWidget *parent, const Model &rModel, bool saveAs)
{
    R_LOG_TRACE_IN;
    QDir dataDir(mainSettings->getDataDir());
    QString binaryExtension = RModel::getDefaultFileExtension(true);
    QString asciiExtension = RModel::getDefaultFileExtension(false);
    QString dialogDesc = "Range model files (*." + binaryExtension + " *." + asciiExtension + ");;Any files (*)";

    QString fileName = RFileManager::getFileNameWithOutTimeStep(rModel.getFileName());
    if (saveAs)
    {
        fileName = QFileDialog::getSaveFileName(parent,
                                                "Save model as",
                                                fileName,
                                                dialogDesc);
    }
    else
    {
        if (fileName.isEmpty())
        {
            fileName = dataDir.filePath(rModel.getName() + "." + binaryExtension);
            fileName = QFileDialog::getSaveFileName(parent,
                                                    "Save model",
                                                    fileName,
                                                    dialogDesc);
        }
    }

    R_LOG_TRACE_OUT;
    return fileName;
}

void ModelIO::save(Model &rModel, const QString &fileName, bool writeLinkFile)
{
    R_LOG_TRACE_IN;
    RLogger::info("Saving model to file \'%s\'.\n", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        rModel.write(fileName,writeLinkFile);

        RLogger::unindent();
        RLogger::info("Model has been successfully saved to file \'%s\'.\n", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to save model to file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

void ModelIO::open(Model &rModel, const QString &fileName)
{
    R_LOG_TRACE_IN;
    RLogger::info("Loading model file \'%s\'.\n", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        rModel.read(fileName);

        RLogger::unindent();
        RLogger::info("Model has been successfully loaded from file \'%s\'.\n", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to load model from file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

void ModelIO::update(Model &rModel, const QString &fileName)
{
    R_LOG_TRACE_IN;
    RLogger::info("Updating model from file \'%s\'.\n", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        RModel tmpModel;

        tmpModel.read(fileName);

        rModel.update(tmpModel);

        RLogger::unindent();
        RLogger::info("Model has been successfully updated from file \'%s\'.\n", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to update from file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

void ModelIO::exportMsh(Model &rModel, const QString &fileName)
{
    R_LOG_TRACE_IN;
    RModelMsh modelMsh;
    rModel.exportTo(modelMsh);

    RLogger::info("Exporting MSH file \'%s\'.\n", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        modelMsh.write(fileName);

        RLogger::unindent();
        RLogger::info("MSH file \'%s\' has been successfully exported.\n", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to export MSH file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

void ModelIO::importMsh(Model &rModel, const QString &fileName)
{
    R_LOG_TRACE_IN;
    RModelMsh modelMsh;

    RLogger::info("Importing MSH file \'%s\'.\n", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        modelMsh.read(fileName);

        rModel = Model(modelMsh);

        RLogger::unindent();
        RLogger::info("MSH file \'%s\' has been successfully imported.\n", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to import MSH file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

void ModelIO::exportRaw(Model &rModel, const QString &fileName)
{
    R_LOG_TRACE_IN;
    RModelRaw modelRaw;
    rModel.exportTo(modelRaw);

    RLogger::info("Exporting RAW file \'%s\'.\n", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        modelRaw.write(fileName);

        RLogger::unindent();
        RLogger::info("RAW file \'%s\' has been successfully exported.\n", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to export RAW file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

void ModelIO::importRaw(Model &rModel, const QString &fileName)
{
    R_LOG_TRACE_IN;
    RModelRaw modelRaw;
    double tolerance = 0.0;

    RLogger::info("Importing RAW file \'%s\'.\n", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        modelRaw.read(fileName, tolerance);

        rModel = Model(modelRaw,ModelIO::findNewModelName(fileName).toUtf8().constData());

        RLogger::unindent();
        RLogger::info("RAW file \'%s\' has been successfully imported.\n", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to import RAW file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

void ModelIO::exportStl(Model &rModel, const QString &fileName, bool binary)
{
    R_LOG_TRACE_IN;
    RModelStl modelStl;
    rModel.exportTo(modelStl);

    RLogger::info("Exporting STL (%s) file \'%s\'.\n", binary?"binary":"ascii", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        modelStl.write(fileName,binary);

        RLogger::unindent();
        RLogger::info("STL (%s) file \'%s\' has been successfully exported.\n", binary?"binary":"ascii", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to export STL file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

void ModelIO::importStl(Model &rModel, const QString &fileName)
{
    R_LOG_TRACE_IN;
    RModelStl modelStl;
    double tolerance = 0.0;

    RLogger::info("Importing STL file \'%s\'.\n", fileName.toUtf8().constData());
    RLogger::indent();

    try
    {
        modelStl.read(fileName, tolerance);

        rModel = Model(modelStl);

        RLogger::unindent();
        RLogger::info("STL file \'%s\' has been successfully imported.\n", fileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to import STL file \'%s\'.\n", fileName.toUtf8().constData());
        throw error;
    }
    R_LOG_TRACE_OUT;
}

int ModelIO::perform()
{
    R_LOG_TRACE_IN;
    try
    {
        if (this->type == MODEL_IO_ADD)
        {
            Application::instance()->getSession()->addModel(*this->pModel);
            Application::instance()->getSession()->setModelSelected(Application::instance()->getSession()->getNModels()-1,true);
            delete this->pModel;
        }
        else if (this->type == MODEL_IO_SAVE)
        {
            ModelIO::save((*this->pModel),this->fileName,true);
        }
        else if (this->type == MODEL_IO_OPEN)
        {
            Model model;
            ModelIO::open(model,this->fileName);
            Application::instance()->getSession()->addModel(model);
        }
        else if (this->type == MODEL_IO_UPDATE)
        {
            uint updateModelID = Application::instance()->getSession()->findModelByPtr(this->pModel);
            if (updateModelID != RConstants::eod)
            {
                Application::instance()->getSession()->storeCurentModelVersion(updateModelID,tr("Update model from file"));
            }
            ModelIO::update((*this->pModel),this->fileName);
            if (updateModelID != RConstants::eod)
            {
                Application::instance()->getSession()->setModelChanged(updateModelID);
            }
        }
        else if (this->type == MODEL_IO_MSH_EXPORT)
        {
            ModelIO::exportMsh((*this->pModel),this->fileName);
        }
        else if (this->type == MODEL_IO_MSH_IMPORT)
        {
            Model model;
            ModelIO::importMsh(model,this->fileName);
            Application::instance()->getSession()->addModel(model);
        }
        else if (this->type == MODEL_IO_RAW_EXPORT)
        {
            ModelIO::exportRaw((*this->pModel),this->fileName);
        }
        else if (this->type == MODEL_IO_RAW_IMPORT)
        {
            Model model;
            ModelIO::importRaw(model,this->fileName);
            Application::instance()->getSession()->addModel(model);
        }
        else if (this->type == MODEL_IO_STL_ASCII_EXPORT)
        {
            ModelIO::exportStl((*this->pModel),this->fileName,false);
        }
        else if (this->type == MODEL_IO_STL_BINARY_EXPORT)
        {
            ModelIO::exportStl((*this->pModel),this->fileName);
        }
        else if (this->type == MODEL_IO_STL_IMPORT)
        {
            Model model;
            ModelIO::importStl(model,this->fileName);
            Application::instance()->getSession()->addModel(model);
        }
    }
    catch (const RError &error)
    {
        RLogger::error("%s File: '%s'\n",
                       error.getMessage().toUtf8().constData(),
                       fileName.toUtf8().constData());
        R_LOG_TRACE_RETURN(1);
    }
    R_LOG_TRACE_RETURN(0);
}
