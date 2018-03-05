#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHITexture3D.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/StructuredBuffer.hpp"
#include "Engine/RHI/ComputeJob.hpp"
#include "Engine/RHI/ComputeShader.hpp"
#include "Engine/RHI/ComputeShaderStage.hpp"
#include "Engine/Renderer/Vertex3.hpp"
#include "Engine/Renderer/CubeMap.hpp"
#include "Engine/Profile/gpu_profile.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"

RHIDeviceContext::RHIDeviceContext(RHIDevice* owner, ID3D11DeviceContext* dx11DeviceContext)
	:m_device(owner)
	,m_dxDeviceContext(dx11DeviceContext)
{
}

RHIDeviceContext::~RHIDeviceContext()
{
	m_dxDeviceContext->Flush();
	DX_SAFE_RELEASE(m_dxDeviceContext);
}

void RHIDeviceContext::ClearState()
{
	if(!m_dxDeviceContext){
		return;
	}

	m_dxDeviceContext->ClearState();
}

void RHIDeviceContext::Flush()
{
	if(!m_dxDeviceContext){
		return;
	}

	m_dxDeviceContext->Flush();
}

void RHIDeviceContext::SetColorTarget(RHITextureBase* colorTarget, RHITextureBase* depthStencilTarget)
{
	if(colorTarget->IsRenderTarget()){
		m_dxDeviceContext->OMSetRenderTargets(1, 
											  &colorTarget->m_dxRenderTargetView, 
											  depthStencilTarget != nullptr ? depthStencilTarget->m_dxDepthStencilView : nullptr);
	}
}

void RHIDeviceContext::SetColorTarget(CubeMap* color_target, CubeMap* depth_target, uint face_index, uint mip_level)
{
	uint rtv_idx = color_target->get_subresource_index(face_index, mip_level);
	ID3D11RenderTargetView* rtv = color_target->m_rtvs[rtv_idx];

	uint dsv_idx = depth_target->get_subresource_index(face_index, 0);
	ID3D11DepthStencilView* dsv = depth_target->m_dsvs[dsv_idx];

	m_dxDeviceContext->OMSetRenderTargets(1, &rtv, dsv);
}

void RHIDeviceContext::SetColorTargets(RHITextureBase** colorTargets, int numColorTargets, RHITextureBase* depthStencilTarget)
{
	ID3D11RenderTargetView** dxRenderTargets = new ID3D11RenderTargetView*[numColorTargets];

	for(int targetIndex = 0; targetIndex < numColorTargets; ++targetIndex){
		dxRenderTargets[targetIndex] = colorTargets[targetIndex] != nullptr ? colorTargets[targetIndex]->m_dxRenderTargetView : nullptr;
	}

	m_dxDeviceContext->OMSetRenderTargets(numColorTargets, 
										  dxRenderTargets, 
										  depthStencilTarget != nullptr ? depthStencilTarget->m_dxDepthStencilView : nullptr);

	delete[] dxRenderTargets;
}

void RHIDeviceContext::ClearColorTarget(RHITextureBase* colorTarget, const Rgba& clearColor)
{
	if(!colorTarget){
		return;
	}

	if(!m_dxDeviceContext){
		return;
	}

	float colorComponentsAsFloats[4];
	clearColor.GetAsFloats(colorComponentsAsFloats);
	m_dxDeviceContext->ClearRenderTargetView(colorTarget->m_dxRenderTargetView, colorComponentsAsFloats);
}

void RHIDeviceContext::ClearDepthTarget(RHITextureBase* depthTarget, float depth, uint8_t stencil)
{
    if(nullptr != depthTarget){
    	ASSERT_OR_DIE(depthTarget->IsDepthStencilView(), "RHITexture2D must be a valid depth target to clear it");
    	m_dxDeviceContext->ClearDepthStencilView(depthTarget->m_dxDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }
}

void RHIDeviceContext::ClearColorTarget(CubeMap* colorTarget, uint face, const Rgba& clearColor)
{
	uint num_mips = colorTarget->get_num_mip_levels();
	for(uint mip = 0; mip < num_mips; ++mip){
		uint subresource_idx = colorTarget->get_subresource_index(face, mip);
		ID3D11RenderTargetView* rtv = colorTarget->m_rtvs[subresource_idx];

		float colorComponentsAsFloats[4];
		clearColor.GetAsFloats(colorComponentsAsFloats);
		m_dxDeviceContext->ClearRenderTargetView(rtv, colorComponentsAsFloats);
	}
}

void RHIDeviceContext::ClearDepthTarget(CubeMap* depthTarget, uint face, float depth, uint8_t stencil)
{
    if(nullptr != depthTarget->m_dsvs && nullptr != depthTarget->m_dsvs[face]){
    	ASSERT_OR_DIE(depthTarget->is_dsv(), "CubeMap must be a valid depth target to clear it");
    	m_dxDeviceContext->ClearDepthStencilView(depthTarget->m_dsvs[face], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }
}

void RHIDeviceContext::SetViewport(int x, int y, int width, int height)
{
	// Also, set which region of the screen we're rendering to, in this case, all of it 
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.TopLeftX = (FLOAT)x;
	viewport.TopLeftY = (FLOAT)y;
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = 0.0f;        // must be between 0 and 1 (defualt is 0);
	viewport.MaxDepth = 1.0f;        // must be between 0 and 1 (default is 1)

	m_dxDeviceContext->RSSetViewports(1, &viewport);
}

void RHIDeviceContext::SetShaderProgram(ShaderProgram* shaderProgram)
{
	m_dxDeviceContext->VSSetShader(shaderProgram->m_vertex_stage->m_dx_vertex_shader, nullptr, 0);

    if(nullptr != shaderProgram->m_hull_stage){
        m_dxDeviceContext->HSSetShader(shaderProgram->m_hull_stage->m_dx_hull_shader, nullptr, 0);
    }else{
        m_dxDeviceContext->HSSetShader(nullptr, nullptr, 0);
    }

    if(nullptr != shaderProgram->m_domain_stage){
        m_dxDeviceContext->DSSetShader(shaderProgram->m_domain_stage->m_dx_domain_shader, nullptr, 0);
    }else{
        m_dxDeviceContext->DSSetShader(nullptr, nullptr, 0);
    }

    if(nullptr != shaderProgram->m_geometry_stage){
        m_dxDeviceContext->GSSetShader(shaderProgram->m_geometry_stage->m_dx_geometry_shader, nullptr, 0);
    }else{
        m_dxDeviceContext->GSSetShader(nullptr, nullptr, 0);
    }

	m_dxDeviceContext->PSSetShader(shaderProgram->m_fragment_stage->m_dx_pixel_shader, nullptr, 0);
	m_dxDeviceContext->IASetInputLayout(shaderProgram->m_vertex_stage->m_dx_input_layout);
}

void RHIDeviceContext::SetTexture(const unsigned int index, CubeMap* texture)
{
	ID3D11ShaderResourceView* srv[] = { nullptr };

	if(texture){
		srv[0] = texture->m_srv;
	}

	m_dxDeviceContext->VSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->HSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->DSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->GSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->PSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->CSSetShaderResources(index, 1, srv);
}

void RHIDeviceContext::SetTexture(const unsigned int index, RHITextureBase* texture)
{
	ID3D11ShaderResourceView* srv[] = { nullptr };

	if(texture){
		srv[0] = texture->m_dxShaderResourceView;
	}

	m_dxDeviceContext->VSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->HSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->DSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->GSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->PSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->CSSetShaderResources(index, 1, srv);
}

void RHIDeviceContext::SetSampler(const unsigned int index, Sampler* sampler)
{
	ID3D11SamplerState* dx_sampler[] = { nullptr };

	if(sampler){
		dx_sampler[0] = sampler->m_dxSampler;
	}

	m_dxDeviceContext->VSSetSamplers(index, 1, dx_sampler);
	m_dxDeviceContext->HSSetSamplers(index, 1, dx_sampler);
	m_dxDeviceContext->DSSetSamplers(index, 1, dx_sampler);
	m_dxDeviceContext->GSSetSamplers(index, 1, dx_sampler);
	m_dxDeviceContext->PSSetSamplers(index, 1, dx_sampler);
	m_dxDeviceContext->CSSetSamplers(index, 1, dx_sampler);
}

void RHIDeviceContext::SetConstantBuffer(const unsigned int index, ConstantBuffer* constantBuffer)
{
	ID3D11Buffer* dx_constant_buffer[] = { nullptr };

	if(constantBuffer){
		dx_constant_buffer[0] = constantBuffer->m_dxBuffer;
	}

    // vertex, geometry, pixel
	m_dxDeviceContext->VSSetConstantBuffers(index, 1, dx_constant_buffer);
	m_dxDeviceContext->HSSetConstantBuffers(index, 1, dx_constant_buffer);
	m_dxDeviceContext->DSSetConstantBuffers(index, 1, dx_constant_buffer);
	m_dxDeviceContext->GSSetConstantBuffers(index, 1, dx_constant_buffer);
	m_dxDeviceContext->PSSetConstantBuffers(index, 1, dx_constant_buffer);

    // compute
	m_dxDeviceContext->CSSetConstantBuffers(index, 1, dx_constant_buffer);
}

void RHIDeviceContext::SetStructuredBuffer(const unsigned int index, StructuredBuffer* structuredBuffer)
{
	ID3D11ShaderResourceView* dx_structured_buffer_srv[] = { nullptr };

	if(structuredBuffer){
		dx_structured_buffer_srv[0] = structuredBuffer->m_dx_srv;
	}

    // vertex, geometry, pixel
	m_dxDeviceContext->VSSetShaderResources(index, 1, dx_structured_buffer_srv);
	m_dxDeviceContext->HSSetShaderResources(index, 1, dx_structured_buffer_srv);
	m_dxDeviceContext->DSSetShaderResources(index, 1, dx_structured_buffer_srv);
	m_dxDeviceContext->GSSetShaderResources(index, 1, dx_structured_buffer_srv);
	m_dxDeviceContext->PSSetShaderResources(index, 1, dx_structured_buffer_srv);

    // compute
	m_dxDeviceContext->PSSetShaderResources(index, 1, dx_structured_buffer_srv);
}

void RHIDeviceContext::SetRasterState(RasterState* rasterState)
{
	m_dxDeviceContext->RSSetState(rasterState->m_dxRasterState);
}

void RHIDeviceContext::SetBlendState(BlendState* blendState)
{
	float constant[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_dxDeviceContext->OMSetBlendState(blendState->m_dxBlendState, constant, 0xFFFFFFFF);
}

void RHIDeviceContext::SetDepthStencilState(DepthStencilState* depthStencilState)
{
	m_dxDeviceContext->OMSetDepthStencilState(depthStencilState->m_dxDepthStencilState, 0U);
}

void RHIDeviceContext::Draw(PrimitiveType topology, VertexBuffer* vbo, const unsigned int vertexCount, const unsigned int startIndex)
{
	D3D11_PRIMITIVE_TOPOLOGY dxTopology = DXGetTopology(topology);
	m_dxDeviceContext->IASetPrimitiveTopology(dxTopology);

	unsigned int stride = sizeof(Vertex3);
	unsigned int offsets = 0;

	m_dxDeviceContext->IASetVertexBuffers(0, 1, &vbo->m_dxBuffer, &stride, &offsets);
	m_dxDeviceContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0U);

	m_dxDeviceContext->Draw(vertexCount, startIndex);
}

void RHIDeviceContext::DrawIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo, const unsigned int indexCount, const unsigned int startIndex)
{
	D3D11_PRIMITIVE_TOPOLOGY dxTopology = DXGetTopology(topology);
	m_dxDeviceContext->IASetPrimitiveTopology(dxTopology);

	unsigned int stride = sizeof(Vertex3);
	unsigned int offsets = 0;

	m_dxDeviceContext->IASetVertexBuffers(0, 1, &vbo->m_dxBuffer, &stride, &offsets);
	m_dxDeviceContext->IASetIndexBuffer(ibo->m_dxBuffer, DXGI_FORMAT_R32_UINT, 0U);

	m_dxDeviceContext->DrawIndexed(indexCount, startIndex, 0);
}

void RHIDeviceContext::DispatchComputeJob(ComputeJob* compute_job)
{
    // set shader
    m_dxDeviceContext->CSSetShader(compute_job->m_compute_shader->m_compute_stage->m_dx_compute_shader, nullptr, 0);

    // set texture
    //#TODO: don't hardcode texture index
    m_dxDeviceContext->CSSetUnorderedAccessViews(0, 1, &compute_job->m_out_texture->m_dxUnorderedAccessView, nullptr);

    // dispatch w, h, d
    gpu_query_begin(this);
    m_dxDeviceContext->Dispatch(compute_job->m_grid_width, compute_job->m_grid_height, compute_job->m_grid_depth);
    gpu_query_end(this);

    // unbind the uav automatically so it can be used in other calls
	ID3D11UnorderedAccessView* uav[] = { nullptr };
    m_dxDeviceContext->CSSetUnorderedAccessViews(0, 1, uav, nullptr);
}

RHITexture2D* RHIDeviceContext::create_staging_texture2d_from_existing_texture2d(RHITexture2D* source)
{
	D3D11_TEXTURE2D_DESC src_desc;
	source->m_dxTexture2D->GetDesc(&src_desc);

	ASSERT_OR_DIE(src_desc.SampleDesc.Count == 1, "MSAA staging textures not supported");

	src_desc.BindFlags = 0;
	src_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	src_desc.Usage = D3D11_USAGE_STAGING;

	ID3D11Texture2D* dx_texture;
	HRESULT hr = m_device->m_dxDevice->CreateTexture2D(&src_desc, 0, &dx_texture);
	if(FAILED(hr)){
		return nullptr;
	}

	RHITexture2D* staging_texture = new RHITexture2D(dx_texture, nullptr);
	return staging_texture;
}

bool RHIDeviceContext::save_texture2d_to_file(RHITexture2D* texture, const char* filename, uint subresource_index)
{
	if((nullptr == texture) || (nullptr == filename)){
		return false;
	}

	// create a staging texture
	RHITexture2D* staging_texture = create_staging_texture2d_from_existing_texture2d(texture);

	// copy from source texture into staging texture
	m_dxDeviceContext->CopyResource(staging_texture->m_dxTexture2D, texture->m_dxTexture2D);

	// map the staging texture
	D3D11_MAPPED_SUBRESOURCE resource;
	if(SUCCEEDED(m_dxDeviceContext->Map(staging_texture->m_dxTexture2D, subresource_index, D3D11_MAP_READ, 0U, &resource))){
		// save out to file
		Image::save_to_png(texture->m_width, texture->m_height, (unsigned char*)resource.pData, filename);
	}

	SAFE_DELETE(staging_texture);

	return true;
}

bool RHIDeviceContext::save_texture2d_to_binary_file(RHITexture2D* texture, const char* filename, uint subresource_index)
{
	if((nullptr == texture) || (nullptr == filename)){
		return false;
	}

	// create a staging texture
	RHITexture2D* staging_texture = create_staging_texture2d_from_existing_texture2d(texture);

	// copy from source texture into staging texture
	m_dxDeviceContext->CopyResource(staging_texture->m_dxTexture2D, texture->m_dxTexture2D);

	// map the staging texture
	D3D11_MAPPED_SUBRESOURCE resource;
	if(SUCCEEDED(m_dxDeviceContext->Map(staging_texture->m_dxTexture2D, subresource_index, D3D11_MAP_READ, 0U, &resource))){

		uint height = texture->GetHeight();
		uint stride = texture->get_stride();

		size_t total_bytes = height * stride;
		unsigned char* raw_image_bytes = (unsigned char*)malloc(total_bytes + 1);
		memset(raw_image_bytes, 0, total_bytes + 1);

		memcpy(raw_image_bytes, resource.pData, total_bytes);

		raw_image_bytes[total_bytes] = '\0';

		//.unsigned char* raw_image_bytes = (unsigned char*)resource.pData;

		SaveBufferToBinaryFile(raw_image_bytes, total_bytes, filename);

		free(raw_image_bytes);
	}

	SAFE_DELETE(staging_texture);

	return true;
}