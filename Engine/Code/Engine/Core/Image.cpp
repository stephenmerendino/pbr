#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

Image::Image()
	:m_width(0),
	 m_height(0),
	 m_bytesPerTexel(0),
	 m_imageTexelBytes(nullptr)
{
}

Image::Image(const std::string& imageFilePath, const ImageLoadMode loadMode)
	:m_width(0),
	 m_height(0),
	 m_bytesPerTexel(0),
	 m_imageTexelBytes(nullptr)
{
	switch (loadMode){
		case IMAGE_LOAD_MODE_FORCE_ALPHA:
		{
			m_imageTexelBytes = stbi_load(imageFilePath.c_str(), &m_width, &m_height, &m_bytesPerTexel, 4);
		} break;

		default:
		case IMAGE_LOAD_MODE_DEFAULT:
		{
			m_imageTexelBytes = stbi_load(imageFilePath.c_str(), &m_width, &m_height, &m_bytesPerTexel, 0);
		} break;
	}

	ASSERT_OR_DIE(m_imageTexelBytes != 0, Stringf("Failed to load image: %s\n", imageFilePath.c_str()).c_str());
}

Image::Image(uint width, uint height, unsigned char* raw_image_data)
	:m_width(width)
	,m_height(height)
{
	m_imageTexelBytes = (unsigned char*)malloc(m_width * m_height * 4);
	memcpy(m_imageTexelBytes, raw_image_data, m_width * m_height * 4);
}

Image::~Image(){
    free(m_imageTexelBytes);
}

Rgba Image::GetTexelColorAtIndex(unsigned int texelIndex){
	int byteOffset = texelIndex * m_bytesPerTexel;

	Rgba texelColor;
	texelColor.r = m_imageTexelBytes[byteOffset];
	texelColor.g = m_imageTexelBytes[byteOffset + 1];
	texelColor.b = m_imageTexelBytes[byteOffset + 2];

	if(m_bytesPerTexel == 4){
		texelColor.a = m_imageTexelBytes[byteOffset + 3];
	} else{
		texelColor.a = 255;
	}

	return texelColor;
}

Rgba Image::GetTexelColorAtPosition(unsigned int xIndex, unsigned int yIndex){
	unsigned int texelIndex = (yIndex * m_width) + xIndex;
	return GetTexelColorAtIndex(texelIndex);
}

bool Image::save_to_png(const char* filename)
{
	return Image::save_to_png(m_width, m_height, m_imageTexelBytes, filename);
}

bool Image::save_to_png(uint width, uint height, unsigned char* raw_image_data, const char* filename)
{
	int ret = stbi_write_png(filename, width, height, 4, raw_image_data, width * 4);
	return (ret != 0);
}