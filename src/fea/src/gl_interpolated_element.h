#ifndef GL_INTERPOLATED_ELEMENT_H
#define GL_INTERPOLATED_ELEMENT_H

#include <rml_entity_group_data.h>
#include <rml_interpolated_element.h>

#include "gl_element_base.h"

class GLInterpolatedElement : public GLElementBase, public RInterpolatedElement
{

    private:

        //! Internal initialization function.
        void _init(const GLInterpolatedElement *pGlElement = nullptr);

    public:

        //! Constructor.
        GLInterpolatedElement(GLWidget *glWidget,
                              const Model *pModel,
                              const RInterpolatedElement &iElement,
                              uint elementID,
                              const REntityGroupData &elementGroupData,
                              GLElementDrawMode drawMode = GL_ELEMENT_DRAW_NORMAL);

        //! Copy constructor.
        GLInterpolatedElement(const GLInterpolatedElement &glElement);

        //! Destructor.
        ~GLInterpolatedElement();

        //! Assignment operator.
        GLInterpolatedElement & operator = (const GLInterpolatedElement &glElement);

    protected:

        //! Draw scene.
        void draw();

        //! Draw normal scene.
        void drawNormal();

};

#endif // GL_INTERPOLATED_ELEMENT_H
