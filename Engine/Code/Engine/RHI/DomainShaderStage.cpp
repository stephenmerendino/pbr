#include "Engine/RHI/DomainShaderStage.hpp"
#include "Engine/RHI/RHIDevice.hpp"

DomainShaderStage::DomainShaderStage(RHIDevice* rhi_device, const char* shader_raw_source, const size_t shader_raw_source_size, const char* entry_point, const char* opt_filename)
	:ShaderStage(shader_raw_source, shader_raw_source_size, entry_point, DX_DOMAIN_SHADER_TARGET, opt_filename)
	,m_dx_domain_shader(nullptr)
{
	rhi_device->m_dxDevice->CreateDomainShader(m_byte_code->GetBufferPointer(), m_byte_code->GetBufferSize(), nullptr, &m_dx_domain_shader);
}

DomainShaderStage::~DomainShaderStage()
{
	DX_SAFE_RELEASE(m_dx_domain_shader);
}
