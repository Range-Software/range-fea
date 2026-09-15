#ifndef RENDER_BACKEND_H
#define RENDER_BACKEND_H

#include <QList>
#include <QString>
#include <QStringList>

//! Rendering backend selection.
//!
//! The application can render either through the legacy OpenGL pipeline
//! (QOpenGLWidget + GLSL 1.20 shaders) or through Qt's Rendering Hardware
//! Interface (QRhiWidget + baked .qsb shaders).  The backend is chosen once
//! at start-up from the command line and stays fixed for the whole session.
namespace RenderBackend
{

    //! Available rendering backends.
    enum class Type
    {
        OpenGL = 0,
        Rhi
    };

    //! Graphics API used when Type::Rhi is selected.
    enum class RhiApi
    {
        Auto = 0,   //!< Platform default (Metal on macOS, D3D11 on Windows, Vulkan/OpenGL elsewhere).
        OpenGL,
        Vulkan,
        Metal,
        D3D11,
        D3D12,
        Null
    };

    //! Return the active rendering backend.
    Type getType();

    //! Set the active rendering backend.  Must be called before any GLWidget is created.
    void setType(Type type);

    //! Return true if the legacy OpenGL backend is active.
    bool isOpenGL();

    //! Return true if the QRhi backend is active.
    bool isRhi();

    //! Return the graphics API requested for the QRhi backend.
    RhiApi getRhiApi();

    //! Set the graphics API to be used by the QRhi backend.
    void setRhiApi(RhiApi api);

    //! Convert backend type to its command-line name.
    QString toString(Type type);

    //! Convert command-line name to backend type.  Sets *ok to false on unknown input.
    Type typeFromString(const QString &name, bool *ok = nullptr);

    //! Convert RHI graphics API to its command-line name.
    QString toString(RhiApi api);

    //! Convert command-line name to RHI graphics API.  Sets *ok to false on unknown input.
    RhiApi rhiApiFromString(const QString &name, bool *ok = nullptr);

    //! Human readable name of a backend type, for use in the settings dialog.
    QString toDisplayString(Type type);

    //! Human readable name of a graphics API, for use in the settings dialog.
    QString toDisplayString(RhiApi api);

    //! List of accepted --render-backend values.
    QStringList getTypeNames();

    //! List of accepted --rhi-api values.
    QStringList getRhiApiNames();

    //! Backend types offered to the user.
    QList<Type> getTypes();

    //! Graphics APIs that can plausibly be used on the platform being built for.
    QList<RhiApi> getAvailableRhiApis();

}

#endif // RENDER_BACKEND_H
