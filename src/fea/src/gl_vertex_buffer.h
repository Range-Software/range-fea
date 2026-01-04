#ifndef GL_VERTEX_BUFFER_H
#define GL_VERTEX_BUFFER_H

#include <vector>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QtOpenGL>

//! Vertex data structure for VBO rendering.
struct GLVertexData
{
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat texCoord;
    GLubyte color[4];
};

//! OpenGL Vertex Buffer Object manager.
//! Replaces display lists with modern VBO-based rendering for better performance.
class GLVertexBuffer
{

    public:

        //! Primitive types supported.
        enum PrimitiveType
        {
            Triangles = GL_TRIANGLES,
            Quads = GL_QUADS,
            Lines = GL_LINES,
            LineLoop = GL_LINE_LOOP,
            Points = GL_POINTS,
            Polygon = GL_POLYGON
        };

    protected:

        //! VBO handle for vertex data.
        GLuint vboId;
        //! VAO handle for vertex attribute state.
        GLuint vaoId;
        //! Number of vertices in the buffer.
        GLsizei vertexCount;
        //! Primitive type for rendering.
        GLenum primitiveType;
        //! Whether the VBO is valid and ready for rendering.
        bool valid;
        //! Accumulated vertex data (before upload to GPU).
        std::vector<GLVertexData> vertices;
        //! Whether we're currently recording vertices.
        bool recording;
        //! Current normal for vertices being added.
        GLfloat currentNormal[3];
        //! Current color for vertices being added.
        GLubyte currentColor[4];
        //! Current texture coordinate.
        GLfloat currentTexCoord;

    private:

        //! Internal initialization function.
        void _init(const GLVertexBuffer *pBuffer = nullptr);

    public:

        //! Constructor.
        GLVertexBuffer();

        //! Copy constructor.
        GLVertexBuffer(const GLVertexBuffer &buffer);

        //! Destructor.
        ~GLVertexBuffer();

        //! Assignment operator.
        GLVertexBuffer &operator=(const GLVertexBuffer &buffer);

        //! Check if VBO is valid and ready for rendering.
        bool isValid() const;

        //! Invalidate the VBO (will be rebuilt on next use).
        void invalidate();

        //! Begin recording vertices for a primitive type.
        void beginRecording(GLenum primitive);

        //! End recording and upload data to GPU.
        void endRecording();

        //! Set current normal (applies to subsequent vertices).
        void setNormal(GLfloat x, GLfloat y, GLfloat z);

        //! Set current color (applies to subsequent vertices).
        void setColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a);

        //! Set current texture coordinate (applies to subsequent vertices).
        void setTexCoord(GLfloat t);

        //! Add a vertex with current normal, color, and texcoord.
        void addVertex(GLfloat x, GLfloat y, GLfloat z);

        //! Render the VBO contents.
        void render() const;

        //! Delete GPU resources.
        void release();

        //! Get number of vertices.
        GLsizei getVertexCount() const;

        //! Check if recording is in progress.
        bool isRecording() const;

};

//! Manages multiple VBOs for different render passes (normal, pick element, pick node).
class GLVertexBufferList
{

    protected:

        //! Array of vertex buffers.
        std::vector<GLVertexBuffer> buffers;

    private:

        //! Internal initialization function.
        void _init(const GLVertexBufferList *pList = nullptr);

    public:

        //! Constructor.
        explicit GLVertexBufferList(size_t nBuffers = 0);

        //! Copy constructor.
        GLVertexBufferList(const GLVertexBufferList &list);

        //! Destructor.
        ~GLVertexBufferList();

        //! Assignment operator.
        GLVertexBufferList &operator=(const GLVertexBufferList &list);

        //! Get number of buffers.
        size_t getBufferCount() const;

        //! Set number of buffers.
        void setBufferCount(size_t count);

        //! Get buffer at index.
        GLVertexBuffer &getBuffer(size_t index);

        //! Get const buffer at index.
        const GLVertexBuffer &getBuffer(size_t index) const;

        //! Check if buffer at index is valid.
        bool isValid(size_t index) const;

        //! Invalidate buffer at index.
        void invalidate(size_t index);

        //! Invalidate all buffers.
        void invalidateAll();

        //! Release all GPU resources.
        void releaseAll();

};

#endif // GL_VERTEX_BUFFER_H
