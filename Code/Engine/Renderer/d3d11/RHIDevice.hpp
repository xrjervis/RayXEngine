#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"
#include "Engine/Renderer/d3d11/Texture.hpp"
#include "Engine/Renderer/d3d11/Buffer.hpp"
#include "Engine/Renderer/d3d11/RHIOutput.hpp"
#include "Engine/Renderer/d3d11/RenderState.hpp"
#include "Engine/Renderer/d3d11/Sampler.hpp"
#include "Engine/Renderer/d3d11/ShaderProgram.hpp"
#include "Engine/Renderer/d3d11/InputLayout.hpp"
#include "Engine/Core/Rgba.hpp"
#include <memory>
#include <string>

class RHIInstance;
class Window;

struct HardwareDeviceInfo_t {
	UINT adapterIndex;
	DXGI_ADAPTER_DESC1 adapterDesc;
};

class RHIDevice{
public:
	RHIDevice(RHIInstance* rhi, const HardwareDeviceInfo_t& hardwareInfo);
	~RHIDevice();

	std::unique_ptr<RHIOutput>		CreateOutput(const std::wstring& windowTitle, eWindowMode mode, float aspect);

	std::unique_ptr<Texture2D>		CreateTexture2D(const TextureDefinition_t& info);
	std::unique_ptr<Texture2D>		CreateTexture2DFromSwapChain(RHIOutput* output);
	std::unique_ptr<Texture2D>		CreateTexture2DFromFile(const std::string& filePath);
	std::unique_ptr<Buffer>			CreateVertexBuffer(u32 size, bool dynamic, bool streamout, void* initialData);
	std::unique_ptr<Buffer>			CreateIndexBuffer(u32 size, bool dynamic, void* initialData);
	std::unique_ptr<Buffer>			CreateConstantBuffer(u32 size, bool dynamic, bool cpuUpdates, void* initialData);
	std::unique_ptr<ShaderProgram>	CreateVertexShader(const std::string& filePath);
	std::unique_ptr<ShaderProgram>	CreatePixelShader(const std::string& filePath);
	std::unique_ptr<InputLayout>	CreateInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& descs, ShaderProgram* vs);
	std::unique_ptr<RenderState>	CreateRasterizerState(eFillMode fillMode, eCullMode cullMode, bool frontCounterClockwise);
	std::unique_ptr<RenderState>	CreateBlendState(eBlendFactor src, eBlendFactor dst, eBlendOp op);
	std::unique_ptr<RenderState>	CreateDepthStencilState(bool depthTestEnable, eComparisonFunc compFunc, eStencilOp op);
	std::unique_ptr<Sampler>		CreateSampler(eFilterType filter, eWrapMode uWrap, eWrapMode vWrap, eWrapMode wWrap, u32 anisotropicAmount = 0U);

	void							ClearColor(RenderTargetView* rtv, const Rgba& color);
	void							ClearDepthStencil(DepthStencilView* dsv, float depth = 1.0f, u8 stencil = 0U);
	void							UpdateBuffer(Buffer* buffer, void* pData);

	void							DrawIndexed(u32 indexCount, u32 start = 0U, u32 base = 0U);
	void							Draw(u32 vertexCount, u32 start = 0U);

public:
	ComPtr<ID3D11Device1>			m_d3d11Device;
	ComPtr<ID3D11DeviceContext1>	m_d3d11Context;

	HardwareDeviceInfo_t			m_hardwareInfo;
	RHIInstance*					m_rhi = nullptr;
};
