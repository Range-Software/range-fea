#ifndef GL_ENTITY_LIST_H
#define GL_ENTITY_LIST_H

#include "gl_list.h"
#include "gl_vertex_buffer.h"

typedef enum _GLEntityListItemType
{
    GL_ENTITY_LIST_ITEM_NORMAL = 0,
    GL_ENTITY_LIST_ITEM_PICK_ELEMENT,
    GL_ENTITY_LIST_ITEM_PICK_NODE,
    GL_ENTITY_LIST_ITEM_N_LISTS
} GLEntityListItemType;

class GLEntityList : public GLList
{

    protected:

        //! VBO for each list type (replaces display lists for better performance).
        GLVertexBuffer vbo[GL_ENTITY_LIST_ITEM_N_LISTS];
        //! Whether to use VBO instead of display list.
        bool useVBO;

    private:

        //! Internal initialization function.
        void _init (const GLEntityList *pGlEntityList = nullptr);

    public:

        //! Constructor.
        GLEntityList();

        //! Copy constructor.
        GLEntityList(const GLEntityList &glEntityList);

        //! Destructor.
        ~GLEntityList();

        //! Assignment operator.
        GLList & operator = (const GLEntityList &glEntityList);

        //! Get whether VBO mode is enabled.
        bool getUseVBO() const;

        //! Set whether to use VBO mode.
        void setUseVBO(bool useVBO);

        //! Get VBO for the specified list type.
        GLVertexBuffer &getVBO(GLuint listPosition);

        //! Get const VBO for the specified list type.
        const GLVertexBuffer &getVBO(GLuint listPosition) const;

        //! Check if VBO is valid for the specified list type.
        bool getVBOValid(GLuint listPosition) const;

        //! Invalidate VBO for the specified list type.
        void setVBOInvalid(GLuint listPosition);

        //! Invalidate both display list and VBO for the specified position.
        //! Overrides GLList::setListInvalid to also invalidate VBO.
        void setListInvalid(GLuint listPosition);
};

#endif /* GL_ENTITY_LIST_H */
