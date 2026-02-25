#include <cstddef>

#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include "gl_vertex_buffer.h"
#include "gl_functions.h"

// GLVertexBuffer implementation

void GLVertexBuffer::_init(const GLVertexBuffer *pBuffer)
{
    this->vboId = 0;
    this->vertexCount = 0;
    this->valid = false;
    this->recording = false;
    this->currentBatchStart = 0;
    this->currentBatchPrimitive = GL_TRIANGLES;
    this->currentNormal[0] = 0.0f;
    this->currentNormal[1] = 0.0f;
    this->currentNormal[2] = 1.0f;
    this->currentColor[0] = 255;
    this->currentColor[1] = 255;
    this->currentColor[2] = 255;
    this->currentColor[3] = 255;
    this->currentTexCoord = 0.0f;

    if (pBuffer)
    {
        // Don't copy GPU resources, just mark as invalid
    }
}

GLVertexBuffer::GLVertexBuffer()
{
    this->_init();
}

GLVertexBuffer::GLVertexBuffer(const GLVertexBuffer &buffer)
{
    this->_init(&buffer);
}

GLVertexBuffer::~GLVertexBuffer()
{
    this->release();
}

GLVertexBuffer &GLVertexBuffer::operator=(const GLVertexBuffer &buffer)
{
    this->release();
    this->_init(&buffer);
    return *this;
}

bool GLVertexBuffer::isValid() const
{
    return this->valid && this->vboId != 0;
}

void GLVertexBuffer::invalidate()
{
    this->valid = false;
}

void GLVertexBuffer::reset()
{
    this->vertices.clear();
    this->batches.clear();
    this->valid = false;
    this->recording = false;
    this->currentBatchStart = 0;
}

void GLVertexBuffer::beginRecording(GLenum primitive)
{
    // Record where this batch starts in the accumulated vertex array.
    // Do NOT clear vertices — we accumulate across multiple begin/end pairs.
    this->currentBatchStart = GLint(this->vertices.size());
    this->currentBatchPrimitive = primitive;
    this->recording = true;
}

void GLVertexBuffer::endRecording()
{
    if (!this->recording)
    {
        return;
    }
    this->recording = false;

    GLsizei count = GLsizei(this->vertices.size()) - this->currentBatchStart;
    if (count > 0)
    {
        Batch batch;
        batch.primitiveType = this->currentBatchPrimitive;
        batch.start = this->currentBatchStart;
        batch.count = count;
        this->batches.push_back(batch);
    }
}

void GLVertexBuffer::uploadToGPU()
{
    if (this->vertices.empty())
    {
        // Nothing recorded — do not touch valid flag so caller can detect empty.
        return;
    }

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions *f = ctx->functions();

    // Generate VBO if needed (reuse existing handle if present).
    if (this->vboId == 0)
    {
        GL_SAFE_CALL(f->glGenBuffers(1, &this->vboId));
    }

    // Upload all accumulated vertex data in a single call.
    GL_SAFE_CALL(f->glBindBuffer(GL_ARRAY_BUFFER, this->vboId));
    GL_SAFE_CALL(f->glBufferData(GL_ARRAY_BUFFER,
                                 GLsizeiptr(this->vertices.size() * sizeof(GLVertexData)),
                                 this->vertices.data(),
                                 GL_STATIC_DRAW));
    GL_SAFE_CALL(f->glBindBuffer(GL_ARRAY_BUFFER, 0));

    this->vertexCount = GLsizei(this->vertices.size());
    this->valid = true;

    // Free CPU-side vertex data — batches[] is retained for render().
    this->vertices.clear();
    this->vertices.shrink_to_fit();
}

void GLVertexBuffer::setNormal(GLfloat x, GLfloat y, GLfloat z)
{
    this->currentNormal[0] = x;
    this->currentNormal[1] = y;
    this->currentNormal[2] = z;
}

void GLVertexBuffer::setColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    this->currentColor[0] = r;
    this->currentColor[1] = g;
    this->currentColor[2] = b;
    this->currentColor[3] = a;
}

void GLVertexBuffer::setTexCoord(GLfloat t)
{
    this->currentTexCoord = t;
}

void GLVertexBuffer::addVertex(GLfloat x, GLfloat y, GLfloat z)
{
    if (!this->recording)
    {
        return;
    }

    GLVertexData vertex;
    vertex.position[0] = x;
    vertex.position[1] = y;
    vertex.position[2] = z;
    vertex.normal[0] = this->currentNormal[0];
    vertex.normal[1] = this->currentNormal[1];
    vertex.normal[2] = this->currentNormal[2];
    vertex.texCoord = this->currentTexCoord;
    vertex.color[0] = this->currentColor[0];
    vertex.color[1] = this->currentColor[1];
    vertex.color[2] = this->currentColor[2];
    vertex.color[3] = this->currentColor[3];

    this->vertices.push_back(vertex);
}

void GLVertexBuffer::render() const
{
    if (!this->valid || this->vboId == 0 || this->batches.empty())
    {
        return;
    }

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions *f = ctx->functions();

    // Bind VBO — with a VBO bound, the pointer offsets below are byte offsets
    // from the start of the buffer (fixed-function VBO path, OpenGL 1.5+).
    f->glBindBuffer(GL_ARRAY_BUFFER, this->vboId);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(GLVertexData),
                    reinterpret_cast<const void*>(offsetof(GLVertexData, position)));
    glNormalPointer(GL_FLOAT, sizeof(GLVertexData),
                    reinterpret_cast<const void*>(offsetof(GLVertexData, normal)));
    glTexCoordPointer(1, GL_FLOAT, sizeof(GLVertexData),
                      reinterpret_cast<const void*>(offsetof(GLVertexData, texCoord)));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(GLVertexData),
                   reinterpret_cast<const void*>(offsetof(GLVertexData, color)));

    for (const Batch &batch : this->batches)
    {
        glDrawArrays(batch.primitiveType, batch.start, batch.count);
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLVertexBuffer::release()
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx)
    {
        QOpenGLFunctions *f = ctx->functions();
        if (this->vboId != 0)
        {
            GL_SAFE_CALL(f->glDeleteBuffers(1, &this->vboId));
            this->vboId = 0;
        }
    }
    this->valid = false;
    this->vertexCount = 0;
    this->vertices.clear();
    this->batches.clear();
}

GLsizei GLVertexBuffer::getVertexCount() const
{
    return this->vertexCount;
}

bool GLVertexBuffer::isRecording() const
{
    return this->recording;
}

// GLVertexBufferList implementation

void GLVertexBufferList::_init(const GLVertexBufferList *pList)
{
    if (pList)
    {
        this->buffers.resize(pList->buffers.size());
    }
}

GLVertexBufferList::GLVertexBufferList(size_t nBuffers)
{
    this->_init();
    this->buffers.resize(nBuffers);
}

GLVertexBufferList::GLVertexBufferList(const GLVertexBufferList &list)
{
    this->_init(&list);
}

GLVertexBufferList::~GLVertexBufferList()
{
    this->releaseAll();
}

GLVertexBufferList &GLVertexBufferList::operator=(const GLVertexBufferList &list)
{
    this->releaseAll();
    this->_init(&list);
    return *this;
}

size_t GLVertexBufferList::getBufferCount() const
{
    return this->buffers.size();
}

void GLVertexBufferList::setBufferCount(size_t count)
{
    if (count != this->buffers.size())
    {
        this->releaseAll();
        this->buffers.resize(count);
    }
}

GLVertexBuffer &GLVertexBufferList::getBuffer(size_t index)
{
    return this->buffers[index];
}

const GLVertexBuffer &GLVertexBufferList::getBuffer(size_t index) const
{
    return this->buffers[index];
}

bool GLVertexBufferList::isValid(size_t index) const
{
    if (index >= this->buffers.size())
    {
        return false;
    }
    return this->buffers[index].isValid();
}

void GLVertexBufferList::invalidate(size_t index)
{
    if (index < this->buffers.size())
    {
        this->buffers[index].invalidate();
    }
}

void GLVertexBufferList::invalidateAll()
{
    for (auto &buffer : this->buffers)
    {
        buffer.invalidate();
    }
}

void GLVertexBufferList::releaseAll()
{
    for (auto &buffer : this->buffers)
    {
        buffer.release();
    }
}
