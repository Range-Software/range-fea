#ifndef GL_ELEMENT_GROUP_H
#define GL_ELEMENT_GROUP_H

#include <rml_element_group.h>

#include "gl_entity.h"
#include "gl_texture.h"
#include "model.h"

class GLElementGroup : public GLEntity, public RElementGroup
{

    protected:

        //! Point volume.
        double pointVolume;
        //! Line cross area.
        double lineCrossArea;
        //! Surface thickness.
        double surfaceThickness;

        // Environment settings
        GLboolean lightingEnabled;
        GLboolean normalize;
        GLfloat pointSize;
        GLfloat lineWidth;

    private:

        //! Internal initialization function.
        void _init(const GLElementGroup *pGlElementGroup = nullptr);

    public:

        //! Constructor.
        GLElementGroup(GLWidget *glWidget, const RElementGroup &elementGroup, const SessionEntityID &entityID);

        //! Copy constructor.
        GLElementGroup(const GLElementGroup &glElementGroup);

        //! Destructor.
        ~GLElementGroup();

        //! Assignment operator.
        GLElementGroup & operator = (const GLElementGroup &glElementGroup);

        //! Set point volume.
        void setPointVolume(double pointVolume);

        //! Set line cross area.
        void setLineCrossArea(double lineCrossArea);

        //! Set surface thickness.
        void setSurfaceThickness(double surfaceThickness);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

};

#endif /* GL_ELEMENT_GROUP_H */
