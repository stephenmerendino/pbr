#include "Engine/Renderer/sprite.h"
#include "Engine/Engine.hpp"
#include "Engine/RHI/RHIDevice.hpp"

Sprite::Sprite()
	:m_texture(nullptr)
	,m_uv_region(AABB2::ZERO_TO_ONE)
{
}

Sprite::Sprite(const char* texture_filename, const AABB2& uv_region)
	:m_uv_region(uv_region)
{
	m_texture = g_theRenderer->m_device->FindOrCreateRHITexture2DFromFile(texture_filename);
}

Sprite::Sprite(RHITexture2D* texture, const AABB2& uv_region)
	:m_texture(texture)
	,m_uv_region(uv_region)
{
}

void Sprite::set_texture(RHITexture2D* texture)
{
	m_texture = texture;
}

void Sprite::set_uv_region(const AABB2& uv_region)
{
	m_uv_region = uv_region;
}