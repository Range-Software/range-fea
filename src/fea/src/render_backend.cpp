#include <QtGlobal>

#include "render_backend.h"

namespace
{

    RenderBackend::Type   activeType   = RenderBackend::Type::OpenGL;
    RenderBackend::RhiApi activeRhiApi = RenderBackend::RhiApi::Auto;

}

RenderBackend::Type RenderBackend::getType()
{
    return activeType;
}

void RenderBackend::setType(RenderBackend::Type type)
{
    activeType = type;
}

bool RenderBackend::isOpenGL()
{
    return activeType == RenderBackend::Type::OpenGL;
}

bool RenderBackend::isRhi()
{
    return activeType == RenderBackend::Type::Rhi;
}

RenderBackend::RhiApi RenderBackend::getRhiApi()
{
    return activeRhiApi;
}

void RenderBackend::setRhiApi(RenderBackend::RhiApi api)
{
    activeRhiApi = api;
}

QString RenderBackend::toString(RenderBackend::Type type)
{
    switch (type)
    {
        case RenderBackend::Type::Rhi:    return QString("rhi");
        case RenderBackend::Type::OpenGL: return QString("opengl");
    }
    return QString("opengl");
}

RenderBackend::Type RenderBackend::typeFromString(const QString &name, bool *ok)
{
    const QString key(name.trimmed().toLower());

    if (ok)
    {
        *ok = true;
    }
    if (key == "opengl" || key == "gl" || key == "legacy")
    {
        return RenderBackend::Type::OpenGL;
    }
    if (key == "rhi" || key == "qrhi")
    {
        return RenderBackend::Type::Rhi;
    }
    if (ok)
    {
        *ok = false;
    }
    return RenderBackend::Type::OpenGL;
}

QString RenderBackend::toString(RenderBackend::RhiApi api)
{
    switch (api)
    {
        case RenderBackend::RhiApi::Auto:   return QString("auto");
        case RenderBackend::RhiApi::OpenGL: return QString("opengl");
        case RenderBackend::RhiApi::Vulkan: return QString("vulkan");
        case RenderBackend::RhiApi::Metal:  return QString("metal");
        case RenderBackend::RhiApi::D3D11:  return QString("d3d11");
        case RenderBackend::RhiApi::D3D12:  return QString("d3d12");
        case RenderBackend::RhiApi::Null:   return QString("null");
    }
    return QString("auto");
}

RenderBackend::RhiApi RenderBackend::rhiApiFromString(const QString &name, bool *ok)
{
    const QString key(name.trimmed().toLower());

    if (ok)
    {
        *ok = true;
    }
    if (key.isEmpty() || key == "auto" || key == "default")
    {
        return RenderBackend::RhiApi::Auto;
    }
    if (key == "opengl" || key == "gl" || key == "gles" || key == "opengles")
    {
        return RenderBackend::RhiApi::OpenGL;
    }
    if (key == "vulkan" || key == "vk")
    {
        return RenderBackend::RhiApi::Vulkan;
    }
    if (key == "metal" || key == "mtl")
    {
        return RenderBackend::RhiApi::Metal;
    }
    if (key == "d3d11" || key == "direct3d11")
    {
        return RenderBackend::RhiApi::D3D11;
    }
    if (key == "d3d12" || key == "direct3d12")
    {
        return RenderBackend::RhiApi::D3D12;
    }
    if (key == "null")
    {
        return RenderBackend::RhiApi::Null;
    }
    if (ok)
    {
        *ok = false;
    }
    return RenderBackend::RhiApi::Auto;
}

QString RenderBackend::toDisplayString(RenderBackend::Type type)
{
    switch (type)
    {
        case RenderBackend::Type::Rhi:    return QString("QRhi");
        case RenderBackend::Type::OpenGL: return QString("OpenGL");
    }
    return QString("OpenGL");
}

QString RenderBackend::toDisplayString(RenderBackend::RhiApi api)
{
    switch (api)
    {
        case RenderBackend::RhiApi::Auto:   return QString("Automatic");
        case RenderBackend::RhiApi::OpenGL: return QString("OpenGL");
        case RenderBackend::RhiApi::Vulkan: return QString("Vulkan");
        case RenderBackend::RhiApi::Metal:  return QString("Metal");
        case RenderBackend::RhiApi::D3D11:  return QString("Direct3D 11");
        case RenderBackend::RhiApi::D3D12:  return QString("Direct3D 12");
        case RenderBackend::RhiApi::Null:   return QString("Null");
    }
    return QString("Automatic");
}

QList<RenderBackend::Type> RenderBackend::getTypes()
{
    return QList<RenderBackend::Type>() << RenderBackend::Type::OpenGL
                                        << RenderBackend::Type::Rhi;
}

QList<RenderBackend::RhiApi> RenderBackend::getAvailableRhiApis()
{
    QList<RenderBackend::RhiApi> apis;

    apis << RenderBackend::RhiApi::Auto;
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    apis << RenderBackend::RhiApi::Metal;
#endif
#if defined(Q_OS_WIN)
    apis << RenderBackend::RhiApi::D3D11;
    apis << RenderBackend::RhiApi::D3D12;
#endif
    apis << RenderBackend::RhiApi::Vulkan;
    apis << RenderBackend::RhiApi::OpenGL;
    apis << RenderBackend::RhiApi::Null;

    return apis;
}

QStringList RenderBackend::getTypeNames()
{
    return QStringList() << "opengl" << "rhi";
}

QStringList RenderBackend::getRhiApiNames()
{
    return QStringList() << "auto" << "opengl" << "vulkan" << "metal" << "d3d11" << "d3d12" << "null";
}
