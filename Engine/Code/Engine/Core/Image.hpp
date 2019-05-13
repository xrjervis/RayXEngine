#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/type.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <vector>
#include <string>

class Image{

public:
	Image();
	explicit Image(const std::string& imageFileName, bool flipY = false);  // origin is at top left corner by default
	~Image();

	void*	GetBuffer(u32 x, u32 y) const;
	Rgba	GetTexel(u32 x, u32 y) const;

	void	PushTexel(const Rgba& color);
	bool	WriteToFile(const std::string& filepath, int width, int height);

private:
	void	PopulateFromData(unsigned char* imageData, const IntVector2& texelSize, int numComponents);

public:
	IntVector2						m_dimensions;
	int								m_numComponents = 0;
	unsigned char*					m_imageData;
	std::vector<Rgba>				m_texels;
};