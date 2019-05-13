#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"

class RHIDevice;

class RHIResource {
public:
	RHIResource(RHIDevice* device);
	virtual ~RHIResource();

	RHIDevice* GetDevice() const;

public:
	RHIDevice*	m_device = nullptr;
};
