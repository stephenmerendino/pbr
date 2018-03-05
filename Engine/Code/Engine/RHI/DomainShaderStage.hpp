#pragma once

#include "Engine/RHI/ShaderStage.hpp"
#include "Engine/Config/EngineConfig.hpp"

class DomainShaderStage : public ShaderStage
{
public:
	ID3D11DomainShader* m_dx_domain_shader;

public:
	DomainShaderStage(RHIDevice* rhi_device, 
					  const char* shader_raw_source, 
					  const size_t shader_raw_source_size, 
					  const char* entry_point = DEFAULT_DOMAIN_SHADER_ENTRY_POINT,
					  const char* opt_filename = nullptr);

	virtual ~DomainShaderStage() override;

	bool is_valid() const { return m_dx_domain_shader != nullptr; }
};
