#include "gl_shader_program.h"

#include <QFile>

#include <rbl_logger.h>

GLShaderProgram::GLShaderProgram()
{
}

GLShaderProgram::~GLShaderProgram()
{
    this->program.removeAllShaders();
}

bool GLShaderProgram::load(const QString &vertexPath, const QString &fragmentPath)
{
    // Read and log the actual source so we can confirm what the driver sees.
    QFile vf(vertexPath);
    if (!vf.open(QIODevice::ReadOnly))
    {
        RLogger::error("GLShaderProgram: cannot open vertex shader resource '%s'\n",
                       vertexPath.toUtf8().constData());
        return false;
    }
    QByteArray vertSrc = vf.readAll();
    vf.close();

    if (!this->program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc))
    {
        QString log = this->program.log();
        RLogger::error("GLShaderProgram: vertex shader '%s' compile failed (log %d chars): [%s]\n",
                       vertexPath.toUtf8().constData(),
                       log.size(),
                       log.toUtf8().constData());
        return false;
    }

    QFile ff(fragmentPath);
    if (!ff.open(QIODevice::ReadOnly))
    {
        RLogger::error("GLShaderProgram: cannot open fragment shader resource '%s'\n",
                       fragmentPath.toUtf8().constData());
        return false;
    }
    QByteArray fragSrc = ff.readAll();
    ff.close();

    if (!this->program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc))
    {
        QString log = this->program.log();
        RLogger::error("GLShaderProgram: fragment shader '%s' compile failed (log %d chars): [%s]\n",
                       fragmentPath.toUtf8().constData(),
                       log.size(),
                       log.toUtf8().constData());
        return false;
    }

    // Bind explicit attribute locations before linking.
    this->program.bindAttributeLocation("aPosition", 0);
    this->program.bindAttributeLocation("aNormal",   1);
    this->program.bindAttributeLocation("aColor",    2);
    this->program.bindAttributeLocation("aTexCoord", 3);

    if (!this->program.link())
    {
        QString log = this->program.log();
        RLogger::error("GLShaderProgram: link failed (log %d chars): [%s]\n",
                       log.size(),
                       log.toUtf8().constData());
        return false;
    }

    RLogger::info("GLShaderProgram: '%s' + '%s' linked OK\n",
                  vertexPath.toUtf8().constData(),
                  fragmentPath.toUtf8().constData());
    return true;
}

bool GLShaderProgram::loadFromSource(const char *vertSrc, const char *fragSrc)
{
    if (!this->program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc))
    {
        QString log = this->program.log();
        RLogger::error("GLShaderProgram: vertex compile failed (log %d chars): [%s]\n",
                       log.size(),
                       log.toUtf8().constData());
        return false;
    }

    if (!this->program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc))
    {
        QString log = this->program.log();
        RLogger::error("GLShaderProgram: fragment compile failed (log %d chars): [%s]\n",
                       log.size(),
                       log.toUtf8().constData());
        return false;
    }

    this->program.bindAttributeLocation("aPosition", 0);
    this->program.bindAttributeLocation("aNormal",   1);
    this->program.bindAttributeLocation("aColor",    2);
    this->program.bindAttributeLocation("aTexCoord", 3);

    if (!this->program.link())
    {
        QString log = this->program.log();
        RLogger::error("GLShaderProgram: link failed (log %d chars): [%s]\n",
                       log.size(),
                       log.toUtf8().constData());
        return false;
    }

    RLogger::info("GLShaderProgram: loaded from source OK\n");
    return true;
}

bool GLShaderProgram::bind()
{
    return this->program.bind();
}

void GLShaderProgram::release()
{
    this->program.release();
}

bool GLShaderProgram::isValid() const
{
    return this->program.isLinked();
}

void GLShaderProgram::setUniformMatrix4x4(const char *name, const QMatrix4x4 &mat)
{
    this->program.setUniformValue(name, mat);
}

void GLShaderProgram::setUniformInt(const char *name, int value)
{
    this->program.setUniformValue(name, value);
}

void GLShaderProgram::setUniformBool(const char *name, bool value)
{
    this->program.setUniformValue(name, value);
}

void GLShaderProgram::setUniformVector3D(const char *name, const QVector3D &v)
{
    this->program.setUniformValue(name, v);
}

void GLShaderProgram::setUniformVector4D(const char *name, const QVector4D &v)
{
    this->program.setUniformValue(name, v);
}
