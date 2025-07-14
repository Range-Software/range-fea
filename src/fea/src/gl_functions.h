#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QOpenGLFunctions>

#define GL_SAFE_CALL(__safe_call) { __safe_call; GLFunctions::printError(#__safe_call,__FILE__,__LINE__); }

namespace GLFunctions
{

    //! Wrapper for glBegin().
    void begin(GLenum  mode);

    //! Wrapper for glEnd().
    void end();

    void printError(const char *command, const char *file, unsigned int line);

}

#endif // GL_FUNCTIONS_H
