#include "draw_engine_raw.h"

DrawEngineRaw::DrawEngineRaw(QObject *parent) :
    DrawEngineObject(parent)
{
    this->name = tr("Geometry");

    QString rawContent("# RAW model file content\n"
                       "# Comments start with hash sign '#'\n"
                       "# Comments and empty lines are ignored\n"
                       "# \n"
                       "# Point element\n"
                       "# 0.0 0.0 0.0\n"
                       "# \n"
                       "# Line element\n"
                       "# 0.0 0.0 0.0  1.0 0.0 0.0\n"
                       "# \n"
                       "# Triangle element\n"
                       "# 0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 0.0\n"
                       "# \n"
                       "# Quadrilateral element\n"
                       "# 0.0 0.0 0.0  1.0 0.0 0.0  1.0 1.0 0.0  0.0 1.0 0.0\n");

    this->inputParameters.append(DrawEngineInput(rawContent,tr("RAW model"),tr("RAW model file content."),"ASCII"));

    this->updateModel();
}

RModelRaw DrawEngineRaw::generate(const QString &rawModelContent)
{
    RModelRaw model;

    QString content(rawModelContent);
    QTextStream textStream(&content);

    try
    {
        model.readTextStream(textStream,RConstants::eps);
    }
    catch (const RError &rError)
    {
        RLogger::warning("Failed to generate object from RAW model input. %s\n",rError.getMessage().toUtf8().constData());
    }

    return model;
}

RModelRaw DrawEngineRaw::generate() const
{
    return DrawEngineRaw::generate(this->inputParameters.at(0).toText());
}
