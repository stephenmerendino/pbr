#pragma once

#include "Engine/RHI/ShaderStage.hpp"
#include "Engine/Config/EngineConfig.hpp"

class HullShaderStage : public ShaderStage
{
public:
	ID3D11HullShader* m_dx_hull_shader;

public:
	HullShaderStage(RHIDevice* rhi_device, 
					const char* shader_raw_source, 
					const size_t shader_raw_source_size, 
					const char* entry_point = DEFAULT_HULL_SHADER_ENTRY_POINT,
		            const char* opt_filename = nullptr);

	virtual ~HullShaderStage() override;

	bool is_valid() const { return m_dx_hull_shader != nullptr; }
};