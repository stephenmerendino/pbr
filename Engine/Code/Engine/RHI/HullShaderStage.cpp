#include "Engine/RHI/HullShaderStage.hpp"
#include "Engine/RHI/RHIDevice.hpp"

HullShaderStage::HullShaderStage(RHIDevice* rhi_device, const char* shader_raw_source, const size_t shader_raw_source_size, const char* entry_point, const char* opt_filename)
	:ShaderStage(shader_raw_source, shader_raw_source_size, entry_point, DX_HULL_SHADER_TARGET, opt_filename)
	,m_dx_hull_shader(nullptr)
{
	rhi_device->m_dxDevice->CreateHullShader(m_byte_code->GetBufferPointer(), m_byte_code->GetBufferSize(), nullptr, &m_dx_hull_shader);
}

HullShaderStage::~HullShaderStage()
{
	DX_SAFE_RELEASE(m_dx_hull_shader);
}