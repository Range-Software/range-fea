#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QOpenGLFunctions>

class GLVertexBuffer;

#define GL_SAFE_CALL(__safe_call) { __safe_call; GLFunctions::printError(#__safe_call,__FILE__,__LINE__); }

namespace GLFunctions
{

    //! Wrapper for glBegin().
    void begin(GLenum  mode);

    //! Wrapper for glEnd().
    void end();

    //! Set normal - records to VBO if in recording mode.
    void normal3f(GLfloat x, GLfloat y, GLfloat z);

    //! Set normal from double - records to VBO if in recording mode.
    void normal3d(GLdouble x, GLdouble y, GLdouble z);

    //! Add vertex - records to VBO if in recording mode.
    void vertex3f(GLfloat x, GLfloat y, GLfloat z);

    //! Add vertex from double - records to VBO if in recording mode.
    void vertex3d(GLdouble x, GLdouble y, GLdouble z);

    //! Set color - records to VBO if in recording mode.
    void color4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a);

    //! Set texture coordinate - records to VBO if in recording mode.
    void texCoord1f(GLfloat t);

    //! Enable VBO recording mode.
    //! All subsequent begin/end blocks will record to the provided buffer.
    void beginVBORecording(GLVertexBuffer *buffer);

    //! Disable VBO recording mode and finalize the buffer.
    void endVBORecording();

    //! Check if currently recording to VBO.
    bool isRecordingVBO();

    //! Get the current VBO being recorded to.
    GLVertexBuffer *getCurrentVBO();

    void printError(const char *command, const char *file, unsigned int line);

}

#endif // GL_FUNCTIONS_H
