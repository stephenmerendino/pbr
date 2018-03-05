#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/Core/Image.hpp"
#include <map>

class RHIDevice;
class RHIOutput;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

class RHITexture2D : public RHITextureBase
{
public:
	ID3D11Texture2D*			m_dxTexture2D;
	unsigned int				m_width;
	unsigned int				m_height;
	unsigned int				m_sample_count;
	unsigned int				m_sample_quality;

public:
	RHITexture2D(RHIDevice* rhiDevice);
	RHITexture2D(RHIDevice* rhiDevice, RHIOutput* rhiOutput);
	RHITexture2D(RHIDevice* rhiDevice, const Image& image);
	RHITexture2D(RHIDevice* rhiDevice, const Rgba& color);
	RHITexture2D(RHIDevice* rhiDevice, const char* filename);
	RHITexture2D(RHIDevice* rhiDevice, unsigned int width, unsigned int height, ImageFormat format, uint sample_count = 1, uint sample_quality = 0, bool is_uav = false);
	RHITexture2D(ID3D11Texture2D* dxTexture, ID3D11ShaderResourceView* dxShaderResourceView);
	virtual ~RHITexture2D() override;

	bool make_depth_stencil_for_output(RHIOutput* output);

	unsigned int GetWidth() const { return m_width; }
	unsigned int GetHeight() const { return m_height; }
	float get_aspect() const;
	uint get_stride() const;

	inline bool IsValid() const { return (m_dxTexture2D != nullptr); }

	bool LoadFromFilename(const char* filename);
	bool LoadFromImage(const Image& image);
	bool LoadFromColor(const Rgba& color);

	bool LoadFromPerlinNoise(const unsigned int width, 
							 const unsigned int height, 
							 const bool greyscale = false, 
							 const float scale = 1.f, 
							 const unsigned int numOctaves = 1, 
							 const float octavePersistence = 0.5f, 
							 const float octaveScale = 2.f);

	bool load_from_binary_file(const char* filename, uint width, uint height, ImageFormat format);

	void CreateViews();
};