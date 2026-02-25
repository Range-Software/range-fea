#ifndef GL_ELEMENT_H
#define GL_ELEMENT_H

#include <rml_entity_group_data.h>

#include "gl_object.h"
#include "gl_element_base.h"

//! Pre-computed per-element data: pure CPU results that can be computed in
//! parallel (read-only model access) before the serial GL recording phase.
struct GLElementPrecomputedData
{
    bool valid;                        //!< false → skip this element in the draw phase
    RElementType type;                 //!< element geometry type
    std::vector<RR3Vector> nodes;      //!< final node positions (displacement applied)
    std::vector<double> textureCoords; //!< per-node scalar texture coords (empty if none)
    std::vector<bool> edgeNodes;       //!< per-node edge flags (tetrahedra only)
    QColor color;
    int drawMask;
    bool useGlCullFace;
    double pointVolume;
    double lineCrossArea;
    double surfaceThickness;

    GLElementPrecomputedData()
        : valid(false)
        , type(R_ELEMENT_POINT)
        , drawMask(0)
        , useGlCullFace(false)
        , pointVolume(0.0)
        , lineCrossArea(0.0)
        , surfaceThickness(0.0)
    {}
};

class GLElement : public GLElementBase, public RElement
{

    protected:

        //! Point volume.
        double pointVolume;
        //! Line cross area.
        double lineCrossArea;
        //! Surface thickness.
        double surfaceThickness;
        //! Optional pre-computed data (not owned; valid for one paint() call).
        const GLElementPrecomputedData *pPrecomputed;

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

        //! Set pre-computed data pointer (not owned; must outlive the paint() call).
        void setPrecomputedData(const GLElementPrecomputedData *pData);

        //! Pre-compute element data (node positions, scalar values, edge flags).
        //! Pure CPU work — safe to call from a worker thread.
        GLElementPrecomputedData precompute() const;

    protected:

        //! Draw scene (uses pPrecomputed if set, otherwise recomputes on the fly).
        void draw();

        //! Draw directly from pre-computed data (no model access; GL calls only).
        void drawFromPrecomputed(const GLElementPrecomputedData &data);

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
