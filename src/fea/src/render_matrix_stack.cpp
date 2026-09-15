#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <vector>

#include <QtOpenGL>

#include "render_backend.h"
#include "render_matrix_stack.h"

namespace
{

    QMatrix4x4               projectionMatrix;
    QMatrix4x4               modelViewMatrix;
    std::vector<QMatrix4x4>  modelViewStack;
    int                      viewportRect[4] = { 0, 0, 0, 0 };

    //! Push the CPU-side model-view matrix into the fixed-function GL state.
    void syncModelViewToGL()
    {
        if (!RenderBackend::isOpenGL())
        {
            return;
        }
        double m[16];
        RenderMatrixStack::toGL(modelViewMatrix, m);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixd(m);
    }

    //! Push the CPU-side projection matrix into the fixed-function GL state.
    void syncProjectionToGL()
    {
        if (!RenderBackend::isOpenGL())
        {
            return;
        }
        double m[16];
        RenderMatrixStack::toGL(projectionMatrix, m);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(m);
        glMatrixMode(GL_MODELVIEW);
    }

}

QMatrix4x4 RenderMatrixStack::fromGL(const double matrix[16])
{
    // QMatrix4x4(m11,m12,...) takes values in ROW-major order, GL arrays are column-major.
    return QMatrix4x4(float(matrix[0]), float(matrix[4]), float(matrix[8]),  float(matrix[12]),
                      float(matrix[1]), float(matrix[5]), float(matrix[9]),  float(matrix[13]),
                      float(matrix[2]), float(matrix[6]), float(matrix[10]), float(matrix[14]),
                      float(matrix[3]), float(matrix[7]), float(matrix[11]), float(matrix[15]));
}

void RenderMatrixStack::toGL(const QMatrix4x4 &matrix, double out[16])
{
    // constData() returns column-major data — direct element-wise copy.
    const float *d = matrix.constData();
    for (int i = 0; i < 16; i++)
    {
        out[i] = double(d[i]);
    }
}

void RenderMatrixStack::reset()
{
    projectionMatrix.setToIdentity();
    modelViewMatrix.setToIdentity();
    modelViewStack.clear();
}

void RenderMatrixStack::setViewport(int x, int y, int width, int height)
{
    viewportRect[0] = x;
    viewportRect[1] = y;
    viewportRect[2] = width;
    viewportRect[3] = height;

    if (RenderBackend::isOpenGL())
    {
        glViewport(GLint(x), GLint(y), GLsizei(width), GLsizei(height));
    }
}

void RenderMatrixStack::getViewport(int out[4])
{
    for (int i = 0; i < 4; i++)
    {
        out[i] = viewportRect[i];
    }
}

void RenderMatrixStack::setOrtho(double left, double right, double bottom, double top, double nearPlane, double farPlane)
{
    QMatrix4x4 m;
    m.ortho(float(left), float(right), float(bottom), float(top), float(nearPlane), float(farPlane));
    RenderMatrixStack::setProjection(m);
}

void RenderMatrixStack::setProjection(const QMatrix4x4 &matrix)
{
    projectionMatrix = matrix;
    syncProjectionToGL();
}

const QMatrix4x4 &RenderMatrixStack::getProjection()
{
    return projectionMatrix;
}

void RenderMatrixStack::getProjectionGL(double out[16])
{
    RenderMatrixStack::toGL(projectionMatrix, out);
}

void RenderMatrixStack::pushMatrix()
{
    modelViewStack.push_back(modelViewMatrix);
}

void RenderMatrixStack::popMatrix()
{
    if (modelViewStack.empty())
    {
        return;
    }
    modelViewMatrix = modelViewStack.back();
    modelViewStack.pop_back();
    syncModelViewToGL();
}

void RenderMatrixStack::loadIdentity()
{
    modelViewMatrix.setToIdentity();
    syncModelViewToGL();
}

void RenderMatrixStack::loadMatrix(const double matrix[16])
{
    modelViewMatrix = RenderMatrixStack::fromGL(matrix);
    syncModelViewToGL();
}

void RenderMatrixStack::loadMatrix(const QMatrix4x4 &matrix)
{
    modelViewMatrix = matrix;
    syncModelViewToGL();
}

void RenderMatrixStack::multMatrix(const QMatrix4x4 &matrix)
{
    modelViewMatrix *= matrix;
    syncModelViewToGL();
}

void RenderMatrixStack::translate(double x, double y, double z)
{
    modelViewMatrix.translate(float(x), float(y), float(z));
    syncModelViewToGL();
}

void RenderMatrixStack::scale(double x, double y, double z)
{
    modelViewMatrix.scale(float(x), float(y), float(z));
    syncModelViewToGL();
}

void RenderMatrixStack::rotate(float angle, float x, float y, float z)
{
    modelViewMatrix.rotate(angle, x, y, z);
    syncModelViewToGL();
}

const QMatrix4x4 &RenderMatrixStack::getModelView()
{
    return modelViewMatrix;
}

void RenderMatrixStack::getModelViewGL(double out[16])
{
    RenderMatrixStack::toGL(modelViewMatrix, out);
}
