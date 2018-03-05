#pragma once

#include "Engine/RHI/ShaderStage.hpp"
#include "Engine/Config/EngineConfig.hpp"

class GeometryShaderStage : public ShaderStage
{
public:
	ID3D11GeometryShader*   m_dx_geometry_shader;

public:
	GeometryShaderStage(RHIDevice* rhi_device, 
					  const char* shader_raw_source, 
					  const size_t shader_raw_source_size, 
					  const char* entry_point = DEFAULT_GEOMETRY_SHADER_ENTRY_POINT,
		              const char* opt_filename = nullptr);

	virtual ~GeometryShaderStage() override;

	bool is_valid() const { return m_dx_geometry_shader != nullptr; }
};