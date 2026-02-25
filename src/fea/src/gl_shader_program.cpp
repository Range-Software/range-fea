#include "gl_shader_program.h"

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
    if (!this->program.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexPath))
    {
        RLogger::error("GLShaderProgram: vertex shader '%s' failed: %s\n",
                       vertexPath.toUtf8().constData(),
                       this->program.log().toUtf8().constData());
        return false;
    }

    if (!this->program.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentPath))
    {
        RLogger::error("GLShaderProgram: fragment shader '%s' failed: %s\n",
                       fragmentPath.toUtf8().constData(),
                       this->program.log().toUtf8().constData());
        return false;
    }

    if (!this->program.link())
    {
        RLogger::error("GLShaderProgram: link failed: %s\n",
                       this->program.log().toUtf8().constData());
        return false;
    }

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
