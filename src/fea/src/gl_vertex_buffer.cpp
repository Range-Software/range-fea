#include <QOpenGLFunctions>

#include "gl_vertex_buffer.h"
#include "gl_functions.h"

// GLVertexBuffer implementation

void GLVertexBuffer::_init(const GLVertexBuffer *pBuffer)
{
    this->vboId = 0;
    this->vertexCount = 0;
    this->primitiveType = GL_TRIANGLES;
    this->valid = false;
    this->recording = false;
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
        this->primitiveType = pBuffer->primitiveType;
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

void GLVertexBuffer::beginRecording(GLenum primitive)
{
    // Clear any existing data
    this->vertices.clear();
    this->primitiveType = primitive;
    this->recording = true;
    this->valid = false;
}

void GLVertexBuffer::endRecording()
{
    if (!this->recording)
    {
        return;
    }

    this->recording = false;

    if (this->vertices.empty())
    {
        this->valid = false;
        return;
    }

    // Generate VBO if needed
    if (this->vboId == 0)
    {
        GL_SAFE_CALL(glGenBuffers(1, &this->vboId));
    }

    // Upload data to GPU
    GL_SAFE_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->vboId));
    GL_SAFE_CALL(glBufferData(GL_ARRAY_BUFFER,
                              GLsizeiptr(this->vertices.size() * sizeof(GLVertexData)),
                              this->vertices.data(),
                              GL_STATIC_DRAW));
    GL_SAFE_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    this->vertexCount = GLsizei(this->vertices.size());
    this->valid = true;

    // Clear CPU-side data to save memory
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
    if (!this->valid || this->vboId == 0 || this->vertexCount == 0)
    {
        return;
    }

    GL_SAFE_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->vboId));

    // Set up vertex attribute pointers
    GL_SAFE_CALL(glEnableClientState(GL_VERTEX_ARRAY));
    GL_SAFE_CALL(glEnableClientState(GL_NORMAL_ARRAY));
    GL_SAFE_CALL(glEnableClientState(GL_COLOR_ARRAY));
    GL_SAFE_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

    // Position: offset 0
    GL_SAFE_CALL(glVertexPointer(3, GL_FLOAT, sizeof(GLVertexData),
                                 reinterpret_cast<void*>(offsetof(GLVertexData, position))));

    // Normal: offset after position
    GL_SAFE_CALL(glNormalPointer(GL_FLOAT, sizeof(GLVertexData),
                                 reinterpret_cast<void*>(offsetof(GLVertexData, normal))));

    // Texture coordinate: offset after normal
    GL_SAFE_CALL(glTexCoordPointer(1, GL_FLOAT, sizeof(GLVertexData),
                                   reinterpret_cast<void*>(offsetof(GLVertexData, texCoord))));

    // Color: offset after texCoord
    GL_SAFE_CALL(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(GLVertexData),
                                reinterpret_cast<void*>(offsetof(GLVertexData, color))));

    // Draw
    GL_SAFE_CALL(glDrawArrays(this->primitiveType, 0, this->vertexCount));

    // Cleanup
    GL_SAFE_CALL(glDisableClientState(GL_VERTEX_ARRAY));
    GL_SAFE_CALL(glDisableClientState(GL_NORMAL_ARRAY));
    GL_SAFE_CALL(glDisableClientState(GL_COLOR_ARRAY));
    GL_SAFE_CALL(glDisableClientState(GL_TEXTURE_COORD_ARRAY));

    GL_SAFE_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void GLVertexBuffer::release()
{
    if (this->vboId != 0)
    {
        GL_SAFE_CALL(glDeleteBuffers(1, &this->vboId));
        this->vboId = 0;
    }
    this->valid = false;
    this->vertexCount = 0;
    this->vertices.clear();
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
