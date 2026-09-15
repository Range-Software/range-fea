#ifndef RENDER_MATRIX_STACK_H
#define RENDER_MATRIX_STACK_H

#include <QMatrix4x4>

//! Backend independent replacement for the fixed-function matrix stack.
//!
//! The projection / model-view matrices and the viewport rectangle are always
//! tracked CPU-side.  When the legacy OpenGL backend is active every mutation
//! is additionally mirrored into the fixed-function GL state so that code which
//! still relies on it (immediate mode geometry drawn without a shader) keeps
//! working unchanged.  When the QRhi backend is active no GL call is made.
namespace RenderMatrixStack
{

    //! Reset projection, model-view stack and viewport to their defaults.
    void reset();

    // --- Viewport ----------------------------------------------------------

    //! Set the viewport rectangle in device pixels (OpenGL bottom-left origin).
    void setViewport(int x, int y, int width, int height);

    //! Copy the current viewport rectangle into out[4] = {x,y,width,height}.
    void getViewport(int out[4]);

    // --- Projection --------------------------------------------------------

    //! Replace the projection matrix by an orthographic projection.
    void setOrtho(double left, double right, double bottom, double top, double nearPlane, double farPlane);

    //! Replace the projection matrix.
    void setProjection(const QMatrix4x4 &matrix);

    //! Return the current projection matrix.
    const QMatrix4x4 &getProjection();

    //! Copy the current projection matrix into a column-major double[16].
    void getProjectionGL(double out[16]);

    // --- Model-view --------------------------------------------------------

    //! Push a copy of the current model-view matrix onto the stack.
    void pushMatrix();

    //! Pop the model-view matrix from the stack.
    void popMatrix();

    //! Set the model-view matrix to identity.
    void loadIdentity();

    //! Replace the model-view matrix by a column-major double[16].
    void loadMatrix(const double matrix[16]);

    //! Replace the model-view matrix.
    void loadMatrix(const QMatrix4x4 &matrix);

    //! Post-multiply the model-view matrix (same semantics as glMultMatrix).
    void multMatrix(const QMatrix4x4 &matrix);

    //! Translate the model-view matrix.
    void translate(double x, double y, double z);

    //! Scale the model-view matrix uniformly along each axis.
    void scale(double x, double y, double z);

    //! Rotate the model-view matrix by angle (degrees) around the given axis.
    void rotate(float angle, float x, float y, float z);

    //! Return the current model-view matrix.
    const QMatrix4x4 &getModelView();

    //! Copy the current model-view matrix into a column-major double[16].
    void getModelViewGL(double out[16]);

    // --- Helpers -----------------------------------------------------------

    //! Build a QMatrix4x4 from a column-major GL double[16] array.
    QMatrix4x4 fromGL(const double matrix[16]);

    //! Write a QMatrix4x4 into a column-major GL double[16] array.
    void toGL(const QMatrix4x4 &matrix, double out[16]);

}

#endif // RENDER_MATRIX_STACK_H
