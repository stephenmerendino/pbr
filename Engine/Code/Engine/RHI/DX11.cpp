#include "Engine/RHI/DX11.hpp"

D3D11_USAGE DXGetBufferUsage(const BufferUsage usage) 
{
	switch (usage){
		case BUFFERUSAGE_GPU:		return D3D11_USAGE_DEFAULT;
		case BUFFERUSAGE_DYNAMIC:	return D3D11_USAGE_DYNAMIC;
		case BUFFERUSAGE_STAGING:	return D3D11_USAGE_STAGING;

		default:
		case BUFFERUSAGE_STATIC:	return D3D11_USAGE_IMMUTABLE;
	}
};

D3D11_PRIMITIVE_TOPOLOGY DXGetTopology(const PrimitiveType type) 
{
	switch (type){
		case PRIMITIVE_NONE:		return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		case PRIMITIVE_POINTS:		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PRIMITIVE_LINES:		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case PRIMITIVE_TRIANGLES_TESSELATED: return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

		default:
		case PRIMITIVE_TRIANGLES:	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
};

D3D11_CULL_MODE DXGetCullMode(CullMode mode)
{
	switch(mode){
		case CULL_NONE: return D3D11_CULL_NONE;
		case CULL_FRONT: return D3D11_CULL_FRONT;

		default:
		case CULL_BACK: return D3D11_CULL_BACK;
	}
}

D3D11_FILL_MODE DXGetFillMode(FillMode mode)
{
	switch(mode){
		case FILL_WIREFRAME: return D3D11_FILL_WIREFRAME;

		default:
		case FILL_SOLID: return D3D11_FILL_SOLID;
	}
}

D3D11_COMPARISON_FUNC DXGetDepthComparisonFunc(DepthTest test)
{
	switch(test){
		case DEPTH_TEST_COMPARISON_NEVER:				return D3D11_COMPARISON_NEVER; break;
		case DEPTH_TEST_COMPARISON_EQUAL:				return D3D11_COMPARISON_EQUAL; break;
		case DEPTH_TEST_COMPARISON_LESS_EQUAL: 			return D3D11_COMPARISON_LESS_EQUAL; break;
		case DEPTH_TEST_COMPARISON_GREATER:				return D3D11_COMPARISON_GREATER; break;
		case DEPTH_TEST_COMPARISON_NOT_EQUAL:			return D3D11_COMPARISON_NOT_EQUAL; break;
		case DEPTH_TEST_COMPARISON_GREATER_EQUAL:		return D3D11_COMPARISON_GREATER_EQUAL; break;
		case DEPTH_TEST_COMPARISON_ALWAYS:				return D3D11_COMPARISON_ALWAYS; break;

		default:
		case DEPTH_TEST_COMPARISON_LESS:				return D3D11_COMPARISON_LESS; break;
	}
}

D3D11_COMPARISON_FUNC DXGetSamplerComparisonFunc(SamplerComparisonFunc func)
{
	switch(func){
		case COMPARISON_NEVER:				return D3D11_COMPARISON_NEVER; break;
		case COMPARISON_EQUAL:				return D3D11_COMPARISON_EQUAL; break;
		case COMPARISON_LESS_EQUAL: 		return D3D11_COMPARISON_LESS_EQUAL; break;
		case COMPARISON_GREATER:			return D3D11_COMPARISON_GREATER; break;
		case COMPARISON_NOT_EQUAL:			return D3D11_COMPARISON_NOT_EQUAL; break;
		case COMPARISON_GREATER_EQUAL:		return D3D11_COMPARISON_GREATER_EQUAL; break;
		case COMPARISON_ALWAYS:				return D3D11_COMPARISON_ALWAYS; break;

		default:
		case COMPARISON_LESS:				return D3D11_COMPARISON_LESS; break;
	}
}

DXGI_FORMAT DXGetImageFormat(ImageFormat format)
{
	switch(format) {
		case IMAGE_FORMAT_RGBA8:
		{
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		} break;

		case IMAGE_FORMAT_RGBA32:
		{
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		} break;
		
		case IMAGE_FORMAT_D24S8:
		{
			//dx_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			return DXGI_FORMAT_R24G8_TYPELESS;
		} break;

		case IMAGE_FORMAT_R16:
		{
			return DXGI_FORMAT_R16_UNORM;
		} break;

		case IMAGE_FORMAT_R16G16:
		{
			return DXGI_FORMAT_R16G16_UNORM;
		} break;

		case IMAGE_FORMAT_R32G32:
		{
			return DXGI_FORMAT_R32G32_FLOAT;
		} break;

		case IMAGE_FORMAT_R32G32B32:
		{
			return DXGI_FORMAT_R32G32B32_FLOAT;
		} break;

		case IMAGE_FORMAT_R16G16B16A16:
		{
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		} break;

		default: 
		{
			return DXGI_FORMAT_UNKNOWN;
		}
	};
}