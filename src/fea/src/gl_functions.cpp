#include <QString>

#include <rbl_logger.h>

#include "gl_functions.h"

static bool insideBeginEnd = false;

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
    glBegin(mode);
}

void GLFunctions::end()
{
    GL_SAFE_CALL(glEnd());
    insideBeginEnd = false;
}
