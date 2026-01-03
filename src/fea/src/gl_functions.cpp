#include <QString>

#include <rbl_logger.h>

#include "gl_functions.h"
#include "gl_vertex_buffer.h"

static bool insideBeginEnd = false;
static GLVertexBuffer *currentVBO = nullptr;
static bool vboRecordingMode = false;

void GLFunctions::printError(const char *command, const char *file, unsigned int line)
{
    if (insideBeginEnd)
    {
        return;
    }

    GLenum glErrorNum = glGetError();

    QString errorMessage;

    switch (glErrorNum)
    {
        case GL_NO_ERROR:
        {
//            No error has been recorded.
            break;
        }
        case GL_INVALID_ENUM:
        {
            errorMessage += "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        }
        case GL_INVALID_VALUE:
        {
            errorMessage += "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        }
        case GL_INVALID_OPERATION:
        {
            errorMessage += "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        }
        case GL_INVALID_FRAMEBUFFER_OPERATION:
        {
            errorMessage += "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        }
        case GL_OUT_OF_MEMORY:
        {
            errorMessage += "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
            break;
        }
        case GL_STACK_UNDERFLOW:
        {
            errorMessage += "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
            break;
        }
        case GL_STACK_OVERFLOW:
        {
            errorMessage += "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
            break;
        }
        default:
        {
            errorMessage += "Unknown OpenGL error";
            break;
        }
    }

    if (errorMessage.length() > 0)
    {
        RLogger::warning("OpenGL call \'%s\' failed at %s:%d > %s\n",command,file,line,errorMessage.toUtf8().constData());
    }
}

void GLFunctions::begin(GLenum mode)
{
    insideBeginEnd = true;
    if (vboRecordingMode && currentVBO)
    {
        currentVBO->beginRecording(mode);
    }
    else
    {
        glBegin(mode);
    }
}

void GLFunctions::end()
{
    if (vboRecordingMode && currentVBO)
    {
        currentVBO->endRecording();
    }
    else
    {
        GL_SAFE_CALL(glEnd());
    }
    insideBeginEnd = false;
}

void GLFunctions::normal3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (vboRecordingMode && currentVBO)
    {
        currentVBO->setNormal(x, y, z);
    }
    else
    {
        glNormal3f(x, y, z);
    }
}

void GLFunctions::normal3d(GLdouble x, GLdouble y, GLdouble z)
{
    normal3f(GLfloat(x), GLfloat(y), GLfloat(z));
}

void GLFunctions::vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (vboRecordingMode && currentVBO)
    {
        currentVBO->addVertex(x, y, z);
    }
    else
    {
        glVertex3f(x, y, z);
    }
}

void GLFunctions::vertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    vertex3f(GLfloat(x), GLfloat(y), GLfloat(z));
}

void GLFunctions::color4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    if (vboRecordingMode && currentVBO)
    {
        currentVBO->setColor(r, g, b, a);
    }
    else
    {
        glColor4ub(r, g, b, a);
    }
}

void GLFunctions::texCoord1f(GLfloat t)
{
    if (vboRecordingMode && currentVBO)
    {
        currentVBO->setTexCoord(t);
    }
    else
    {
        glTexCoord1f(t);
    }
}

void GLFunctions::beginVBORecording(GLVertexBuffer *buffer)
{
    currentVBO = buffer;
    vboRecordingMode = (buffer != nullptr);
}

void GLFunctions::endVBORecording()
{
    currentVBO = nullptr;
    vboRecordingMode = false;
}

bool GLFunctions::isRecordingVBO()
{
    return vboRecordingMode;
}

GLVertexBuffer *GLFunctions::getCurrentVBO()
{
    return currentVBO;
}
