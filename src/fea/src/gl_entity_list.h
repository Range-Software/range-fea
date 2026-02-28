#ifndef GL_ENTITY_LIST_H
#define GL_ENTITY_LIST_H

#include "gl_vertex_buffer.h"

typedef enum _GLEntityListItemType
{
    GL_ENTITY_LIST_ITEM_NORMAL = 0,
    GL_ENTITY_LIST_ITEM_PICK_ELEMENT,
    GL_ENTITY_LIST_ITEM_PICK_NODE,
    GL_ENTITY_LIST_ITEM_N_LISTS
} GLEntityListItemType;

class GLEntityList
{

    public:

        //! VBO for each list type.
        GLVertexBuffer vbo[GL_ENTITY_LIST_ITEM_N_LISTS];

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

        // Compat stubs — used by gl_scalar_field, gl_vector_field, gl_interpolated_entity.
        // These redirect to the VBO path so those callers need no changes.
        bool getListValid(GLuint listPosition) const;
        void newList(GLuint listPosition, GLenum mode = GL_COMPILE);
        void endList(GLuint listPosition);
        void callList(GLuint listPosition) const;
};

#endif /* GL_ENTITY_LIST_H */
