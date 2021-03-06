#pragma once

#include "Engine/RHI/ShaderStage.hpp"
#include "Engine/Config/EngineConfig.hpp"

class FragmentShaderStage : public ShaderStage
{
public:
	ID3D11PixelShader* m_dx_pixel_shader;

public:
	FragmentShaderStage(RHIDevice* rhi_device, 
					  const char* shader_raw_source, 
					  const size_t shader_raw_source_size, 
					  const char* entry_point = DEFAULT_FRAGMENT_SHADER_ENTRY_POINT,
		              const char* opt_filename = nullptr);

	virtual ~FragmentShaderStage() override;

	bool is_valid() const { return m_dx_pixel_shader != nullptr; }
};