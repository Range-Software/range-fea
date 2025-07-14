#ifndef GL_INTERPOLATED_ENTITY_H
#define GL_INTERPOLATED_ENTITY_H

#include <rml_interpolated_entity.h>

#include "gl_entity.h"

class GLInterpolatedEntity : public GLEntity, public RInterpolatedEntity
{

    protected:

        // Environment settings
        GLboolean lightingEnabled;
        GLboolean normalize;
        GLfloat pointSize;
        GLfloat lineWidth;

    private:

        //! Internal initialization function.
        void _init(const GLInterpolatedEntity *pGlInterpolatedEntity = nullptr);

    public:

        //! Constructor.
        GLInterpolatedEntity(GLWidget *glWidget, const RInterpolatedEntity &iEntity, const SessionEntityID &entityID);

        //! Copy constructor.
        GLInterpolatedEntity(const GLInterpolatedEntity &glInterpolatedEntity);

        //! Destructor.
        ~GLInterpolatedEntity();

        //! Assignment operator.
        GLInterpolatedEntity & operator = (const GLInterpolatedEntity &glInterpolatedEntity);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

};

#endif // GL_INTERPOLATED_ENTITY_H
