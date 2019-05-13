#pragma once
#include "Engine/Renderer/d3d11/RHIResource.hpp"
#include "Engine/Core/type.hpp"

class Buffer : public RHIResource{
public:
	Buffer(RHIDevice* device);
	virtual ~Buffer();

	ComPtr<ID3D11Buffer> m_d3d11Buffer;
};