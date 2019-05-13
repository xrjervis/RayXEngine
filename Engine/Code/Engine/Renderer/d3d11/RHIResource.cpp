#include "Engine/Renderer/d3d11/RHIResource.hpp"

RHIResource::RHIResource(RHIDevice* device) 
	:m_device(device) {

}

RHIResource::~RHIResource() {

}

RHIDevice* RHIResource::GetDevice() const {
	return m_device;
}

