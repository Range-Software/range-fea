#ifndef GL_ENTITY_LIST_H
#define GL_ENTITY_LIST_H

#include "gl_vertex_buffer.h"

typedef enum _GLEntityListItemType
{
    //! Geometry of the entity. Picked elements and picked nodes are drawn as
    //! an overlay straight from the pick list rather than from a buffer of
    //! their own, so this is the only one an entity keeps.
    GL_ENTITY_LIST_ITEM_NORMAL = 0,
    GL_ENTITY_LIST_ITEM_N_LISTS
} GLEntityListItemType;

class GLEntityList
{

    public:

        //! VBO for each list type.
        GLVertexBuffer vbo[GL_ENTITY_LIST_ITEM_N_LISTS];

        //! Display properties the VBO of each list type was recorded with.
        //! A recorded VBO bakes in what it was told to draw, so it has to be
        //! recorded again when those properties change. Comparing them here
        //! catches a change whose invalidation was missed rather than leaving
        //! the entity drawn the way it was when it was first displayed.
        size_t buildSignature[GL_ENTITY_LIST_ITEM_N_LISTS];

        //! Constructor.
        GLEntityList();

        //! Copy constructor.
        GLEntityList(const GLEntityList &glEntityList);

        //! Destructor.
        ~GLEntityList();

        //! Assignment operator.
        GLEntityList & operator = (const GLEntityList &glEntityList);

        //! Get VBO for the specified list type.
        GLVertexBuffer &getVBO(GLuint listPosition);

        //! Get const VBO for the specified list type.
        const GLVertexBuffer &getVBO(GLuint listPosition) const;

        //! Check if VBO is valid for the specified list type.
        bool getVBOValid(GLuint listPosition) const;

        //! Invalidate VBO for the specified list type.
        void setVBOInvalid(GLuint listPosition);

        //! Invalidate VBO for the specified list type (alias for setVBOInvalid).
        void setListInvalid(GLuint listPosition);

        //! Return the display properties signature the given list was recorded with.
        size_t getBuildSignature(GLuint listPosition) const;

        //! Store the display properties signature the given list was recorded with.
        void setBuildSignature(GLuint listPosition, size_t signature);

        // Compat stubs — used by gl_scalar_field, gl_vector_field, gl_interpolated_entity.
        // These redirect to the VBO path so those callers need no changes.
        bool getListValid(GLuint listPosition) const;
        void newList(GLuint listPosition, GLenum mode = GL_COMPILE);
        void endList(GLuint listPosition);
        void callList(GLuint listPosition) const;
};

#endif /* GL_ENTITY_LIST_H */
