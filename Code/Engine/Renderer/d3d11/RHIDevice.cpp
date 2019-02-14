#include "Engine/Renderer/d3d11/RHIDevice.hpp"

RHIDevice::RHIDevice(RHIInstance* rhi, const HardwareDeviceInfo_t& hardwareInfo)
	: m_hardwareInfo(hardwareInfo) 
	, m_rhi(rhi) {

}

RHIDevice::~RHIDevice() {
	m_d3d11Context->ClearState();
}

std::unique_ptr<RHIOutput> RHIDevice::CreateOutput(const std::wstring& windowTitle, float height, float aspect) {
	std::unique_ptr<RHIOutput> output = std::make_unique<RHIOutput>(this, windowTitle, height, aspect);
	return output;
}

std::unique_ptr<Texture2D> RHIDevice::CreateTexture2DFromSwapChain(RHIOutput* output) {
	std::unique_ptr<Texture2D> texture = std::make_unique<Texture2D>(this);
	HR(output->m_swapChain->GetBuffer(0, IID_PPV_ARGS(texture->m_d3d11Texture2D.ReleaseAndGetAddressOf())));
	return texture;
}

std::unique_ptr<Texture2D> RHIDevice::CreateTexture2DFromFile(const std::string& filePath) {
	std::unique_ptr<Image> image = std::make_unique<Image>(filePath, false);

	TextureDefinition_t texDefn;
	texDefn.m_format = TEXTURE_FORMAT_R8G8B8A8_UNORM;
	texDefn.m_dimension = Vector2(image->m_dimensions);
	texDefn.m_bindFlags = BIND_SHADER_RESOURCE;
	texDefn.m_initialData = image->m_imageData;
	texDefn.m_stride = image->m_numComponents;

	std::unique_ptr<Texture2D> texture = CreateTexture2D(texDefn);
	texture->CreateShaderResourceView();
	texture->m_image = std::move(image);

	return texture;
}

std::unique_ptr<Texture2D> RHIDevice::CreateTexture2D(const TextureDefinition_t& defn) {
	D3D11_SUBRESOURCE_DATA* subresourcePtr = nullptr;
	if(defn.m_initialData != nullptr){
		D3D11_SUBRESOURCE_DATA subresource;
		memset(&subresource, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		subresource.pSysMem = defn.m_initialData;
		subresource.SysMemPitch = (u32)defn.m_dimension.x * defn.m_stride;
		subresourcePtr = &subresource;
	}


	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = (UINT)defn.m_dimension.x;
	texDesc.Height = (UINT)defn.m_dimension.y;
	texDesc.MipLevels = defn.m_mipLevels;
	texDesc.ArraySize = 1;
	texDesc.Format = ToDXGIFormat(defn.m_format);
	texDesc.SampleDesc.Count = defn.m_sampleCount;
	texDesc.SampleDesc.Quality = defn.m_sampleQuality;
	texDesc.Usage = ToDXMemoryUsage(defn.m_usage);
	texDesc.BindFlags = ToDXBindFlags(defn.m_bindFlags);
	if (defn.m_usage == MEMORY_USAGE_DYNAMIC) {
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (defn.m_usage == MEMORY_USAGE_STAGING) {
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}
	else {
		texDesc.CPUAccessFlags = 0;
	}

	if(defn.m_mipLevels > 1){
		texDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	else{
		texDesc.MiscFlags = 0;
	}

	std::unique_ptr<Texture2D> texture = std::make_unique<Texture2D>(this);
	texture->m_definition = defn;
	HR(m_d3d11Device->CreateTexture2D(&texDesc, subresourcePtr, texture->m_d3d11Texture2D.ReleaseAndGetAddressOf()));
	return texture;
}

std::unique_ptr<Buffer> RHIDevice::CreateVertexBuffer(u32 size, bool dynamic, bool streamout, void* initialData) {
	std::unique_ptr<Buffer>	buffer = std::make_unique<Buffer>(this);

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	if(streamout){
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	}
	else {
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	}

	if(dynamic) {
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else {
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = initialData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	HRESULT hr = (m_d3d11Device->CreateBuffer(&desc, &InitData, buffer->m_d3d11Buffer.ReleaseAndGetAddressOf()));

	if(FAILED(hr)){
		DebuggerPrintf("Failed to create vertex buffer");
	}
	return buffer;
}

std::unique_ptr<Buffer> RHIDevice::CreateIndexBuffer(u32 size, bool dynamic, void* initialData) {
	std::unique_ptr<Buffer>	buffer = std::make_unique<Buffer>(this);

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	if(dynamic) {
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = initialData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	HR(m_d3d11Device->CreateBuffer(&desc, &InitData, buffer->m_d3d11Buffer.ReleaseAndGetAddressOf()));

	return buffer;
}

std::unique_ptr<Buffer> RHIDevice::CreateConstantBuffer(u32 size, bool dynamic, bool cpuUpdates, void* initialData) {
	D3D11_SUBRESOURCE_DATA* subresourcePtr = nullptr;
	if (initialData != nullptr) {
		D3D11_SUBRESOURCE_DATA subresource;
		memset(&subresource, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		subresource.pSysMem = initialData;
		subresource.SysMemPitch = 0;
		subresource.SysMemSlicePitch = 0;
		subresourcePtr = &subresource;
	}

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if(dynamic && cpuUpdates){
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (dynamic && !cpuUpdates){
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = 0;
	}
	else{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}

	std::unique_ptr<Buffer>	buffer = std::make_unique<Buffer>(this);
	HR(m_d3d11Device->CreateBuffer(&desc, subresourcePtr, buffer->m_d3d11Buffer.ReleaseAndGetAddressOf()));

	return buffer;
}

std::unique_ptr<ShaderProgram> RHIDevice::CreateVertexShader(const std::string& filePath) {
	std::unique_ptr<ShaderProgram> shader = std::make_unique<ShaderProgram>(filePath, SHADER_TYPE_VERTEX_SHADER);
	HR(m_d3d11Device->CreateVertexShader(shader->m_compiledBlob->GetBufferPointer(), shader->m_compiledBlob->GetBufferSize(), NULL, shader->m_d3d11VertexShader.GetAddressOf()));
	return shader;
}

std::unique_ptr<ShaderProgram> RHIDevice::CreatePixelShader(const std::string& filePath) {
	std::unique_ptr<ShaderProgram> shader = std::make_unique<ShaderProgram>(filePath, SHADER_TYPE_PIXEL_SHADER);
	HR(m_d3d11Device->CreatePixelShader(shader->m_compiledBlob->GetBufferPointer(), shader->m_compiledBlob->GetBufferSize(), NULL, shader->m_d3d11PixelShader.GetAddressOf()));
	return shader;
}

std::unique_ptr<InputLayout> RHIDevice::CreateInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& descs, ShaderProgram* vs) {
	std::unique_ptr<InputLayout> inputLayout = std::make_unique<InputLayout>();
	HR(m_d3d11Device->CreateInputLayout(descs.data(), (UINT)descs.size(), vs->m_compiledBlob->GetBufferPointer(), vs->m_compiledBlob->GetBufferSize(),
		inputLayout->m_d3d11InputLayout.ReleaseAndGetAddressOf()));
	return inputLayout;
}

std::unique_ptr<RenderState> RHIDevice::CreateRasterizerState(eFillMode fillMode, eCullMode cullMode, bool frontCounterClockwise) {
	D3D11_RASTERIZER_DESC desc = {};
	desc.FillMode = ToDXFillMode(fillMode);
	desc.CullMode = ToDXCullMode(cullMode);
	desc.FrontCounterClockwise = frontCounterClockwise;
	desc.DepthClipEnable = true;

	std::unique_ptr<RenderState> rs = std::make_unique<RenderState>();
	HR(m_d3d11Device->CreateRasterizerState(&desc, rs->m_d3d11RasterizerState.GetAddressOf()));
	return rs;
}

std::unique_ptr<RenderState> RHIDevice::CreateBlendState(eBlendFactor src, eBlendFactor dst, eBlendOp op) {
	D3D11_BLEND_DESC desc = {};
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = ToDXBlend(src);
	desc.RenderTarget[0].DestBlend = ToDXBlend(dst);
	desc.RenderTarget[0].BlendOp = ToDXBlendOp(op);
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	std::unique_ptr<RenderState> bs = std::make_unique<RenderState>();
	HR(m_d3d11Device->CreateBlendState(&desc, bs->m_d3d11BlendState.GetAddressOf()));
	return bs;
}

std::unique_ptr<RenderState> RHIDevice::CreateDepthStencilState(bool depthTestEnable, eComparisonFunc compFunc, eStencilOp op) {
	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = depthTestEnable;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = ToDXComparisonFunc(compFunc);

	// Stencil test parameters
	desc.StencilEnable = false;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	op = STENCIL_OP_KEEP;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	std::unique_ptr<RenderState> ds = std::make_unique<RenderState>();
	HR(m_d3d11Device->CreateDepthStencilState(&desc, ds->m_d3d11DepthStencilState.GetAddressOf()));
	return ds;
}

std::unique_ptr<Sampler> RHIDevice::CreateSampler(eFilterType filter, eWrapMode uWrap, eWrapMode vWrap, eWrapMode wWrap, u32 anisotropicAmount /*= 0U*/) {
	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = ToDXFilter(filter);
	desc.MaxAnisotropy = anisotropicAmount;
	desc.AddressU = ToDXWrapMode(uWrap);
	desc.AddressV = ToDXWrapMode(vWrap);
	desc.AddressW = ToDXWrapMode(wWrap);
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	std::unique_ptr<Sampler> sampler = std::make_unique<Sampler>();
	HR(m_d3d11Device->CreateSamplerState(&desc, sampler->m_d3d11Sampler.GetAddressOf()));
	return sampler;
}

void RHIDevice::ClearColor(RenderTargetView* rtv, const Rgba& color) {
	Vector4 c = color.GetAsFloats();
	float* colorFloats = reinterpret_cast<float*>(&c);
	m_d3d11Context->ClearRenderTargetView(rtv->m_d3d11RTV.Get(), colorFloats);
}

void RHIDevice::ClearDepthStencil(DepthStencilView* dsv, float depth /*= 1.0f*/, u8 stencil /*= 0U*/) {
	m_d3d11Context->ClearDepthStencilView(dsv->m_d3d11DSV.Get(), D3D11_CLEAR_DEPTH, depth, stencil);
}

void RHIDevice::UpdateBuffer(Buffer* buffer, void* pData) {
	m_d3d11Context->UpdateSubresource(buffer->m_d3d11Buffer.Get(), 0, nullptr, pData, 0, 0);
}

void RHIDevice::DrawIndexed(u32 indexCount, u32 start /*= 0U*/, u32 base /*= 0U*/) {
	m_d3d11Context->DrawIndexed(indexCount, start, base);
}

void RHIDevice::Draw(u32 vertexCount, u32 start) {
	m_d3d11Context->Draw(vertexCount, start);
}
