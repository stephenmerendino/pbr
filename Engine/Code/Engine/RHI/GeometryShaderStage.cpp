#include "Engine/RHI/GeometryShaderStage.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Renderer/Vertex3.hpp"

GeometryShaderStage::GeometryShaderStage(RHIDevice* rhi_device, const char* shader_raw_source, const size_t shader_raw_source_size, const char* entry_point, const char* opt_filename)
	:ShaderStage(shader_raw_source, shader_raw_source_size, entry_point, DX_GEOMETRY_SHADER_TARGET, opt_filename)
	,m_dx_geometry_shader(nullptr)
{
	rhi_device->m_dxDevice->CreateGeometryShader(m_byte_code->GetBufferPointer(), m_byte_code->GetBufferSize(), nullptr, &m_dx_geometry_shader);
}

GeometryShaderStage::~GeometryShaderStage()
{
	DX_SAFE_RELEASE(m_dx_geometry_shader);
}