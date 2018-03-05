#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include <stdint.h>

class RHIDevice;
class RHITextureBase;
class RHITexture2D;
class ShaderProgram;
class Sampler;
class VertexBuffer;
class IndexBuffer;
class RasterState;
class BlendState;
class DepthStencilState;
class ConstantBuffer;
class StructuredBuffer;
class ComputeJob;
class CubeMap;
struct ID3D11DeviceContext;

class RHIDeviceContext
{
public:
	RHIDevice*				m_device;
	ID3D11DeviceContext*	m_dxDeviceContext;

public:
	RHIDeviceContext(RHIDevice* owner, ID3D11DeviceContext* dx11DeviceContext);
	~RHIDeviceContext();

	void ClearState();
	void Flush();

	void SetColorTarget(RHITextureBase* colorTarget, RHITextureBase* depthStencilTarget = nullptr);
	void SetColorTarget(CubeMap* color_target, CubeMap* depth_target, uint face_index, uint mip_level);
	void SetColorTargets(RHITextureBase** colorTargets, int numColorTargets, RHITextureBase* depthStencilTarget = nullptr);
	void ClearColorTarget(RHITextureBase* colorTarget, const Rgba& clearColor);
	void ClearDepthTarget(RHITextureBase* depthTarget, float depth = 1.0f, uint8_t stencil = 0);
	void ClearColorTarget(CubeMap* colorTarget, uint face, const Rgba& clearColor);
	void ClearDepthTarget(CubeMap* depthTarget, uint face, float depth = 1.0f, uint8_t stencil = 0);

	void SetViewport(int x, int y, int width, int height);

	void SetShaderProgram(ShaderProgram* shaderProgram);
	void SetTexture(const unsigned int index, RHITextureBase* texture);
	void SetTexture(const unsigned int index, CubeMap* texture);
	void SetSampler(const unsigned int index, Sampler* sampler);
	void SetConstantBuffer(const unsigned int index, ConstantBuffer* constantBuffer);
	void SetStructuredBuffer(const unsigned int index, StructuredBuffer* structuredBuffer);

	void SetRasterState(RasterState* rasterState);
	void SetBlendState(BlendState* blendState);
	void SetDepthStencilState(DepthStencilState* depthStencilState);

	void Draw(PrimitiveType topology, VertexBuffer* vbo, const unsigned int vertexCount, const unsigned int startIndex = 0);
	void DrawIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo, const unsigned int indexCount, const unsigned int startIndex = 0);

    void DispatchComputeJob(ComputeJob* compute_job);

	RHITexture2D* create_staging_texture2d_from_existing_texture2d(RHITexture2D* source);
	bool save_texture2d_to_file(RHITexture2D* texture, const char* filename, uint subresource_index = 0);
	bool save_texture2d_to_binary_file(RHITexture2D* texture, const char* filename, uint subresource_index = 0);
};