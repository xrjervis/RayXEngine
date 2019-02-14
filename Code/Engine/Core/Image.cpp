#include "Engine/Core/Image.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image_write.h"

Image::Image(const std::string& imageFileName, bool flipY) {
	int numComponentsRequested = 4; // we support 4 (RGBA)

	std::string imageFilePath(imageFileName);

	// Load (and decompress) the image RGB(A) bytes from a file on disk
	stbi_set_flip_vertically_on_load(flipY);
	m_imageData = stbi_load(imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &m_numComponents, numComponentsRequested);
	GUARANTEE_OR_DIE((m_dimensions.x != 0.f) && (m_dimensions.y != 0.f), "Failed to load image.");
	PopulateFromData(m_imageData, m_dimensions, m_numComponents);
}

Image::Image() 
	: m_dimensions(0, 0)
	, m_numComponents(4)
	, m_imageData(nullptr){

}

Image::~Image() {
	stbi_image_free(m_imageData);
}

void* Image::GetBuffer(u32 x, u32 y) const{
	uint idx = m_dimensions.x * m_numComponents * y + x * m_numComponents;
	return m_imageData + idx;
}

Rgba Image::GetTexel(u32 x, u32 y) const {
	uint idx = m_dimensions.x * y + x;
	return m_texels[idx];
}

void Image::PushTexel(const Rgba& color) {
	m_texels.push_back(color);
}

bool Image::WriteToFile(const std::string& filepath, int width, int height) {
	stbi_flip_vertically_on_write(1);
	stbi_write_png(filepath.c_str(), width, height, m_numComponents, m_texels.data(), 4 * width);
	return true;
}

void Image::PopulateFromData(unsigned char* imageData, const IntVector2& texelSize, int numComponents) {
	int imageDataIndex = 0;
	Rgba rgba;
	m_numComponents = numComponents;
	for (int i = 0; i < texelSize.y * texelSize.x; ++i) {
		if (numComponents == 3) {
			rgba = Rgba(imageData[imageDataIndex], imageData[imageDataIndex + 1], imageData[imageDataIndex + 2]);
			imageDataIndex += 3;
		}
		else if (numComponents == 4) {
			rgba = Rgba(imageData[imageDataIndex], imageData[imageDataIndex + 1], imageData[imageDataIndex + 2], imageData[imageDataIndex + 3]);
			imageDataIndex += 4;
		}
		m_texels.push_back(rgba);
	}
}
