#include <cmath>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QtOpenGL>

#include "gl_frustum.h"

GLFrustum::GLFrustum()
{
    // Initialize planes to zero
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            this->planes[i][j] = 0.0;
        }
    }
}

void GLFrustum::extractFromGL()
{
    double modelview[16];
    double projection[16];
    double mvp[16];

    // Get current matrices from OpenGL
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    // Multiply projection * modelview to get combined matrix.
    // OpenGL matrices are column-major: element at (row r, col c) is stored at index c*4+r.
    // For C = A * B: C[i*4+j] = sum_k A[k*4+j] * B[i*4+k]
    // where i=col, j=row. Here A=projection, B=modelview.
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mvp[i * 4 + j] = 0.0;
            for (int k = 0; k < 4; k++)
            {
                mvp[i * 4 + j] += projection[k * 4 + j] * modelview[i * 4 + k];
            }
        }
    }

    this->extractFromMatrix(mvp);
}

void GLFrustum::extractFromMatrix(const double mvp[16])
{
    // Extract frustum planes using Gribb/Hartmann method
    // Each row of the MVP matrix contributes to the plane equations

    // Right plane: row4 - row1
    this->planes[Right][0] = mvp[3]  - mvp[0];
    this->planes[Right][1] = mvp[7]  - mvp[4];
    this->planes[Right][2] = mvp[11] - mvp[8];
    this->planes[Right][3] = mvp[15] - mvp[12];
    this->normalizePlane(Right);

    // Left plane: row4 + row1
    this->planes[Left][0] = mvp[3]  + mvp[0];
    this->planes[Left][1] = mvp[7]  + mvp[4];
    this->planes[Left][2] = mvp[11] + mvp[8];
    this->planes[Left][3] = mvp[15] + mvp[12];
    this->normalizePlane(Left);

    // Bottom plane: row4 + row2
    this->planes[Bottom][0] = mvp[3]  + mvp[1];
    this->planes[Bottom][1] = mvp[7]  + mvp[5];
    this->planes[Bottom][2] = mvp[11] + mvp[9];
    this->planes[Bottom][3] = mvp[15] + mvp[13];
    this->normalizePlane(Bottom);

    // Top plane: row4 - row2
    this->planes[Top][0] = mvp[3]  - mvp[1];
    this->planes[Top][1] = mvp[7]  - mvp[5];
    this->planes[Top][2] = mvp[11] - mvp[9];
    this->planes[Top][3] = mvp[15] - mvp[13];
    this->normalizePlane(Top);

    // Far plane: row4 - row3
    this->planes[Far][0] = mvp[3]  - mvp[2];
    this->planes[Far][1] = mvp[7]  - mvp[6];
    this->planes[Far][2] = mvp[11] - mvp[10];
    this->planes[Far][3] = mvp[15] - mvp[14];
    this->normalizePlane(Far);

    // Near plane: row4 + row3
    this->planes[Near][0] = mvp[3]  + mvp[2];
    this->planes[Near][1] = mvp[7]  + mvp[6];
    this->planes[Near][2] = mvp[11] + mvp[10];
    this->planes[Near][3] = mvp[15] + mvp[14];
    this->normalizePlane(Near);
}

void GLFrustum::normalizePlane(int planeIndex)
{
    double length = std::sqrt(
        this->planes[planeIndex][0] * this->planes[planeIndex][0] +
        this->planes[planeIndex][1] * this->planes[planeIndex][1] +
        this->planes[planeIndex][2] * this->planes[planeIndex][2]
    );

    if (length > 1e-10)
    {
        this->planes[planeIndex][0] /= length;
        this->planes[planeIndex][1] /= length;
        this->planes[planeIndex][2] /= length;
        this->planes[planeIndex][3] /= length;
    }
}

bool GLFrustum::containsPoint(double x, double y, double z) const
{
    for (int i = 0; i < 6; i++)
    {
        double distance = this->planes[i][0] * x +
                         this->planes[i][1] * y +
                         this->planes[i][2] * z +
                         this->planes[i][3];
        if (distance < 0.0)
        {
            return false;
        }
    }
    return true;
}

bool GLFrustum::intersectsBox(const RLimitBox &box) const
{
    double xMin, xMax, yMin, yMax, zMin, zMax;
    box.getLimits(xMin, xMax, yMin, yMax, zMin, zMax);
    return this->intersectsBox(xMin, xMax, yMin, yMax, zMin, zMax);
}

bool GLFrustum::intersectsBox(double xMin, double xMax,
                              double yMin, double yMax,
                              double zMin, double zMax) const
{
    // For each frustum plane, find the corner of the box that is most
    // in the direction of the plane normal (the "positive vertex").
    // If this corner is behind the plane, the entire box is outside.

    for (int i = 0; i < 6; i++)
    {
        // Find the positive vertex (corner furthest along plane normal)
        double px = (this->planes[i][0] >= 0.0) ? xMax : xMin;
        double py = (this->planes[i][1] >= 0.0) ? yMax : yMin;
        double pz = (this->planes[i][2] >= 0.0) ? zMax : zMin;

        // Calculate signed distance from plane
        double distance = this->planes[i][0] * px +
                         this->planes[i][1] * py +
                         this->planes[i][2] * pz +
                         this->planes[i][3];

        // If positive vertex is behind the plane, box is completely outside
        if (distance < 0.0)
        {
            return false;
        }
    }

    // Box is at least partially inside the frustum
    return true;
}
