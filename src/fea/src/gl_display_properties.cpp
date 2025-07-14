#include <rbl_error.h>
#include <rbl_logger.h>

#include <rml_file.h>

#include "application.h"

#include "gl_display_properties.h"
#include "application_settings.h"

void GLDisplayProperties::_init(const GLDisplayProperties *pDisplayProperties)
{
    if (pDisplayProperties)
    {
        this->fileName = pDisplayProperties->fileName;
    }
}

GLDisplayProperties::GLDisplayProperties()
{
    this->_init();
    this->bgColor = RApplicationSettings::getDefaultBackgroundColor(Application::instance()->getApplicationSettings()->getStyle());
}

GLDisplayProperties::GLDisplayProperties(const GLDisplayProperties &displayProperties)
    : RGLDisplayProperties(displayProperties)
{
    this->_init(&displayProperties);
}

GLDisplayProperties::~GLDisplayProperties()
{

}

GLDisplayProperties &GLDisplayProperties::operator =(const GLDisplayProperties &displayProperties)
{
    this->RGLDisplayProperties::operator =(displayProperties);
    this->_init(&displayProperties);
    return (*this);
}

const QString &GLDisplayProperties::getFileName() const
{
    return this->fileName;
}

void GLDisplayProperties::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}

void GLDisplayProperties::load(const QString &fileName)
{
    this->fileName = fileName;

    if (!RFile::exists(this->fileName))
    {
        return;
    }

    RLogger::info("Reading display properties file \'%s\'.\n",this->fileName.toUtf8().constData());
    RLogger::indent();
    try
    {
        this->read(this->fileName);
    }
    catch (const RError &rError)
    {
        RLogger::error("Failed to read the file \'%s\': %s\n",this->fileName.toUtf8().constData(),rError.getMessage().toUtf8().constData());
    }
    RLogger::unindent();
}

void GLDisplayProperties::store() const
{
    RLogger::info("Writing display properties file \'%s\'.\n",this->fileName.toUtf8().constData());
    RLogger::indent();
    try
    {
        this->write(this->fileName);
    }
    catch (const RError &rError)
    {
        RLogger::error("Failed to write the file \'%s\': %s\n",this->fileName.toUtf8().constData(),rError.getMessage().toUtf8().constData());
    }
    RLogger::unindent();
}
