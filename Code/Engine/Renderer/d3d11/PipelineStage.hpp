#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"
#include "Engine/Renderer/d3d11/Buffer.hpp"
#include "Engine/Renderer/d3d11/InputLayout.hpp"
#include "Engine/Renderer/d3d11/ShaderProgram.hpp"
#include "Engine/Renderer/d3d11/ResourceView.hpp"
#include "Engine/Renderer/d3d11/Sampler.hpp"
#include "Engine/Renderer/d3d11/RenderState.hpp"

// Input Assembler stage
struct Stage_IA_t{
	u32							vertexBufferStart;
	u32							numVertexBuffers;
	Buffer*						vertexBuffers[VERTEX_BUFFER_SLOT_COUNT];
	u32							vertexBufferStrides[VERTEX_BUFFER_SLOT_COUNT];
	u32							vertexBufferOffsets[VERTEX_BUFFER_SLOT_COUNT];

	Buffer*						indexBuffer = nullptr;
	u32							indexBufferOffset;

	InputLayout*				inputLayout = nullptr;
	ePrimitiveType				primitiveTopology;
};


// Vertex Shader stage
struct Stage_VS_t{
	ShaderProgram*				vertexShader = nullptr;

	u32							constantBufferStart;
	u32							numConstantBuffers;
	Buffer*						constantBuffers[CONSTANT_BUFFER_SLOT_COUNT];
};

struct Stage_RS_t {
	u32							numViewports;
	D3D11_VIEWPORT				viewports[VIEWPORT_SLOT_COUNT];
	RenderState*				rasterizerState;
};

struct Stage_PS_t {
	ShaderProgram*				pixelShader = nullptr;

	u32							constantBufferStart;
	u32							numConstantBuffers;
	Buffer*						constantBuffers[CONSTANT_BUFFER_SLOT_COUNT];

	u32							shaderResourceStart;
	u32							numShaderResources;
	ShaderResourceView*			shaderResourceViews[SHADER_RESOURCE_SLOT_COUNT];

	u32							samplerStart;
	u32							numSampler;
	Sampler*					samplers[SAMPLER_SLOT_COUNT];
};

// Output Merger stage
struct Stage_OM_t {
	RenderState*				depthStencilState;
	RenderState*				blendState;

	u32							numRenderTargetViews;
	RenderTargetView*			renderTargetViews[RTV_SLOT_COUNT];
	DepthStencilView*			depthStencilView;
};

