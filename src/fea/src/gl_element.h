#ifndef GL_ELEMENT_H
#define GL_ELEMENT_H

#include <rml_entity_group_data.h>

#include "gl_object.h"
#include "gl_element_base.h"

class GLElement : public GLElementBase, public RElement
{

    protected:

        //! Point volume.
        double pointVolume;
        //! Line cross area.
        double lineCrossArea;
        //! Surface thickness.
        double surfaceThickness;

    private:

        //! Internal initialization function.
        void _init(const GLElement *pGlElement = nullptr);

    public:

        //! Constructor.
        GLElement(GLWidget *glWidget, const Model *pModel, uint elementID, const REntityGroupData &elementGroupData, const QColor &color, GLElementDrawMode drawMode = GL_ELEMENT_DRAW_NORMAL);

        //! Copy constructor.
        GLElement(const GLElement &glElement);

        //! Destructor.
        ~GLElement();

        //! Assignment operator.
        GLElement & operator = (const GLElement &glElement);

        //! Set point volume.
        void setPointVolume(double pointVolume);

        //! Set line cross area.
        void setLineCrossArea(double lineCrossArea);

        //! Set surface thickness.
        void setSurfaceThickness(double surfaceThickness);

    protected:

        //! Draw scene.
        void draw();

        //! Draw point.
        void drawPoint();

        //! Draw line.
        void drawLine();

        //! Draw triangle.
        void drawTriangle();

        //! Draw quadrilateral.
        void drawQuadrilateral();

        //! Draw tetrahedra.
        void drawTetrahedra();

};

#endif /* GL_ELEMENT_H */
