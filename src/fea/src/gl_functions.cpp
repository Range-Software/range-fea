#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

#include "gl_functions.h"
#include "gl_vertex_buffer.h"
#include "render_backend.h"
#include "rhi_renderer.h"

#include <QString>

#include <rbl_logger.h>

static bool insideBeginEnd = false;
static GLVertexBuffer *currentVBO = nullptr;
static bool vboRecordingMode = false;

// Immediate mode scratch state, used by the QRhi backend where there is no
// fixed-function pipeline to fall back to.  Vertices recorded between begin()
// and end() are handed to the renderer as one draw item.
static std::vector<GLVertexData> immediateVertices;
static GLenum   immediatePrimitive = GL_TRIANGLES;
static GLfloat  immediateNormal[3] = { 0.0f, 0.0f, 1.0f };
static GLubyte  immediateColor[4]  = { 255, 255, 255, 255 };
static GLfloat  immediateTexCoord  = 0.0f;

void GLFunctions::printError(const char *command, const char *file, unsigned int line)
{
    if (insideBeginEnd || !RenderBackend::isOpenGL())
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
    else if (RenderBackend::isRhi())
    {
        immediateVertices.clear();
        immediatePrimitive = mode;
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
    else if (RenderBackend::isRhi())
    {
        RhiRenderer *renderer = RhiRenderer::current();
        if (renderer && !immediateVertices.empty())
        {
            renderer->drawImmediate(immediateVertices.data(), immediateVertices.size(), immediatePrimitive);
        }
        immediateVertices.clear();
    }
    else
    {
        glEnd();
    }
    insideBeginEnd = false;
}

void GLFunctions::normal3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (vboRecordingMode && currentVBO)
    {
        currentVBO->setNormal(x, y, z);
    }
    else if (RenderBackend::isRhi())
    {
        immediateNormal[0] = x;
        immediateNormal[1] = y;
        immediateNormal[2] = z;
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
    else if (RenderBackend::isRhi())
    {
        GLVertexData vertex;
        vertex.position[0] = x;
        vertex.position[1] = y;
        vertex.position[2] = z;
        vertex.normal[0] = immediateNormal[0];
        vertex.normal[1] = immediateNormal[1];
        vertex.normal[2] = immediateNormal[2];
        vertex.texCoord = immediateTexCoord;
        vertex.color[0] = immediateColor[0];
        vertex.color[1] = immediateColor[1];
        vertex.color[2] = immediateColor[2];
        vertex.color[3] = immediateColor[3];
        immediateVertices.push_back(vertex);
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
    else if (RenderBackend::isRhi())
    {
        immediateColor[0] = r;
        immediateColor[1] = g;
        immediateColor[2] = b;
        immediateColor[3] = a;
    }
    else
    {
        glColor4ub(r, g, b, a);
        // Generic attribute 2 (aColor) is not fed by glColor4ub, so set it explicitly
        // so immediate-mode geometry drawn while a shader is active gets the right colour.
        QOpenGLContext::currentContext()->functions()->glVertexAttrib4f(2, r/255.0f, g/255.0f, b/255.0f, a/255.0f);
    }
}

void GLFunctions::texCoord1f(GLfloat t)
{
    if (vboRecordingMode && currentVBO)
    {
        currentVBO->setTexCoord(t);
    }
    else if (RenderBackend::isRhi())
    {
        immediateTexCoord = t;
    }
    else
    {
        glTexCoord1f(t);
        // Generic attribute 3 (aTexCoord) mirrors the above for the same reason.
        QOpenGLContext::currentContext()->functions()->glVertexAttrib1f(3, t);
    }
}

void GLFunctions::beginVBORecording(GLVertexBuffer *buffer)
{
    if (buffer)
    {
        // Clear any leftover CPU data from a previous (possibly interrupted) session.
        buffer->reset();
    }
    currentVBO = buffer;
    vboRecordingMode = (buffer != nullptr);
}

void GLFunctions::endVBORecording()
{
    if (currentVBO)
    {
        // Upload all accumulated batches to the GPU in one call.
        currentVBO->uploadToGPU();
    }
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
