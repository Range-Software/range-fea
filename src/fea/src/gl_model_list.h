#ifndef GL_MODEL_LIST_H
#define GL_MODEL_LIST_H

#include <QList>
#include <QVector>

#include "gl_entity_list.h"

class GLModelList
{

    protected:

        QVector<GLEntityList> glPointList;
        QVector<GLEntityList> glLineList;
        QVector<GLEntityList> glSurfaceList;
        QVector<GLEntityList> glVolumeList;
        QVector<GLEntityList> glVectorFieldList;
        QVector<GLEntityList> glScalarFieldList;
        QVector<GLEntityList> glStreamLineList;
        QVector<GLEntityList> glCutList;
        QVector<GLEntityList> glIsoList;

    private:

        //! Internal initialization function.
        void _init (const GLModelList *pGlModelList = nullptr);

    public:

        //! Constructor.
        GLModelList();

        //! Copy constructor.
        GLModelList(const GLModelList &glModelList);

        //! Destructor.
        ~GLModelList();

        //! Assignment operator.
        GLModelList & operator = (const GLModelList &glModelList);

        //! Clear list content.
        void clear();

        //! Return number of OpenGL point lists.
        uint getNGlPointLists() const;

        //! Return number of OpenGL point lists.
        void setNGlPointLists(uint nGlEntityLists);

        //! Return const reference of OpenGL point list with given entity ID.
        const GLEntityList & getGlPointList(uint entityID) const;

        //! Return reference to OpenGL point list with given entity ID.
        GLEntityList & getGlPointList(uint entityID);

        //! Set new OpenGL point list with given entity ID.
        void setGlPointList(uint entityID, const GLEntityList &glEntityList);

        //! Return number of OpenGL line lists.
        uint getNGlLineLists() const;

        //! Return number of OpenGL line lists.
        void setNGlLineLists(uint nGlEntityLists);

        //! Return const reference of OpenGL line list with given entity ID.
        const GLEntityList & getGlLineList(uint entityID) const;

        //! Return reference to OpenGL line list with given entity ID.
        GLEntityList & getGlLineList(uint entityID);

        //! Set new OpenGL line list with given entity ID.
        void setGlLineList(uint entityID, const GLEntityList &glEntityList);

        //! Return number of OpenGL surface lists.
        uint getNGlSurfaceLists() const;

        //! Return number of OpenGL surface lists.
        void setNGlSurfaceLists(uint nGlEntityLists);

        //! Return const reference of OpenGL surface list with given entity ID.
        const GLEntityList & getGlSurfaceList(uint entityID) const;

        //! Return reference to OpenGL surface list with given entity ID.
        GLEntityList & getGlSurfaceList(uint entityID);

        //! Set new OpenGL surface list with given entity ID.
        void setGlSurfaceList(uint entityID, const GLEntityList &glEntityList);

        //! Return number of OpenGL volume lists.
        uint getNGlVolumeLists() const;

        //! Return number of OpenGL volume lists.
        void setNGlVolumeLists(uint nGlEntityLists);

        //! Return const reference of OpenGL volume list with given entity ID.
        const GLEntityList & getGlVolumeList(uint entityID) const;

        //! Return reference to OpenGL volume list with given entity ID.
        GLEntityList & getGlVolumeList(uint entityID);

        //! Set new OpenGL volume list with given entity ID.
        void setGlVolumeList(uint entityID, const GLEntityList &glEntityList);

        //! Return number of OpenGL vector field lists.
        uint getNGlVectorFieldLists() const;

        //! Return number of OpenGL vector field lists.
        void setNGlVectorFieldLists(uint nGlEntityLists);

        //! Return const reference of OpenGL vector field list with given entity ID.
        const GLEntityList & getGlVectorFieldList(uint entityID) const;

        //! Return reference to OpenGL vector field list with given entity ID.
        GLEntityList & getGlVectorFieldList(uint entityID);

        //! Set new OpenGL vector field list with given entity ID.
        void setGlVectorFieldList(uint entityID, const GLEntityList &glEntityList);

        //! Return number of OpenGL scalar field lists.
        uint getNGlScalarFieldLists() const;

        //! Return number of OpenGL scalar field lists.
        void setNGlScalarFieldLists(uint nGlEntityLists);

        //! Return const reference of OpenGL scalar field list with given entity ID.
        const GLEntityList & getGlScalarFieldList(uint entityID) const;

        //! Return reference to OpenGL scalar field list with given entity ID.
        GLEntityList & getGlScalarFieldList(uint entityID);

        //! Set new OpenGL scalar field list with given entity ID.
        void setGlScalarFieldList(uint entityID, const GLEntityList &glEntityList);

        //! Return number of OpenGL stream line lists.
        uint getNGlStreamLineLists() const;

        //! Return number of OpenGL stream line lists.
        void setNGlStreamLineLists(uint nGlEntityLists);

        //! Return const reference of OpenGL stream line list with given entity ID.
        const GLEntityList & getGlStreamLineList(uint entityID) const;

        //! Return reference to OpenGL stream line list with given entity ID.
        GLEntityList & getGlStreamLineList(uint entityID);

        //! Set new OpenGL stream line list with given entity ID.
        void setGlStreamLineList(uint entityID, const GLEntityList &glEntityList);

        //! Return number of OpenGL cut lists.
        uint getNGlCutLists() const;

        //! Return number of OpenGL cut lists.
        void setNGlCutLists(uint nGlEntityLists);

        //! Return const reference of OpenGL cut list with given entity ID.
        const GLEntityList & getGlCutList(uint entityID) const;

        //! Return reference to OpenGL cut list with given entity ID.
        GLEntityList & getGlCutList(uint entityID);

        //! Set new OpenGL cut list with given entity ID.
        void setGlCutList(uint entityID, const GLEntityList &glEntityList);

        //! Return number of OpenGL iso lists.
        uint getNGlIsoLists() const;

        //! Return number of OpenGL iso lists.
        void setNGlIsoLists(uint nGlEntityLists);

        //! Return const reference of OpenGL iso list with given entity ID.
        const GLEntityList & getGlIsoList(uint entityID) const;

        //! Return reference to OpenGL iso list with given entity ID.
        GLEntityList & getGlIsoList(uint entityID);

        //! Set new OpenGL iso list with given entity ID.
        void setGlIsoList(uint entityID, const GLEntityList &glEntityList);

};

#endif /* GL_MODEL_LIST_H */
