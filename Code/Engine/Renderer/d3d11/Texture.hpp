#pragma once
#include "Engine/Renderer/d3d11/RHIResource.hpp"
#include "Engine/Renderer/d3d11/ResourceView.hpp"
#include "Engine/Renderer/d3d11/Sampler.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vector3.hpp"

struct TextureDefinition_t{
	eTextureFormat	m_format;
	eMemoryUsage	m_usage;
	u32				m_bindFlags;
	u32				m_mipLevels = 1;
	Vector3			m_dimension;
	u32				m_sampleCount = 1;
	u32				m_sampleQuality = 0;
	void*			m_initialData = nullptr;
	u32				m_stride;
};

class Texture2D : public RHIResource {
public: 
	Texture2D(RHIDevice* device);
	virtual ~Texture2D();

	RenderTargetView*	CreateRenderTargetView();
	DepthStencilView*	CreateDepthStencilView();
	ShaderResourceView* CreateShaderResourceView();

public:
	TextureDefinition_t					m_definition;
	std::unique_ptr<Image>				m_image;

	ComPtr<ID3D11Texture2D>				m_d3d11Texture2D;
	std::unique_ptr<RenderTargetView>	m_rtv;
	std::unique_ptr<DepthStencilView>	m_dsv;
	std::unique_ptr<ShaderResourceView> m_srv;
	Sampler*							m_sampler = nullptr;
};