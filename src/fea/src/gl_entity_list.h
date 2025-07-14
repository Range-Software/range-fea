#ifndef GL_ENTITY_LIST_H
#define GL_ENTITY_LIST_H

#include "gl_list.h"

typedef enum _GLEntityListItemType
{
    GL_ENTITY_LIST_ITEM_NORMAL = 0,
    GL_ENTITY_LIST_ITEM_PICK_ELEMENT,
    GL_ENTITY_LIST_ITEM_PICK_NODE,
    GL_ENTITY_LIST_ITEM_N_LISTS
} GLEntityListItemType;

class GLEntityList : public GLList
{

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
};

#endif /* GL_ENTITY_LIST_H */
