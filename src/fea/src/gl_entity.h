#ifndef GL_ENTITY_H
#define GL_ENTITY_H

#include "gl_object.h"
#include "gl_texture.h"
#include "model.h"
#include "session_entity_id.h"

class GLEntity : public GLObject
{

    protected:

        //! entity ID.
        SessionEntityID entityID;
        //! GL texture.
        GLTexture texture;

    private:

        //! Internal initialization function.
        void _init(const GLEntity *pGlEntity = nullptr);

    public:

        //! Constructor.
        GLEntity(GLWidget *glWidget, const SessionEntityID &entityID);

        //! Copy constructor.
        GLEntity(const GLEntity &glEntity);

        //! Destructor.
        ~GLEntity();

        //! Assignment operator.
        GLEntity & operator = (const GLEntity &glEntity);

        //! Return const reference to entity ID.
        const SessionEntityID &getEntityID() const;

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();
};

#endif // GL_ENTITY_H
