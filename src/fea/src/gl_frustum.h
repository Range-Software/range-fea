#ifndef GL_FRUSTUM_H
#define GL_FRUSTUM_H

#include <rbl_limit_box.h>

//! OpenGL view frustum for culling.
//! Extracts frustum planes from modelview-projection matrix and tests
//! bounding boxes for visibility.
class GLFrustum
{

    public:

        //! Frustum plane indices.
        enum Plane
        {
            Right = 0,
            Left = 1,
            Bottom = 2,
            Top = 3,
            Far = 4,
            Near = 5
        };

    protected:

        //! Frustum planes (6 planes, each with 4 coefficients: A, B, C, D).
        //! Plane equation: Ax + By + Cz + D = 0
        double planes[6][4];

    public:

        //! Constructor.
        GLFrustum();

        //! Extract frustum planes from current OpenGL modelview and projection matrices.
        void extractFromGL();

        //! Extract frustum planes from combined modelview-projection matrix.
        void extractFromMatrix(const double mvp[16]);

        //! Test if a point is inside the frustum.
        bool containsPoint(double x, double y, double z) const;

        //! Test if a bounding box intersects the frustum.
        //! Returns true if the box is at least partially visible.
        bool intersectsBox(const RLimitBox &box) const;

        //! Test if a bounding box intersects the frustum.
        bool intersectsBox(double xMin, double xMax,
                          double yMin, double yMax,
                          double zMin, double zMax) const;

    private:

        //! Normalize a frustum plane.
        void normalizePlane(int planeIndex);

};

#endif // GL_FRUSTUM_H
