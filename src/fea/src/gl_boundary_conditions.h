#ifndef GL_BOUNDARY_CONDITIONS_H
#define GL_BOUNDARY_CONDITIONS_H

#include <vector>

#include <QColor>
#include <QRectF>
#include <QString>

#include <rbl_r3vector.h>
#include <rml_boundary_condition.h>
#include <rml_element_group.h>

#include "gl_object.h"

class QPainter;

//! Draws symbols of the boundary conditions applied on an element group.
//! Symbols are drawn in immediate mode (they are never recorded into the
//! entity vertex buffer), because their size is kept constant on screen and
//! therefore depends on the current zoom.
class GLBoundaryConditions : public GLObject
{

    public:

        //! Boundary condition symbol category.
        enum Category
        {
            None = 0,
            //! Fixed support - each constrained component.
            FixedSupport,
            //! Support in local (normal) direction.
            NormalSupport,
            //! Sliding (roller) support.
            RollerSupport,
            //! Load given by vector components.
            VectorLoad,
            //! Load acting along the surface normal.
            NormalLoad,
            //! Body load acting in direction of gravity.
            WeightLoad,
            //! Flow entering the domain.
            Inflow,
            //! Velocity along the surface normal.
            NormalVelocity,
            //! No-slip wall.
            Wall,
            //! Frictionless wall.
            WallFrictionless,
            //! Scalar condition shown as text tag.
            Scalar
        };

    protected:

        //! Sample position at which symbols are drawn.
        struct Sample
        {
            //! Position.
            RR3Vector position;
            //! Unit direction - surface normal, line tangent or zero.
            RR3Vector direction;
            //! Whether the direction is a surface normal.
            bool isNormal;
            //! Whether the direction is a line tangent.
            bool isTangent;
        };

        //! Parent model.
        const Model *pModel;
        //! Element group whose boundary conditions are drawn.
        const RElementGroup *pElementGroup;
        //! Entity group type.
        REntityGroupType entityGroupType;

        // Environment settings
        GLboolean lightingEnabled;
        GLboolean normalizeEnabled;
        GLboolean cullFaceEnabled;
        bool twoSidedEnabled;
        GLfloat lineWidth;

    public:

        //! Constructor.
        explicit GLBoundaryConditions(GLWidget *glWidget,
                                      const Model *pModel,
                                      const RElementGroup &elementGroup,
                                      REntityGroupType entityGroupType);

        //! Return symbol category for given boundary condition type.
        static Category findCategory(RBoundaryConditionType type);

        //! Return color used to draw symbols of given category.
        static QColor getCategoryColor(Category category);

        //! Return short text tag for given boundary condition type.
        static QString getTag(RBoundaryConditionType type);

        //! Draw 2D symbol of given boundary condition type into rectangle.
        static void drawSymbol(QPainter &painter, const QRectF &rect, RBoundaryConditionType type);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

        //! Find sample positions (at most maxSamples) at which symbols are drawn.
        void findSamples(uint maxSamples, std::vector<Sample> &samples) const;

        //! Return direction derived from the geometry or entered by the user for normal/roller supports.
        RR3Vector findSupportDirection(const RBoundaryCondition &bc, const Sample &sample) const;

        //! Return gravity direction (unit vector) for weight load.
        RR3Vector findGravityDirection() const;

        //! Return vector given by the X/Y/Z components of the condition at current time.
        RR3Vector findComponentVector(const RBoundaryCondition &bc,
                                      RVariableType xType,
                                      RVariableType yType,
                                      RVariableType zType) const;

        //! Return text tag with value for scalar condition.
        QString findScalarText(const RBoundaryCondition &bc) const;

        //! Draw symbols of one boundary condition.
        void drawCondition(const RBoundaryCondition &bc, const std::vector<Sample> &samples, double size) const;

        //! Draw cone with apex at given position, opening along the axis (unit vector).
        void drawCone(const RR3Vector &apex, const RR3Vector &axis, double height, double radius) const;

        //! Draw circle in the plane perpendicular to the axis (unit vector).
        void drawCircle(const RR3Vector &center, const RR3Vector &axis, double radius) const;

        //! Draw line segment.
        void drawSegment(const RR3Vector &start, const RR3Vector &end) const;

        //! Draw arrow in given direction with its tip at given position.
        void drawArrow(const RR3Vector &position, const RR3Vector &direction, double length) const;

};

#endif // GL_BOUNDARY_CONDITIONS_H
