#include "Engine/Renderer/d3d11/Buffer.hpp"

Buffer::Buffer(RHIDevice* device) 
	: RHIResource(device) {

}

Buffer::~Buffer() {
	m_d3d11Buffer.Reset();
}

