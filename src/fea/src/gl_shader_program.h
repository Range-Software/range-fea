#ifndef GL_SHADER_PROGRAM_H
#define GL_SHADER_PROGRAM_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QVector4D>

//! Thin wrapper around QOpenGLShaderProgram.
//! Provides a stable API for loading GLSL shaders from Qt resource files and
//! setting the standard uniforms used across the rendering pipeline.
class GLShaderProgram
{
    public:

        //! Constructor.
        GLShaderProgram();

        //! Destructor.
        ~GLShaderProgram();

        //! Load vertex and fragment shaders from Qt resource paths (e.g. ":/shaders/main.vert").
        //! Must be called with a current OpenGL context.
        bool load(const QString &vertexPath, const QString &fragmentPath);

        //! Bind the program for rendering. Returns false if the program is not linked.
        bool bind();

        //! Release the program (restore fixed-function pipeline).
        void release();

        //! Returns true if the program compiled and linked successfully.
        bool isValid() const;

        //! Upload a 4×4 matrix uniform.
        void setUniformMatrix4x4(const char *name, const QMatrix4x4 &mat);

        //! Upload an integer uniform.
        void setUniformInt(const char *name, int value);

        //! Upload a boolean uniform.
        void setUniformBool(const char *name, bool value);

        //! Upload a vec3 uniform.
        void setUniformVector3D(const char *name, const QVector3D &v);

        //! Upload a vec4 uniform.
        void setUniformVector4D(const char *name, const QVector4D &v);

    private:

        QOpenGLShaderProgram program;
};

#endif // GL_SHADER_PROGRAM_H
