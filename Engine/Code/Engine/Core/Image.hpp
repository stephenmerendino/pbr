#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/types.h"
#include "Engine/Math/IntVector2.hpp"
#include <string>

enum ImageLoadMode{
	IMAGE_LOAD_MODE_DEFAULT,
	IMAGE_LOAD_MODE_FORCE_ALPHA,
	NUM_IMAGE_LOAD_MODES
};

class Image{
public:
	Image();
	Image(const std::string& imageFilePath, const ImageLoadMode loadMode = IMAGE_LOAD_MODE_DEFAULT);
	Image(uint width, uint height, unsigned char* raw_image_data);
	~Image();

	int GetWidth()								const		{ return m_width; }
	int GetHeight()								const		{ return m_height; }
	IntVector2 GetDimensions()					const		{ return IntVector2(m_width, m_height); }
	int GetNumTexels()							const		{ return m_width * m_height; }
	int GetBytesPerTexel()						const		{ return m_bytesPerTexel; }
	const unsigned char* GetImageTexelBytes()	const		{ return m_imageTexelBytes; }
    bool IsValid()                              const       { return nullptr != m_imageTexelBytes; }

	Rgba GetTexelColorAtIndex(unsigned int texelIndex);
	Rgba GetTexelColorAtPosition(unsigned int xIndex, unsigned int yIndex);

	bool save_to_png(const char* filename);

	static bool save_to_png(uint width, uint height, unsigned char* raw_image_data, const char* filename);

private:
	int m_width;
	int m_height;
	int m_bytesPerTexel;
	unsigned char* m_imageTexelBytes;
};