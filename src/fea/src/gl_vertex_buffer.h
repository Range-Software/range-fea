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
//! Uses fixed-function VBO rendering (glVertexPointer / glDrawArrays) compatible
//! with the legacy pipeline.  A session of GL recording produces multiple
//! glBegin/glEnd batches; all batches are accumulated CPU-side and uploaded to
//! the GPU in a single glBufferData call at the end of the session.
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

        //! One glBegin/glEnd primitive batch within the buffer.
        struct Batch
        {
            GLenum  primitiveType;
            GLint   start;   //!< First vertex index in the uploaded VBO.
            GLsizei count;   //!< Number of vertices in this batch.
        };

    protected:

        //! VBO handle for vertex data.
        GLuint vboId;
        //! Total uploaded vertex count (valid after uploadToGPU).
        GLsizei vertexCount;
        //! Whether the VBO is valid and ready for rendering.
        bool valid;
        //! Accumulated vertex data (CPU-side; cleared after uploadToGPU).
        std::vector<GLVertexData> vertices;
        //! Recorded primitive batches (retained after upload for render()).
        std::vector<Batch> batches;
        //! Whether we're currently recording vertices for a batch.
        bool recording;
        //! Start index in vertices[] of the current in-progress batch.
        GLint currentBatchStart;
        //! Primitive type of the current in-progress batch.
        GLenum currentBatchPrimitive;
        //! Current normal for vertices being added.
        GLfloat currentNormal[3];
        //! Current color for vertices being added.
        GLubyte currentColor[4];
        //! Current texture coordinate.
        GLfloat currentTexCoord;
        //! Whether any geometry recorded into this VBO used the 1D texture (colour map).
        //! Set during recording; used by callList() to drive uUseTexture uniform.
        bool usesTexture;

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

        //! Mark as invalid so it will be rebuilt next frame.
        void invalidate();

        //! Reset CPU-side buffers ready for a fresh recording session.
        //! Does not release the existing GPU buffer (it will be reused).
        void reset();

        //! Begin recording vertices for a new primitive batch.
        void beginRecording(GLenum primitive);

        //! End recording the current primitive batch (CPU-side only).
        void endRecording();

        //! Upload all accumulated CPU-side batches to the GPU VBO.
        //! Call once after all primitives have been recorded.
        void uploadToGPU();

        //! Set current normal (applies to subsequent vertices).
        void setNormal(GLfloat x, GLfloat y, GLfloat z);

        //! Set current color (applies to subsequent vertices).
        void setColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a);

        //! Set current texture coordinate (applies to subsequent vertices).
        void setTexCoord(GLfloat t);

        //! Add a vertex with current normal, color, and texcoord.
        void addVertex(GLfloat x, GLfloat y, GLfloat z);

        //! Render the VBO contents using fixed-function client-state arrays.
        void render() const;

        //! Delete GPU resources.
        void release();

        //! Get total number of uploaded vertices.
        GLsizei getVertexCount() const;

        //! Check if recording is in progress.
        bool isRecording() const;

        //! Mark this VBO as containing texture-mapped geometry (drives uUseTexture uniform).
        void setUsesTexture(bool t);

        //! Return true if the VBO contains texture-mapped geometry.
        bool getUsesTexture() const;

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
