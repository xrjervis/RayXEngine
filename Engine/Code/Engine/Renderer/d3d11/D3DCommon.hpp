#pragma once
#include <WinSDKVer.h>
#include <SDKDDKVer.h>

// Use the C++ standard template min/max
#define NOMINMAX

// DirectX apps don't need GDI
// #define NODRAWTEXT
// #define NOGDI
// #define NOBITMAP

// DirectX includes
#if defined(NTDDI_WIN10_RS2)
#include <dxgi1_6.h>
#else
#include <dxgi1_5.h>
#endif

#include <d2d1_2.h>
#include <dwrite_2.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include "Engine/Renderer/external/d3d11/DXErr.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/type.hpp"
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

using namespace Microsoft::WRL;
#define DX_SAFE_RELEASE(p) if (nullptr != (p)) { (p)->Release(); (p) = nullptr; }

#ifdef _DEBUG
#ifndef HR
#define HR(x)													\
{																\
    HRESULT _hr = (x);											\
    if(FAILED(_hr))												\
    {															\
        DXTrace(__FILEW__, (DWORD)__LINE__, _hr, L#x, true);	\
    }															\
}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif

constexpr size_t VERTEX_BUFFER_SLOT_COUNT = 8;
constexpr size_t CONSTANT_BUFFER_SLOT_COUNT = 8;
constexpr size_t SHADER_RESOURCE_SLOT_COUNT = 8;
constexpr size_t SAMPLER_SLOT_COUNT = 16;
constexpr size_t VIEWPORT_SLOT_COUNT = 16;
constexpr size_t RTV_SLOT_COUNT = 8;

D3D11_USAGE					ToDXMemoryUsage(eMemoryUsage usage);
u32							ToDXBindFlags(u32 flags);
DXGI_FORMAT					ToDXGIFormat(eTextureFormat textureFormat);
D3D11_PRIMITIVE_TOPOLOGY	ToDXPrimitiveType(ePrimitiveType primitiveType);
D3D11_FILL_MODE				ToDXFillMode(eFillMode fillMode);
D3D11_CULL_MODE				ToDXCullMode(eCullMode cullMode);
D3D11_COMPARISON_FUNC		ToDXComparisonFunc(eComparisonFunc compare);
D3D11_BLEND					ToDXBlend(eBlendFactor blend);
D3D11_BLEND_OP				ToDXBlendOp(eBlendOp blendOp);
D3D11_STENCIL_OP			ToDXStencilOp(eStencilOp stencilOp);
std::string					ToDXShaderEntryPoint(eShaderType type);
std::string					ToDXShaderCompileModel(eShaderType type);
D3D11_TEXTURE_ADDRESS_MODE	ToDXWrapMode(eWrapMode wrap);
D3D11_FILTER				ToDXFilter(eFilterType filter);

eWrapMode					ToWrapMode(const std::string& str);
eFillMode					ToFillMode(const std::string& str);
eCullMode					ToCullMode(const std::string& str);
eBlendFactor				ToBlendFactor(const std::string& str);
eBlendOp					ToBlendOp(const std::string& str);
eComparisonFunc				ToCompareFunc(const std::string& str);