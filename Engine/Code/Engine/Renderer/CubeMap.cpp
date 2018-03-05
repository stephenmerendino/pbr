#include "Engine/Renderer/CubeMap.hpp"
#include "Engine/Renderer/camera.h"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/Shader.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/job.h"
#include "Engine/Engine.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void job_load_cubemap_face_image(Image** out_image, const std::string& filename)
{
	*out_image = new Image(filename, IMAGE_LOAD_MODE_FORCE_ALPHA);
}

void job_init_texture(CubeMap* cm, Image** loadedImages)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = loadedImages[0]->GetWidth();
	texDesc.Height = loadedImages[0]->GetHeight();
	texDesc.MipLevels = 0;
	texDesc.ArraySize = NUM_CUBE_FACES;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* dxCubeTexture;
	ID3D11ShaderResourceView* dxShaderResourceView;
	ID3D11RenderTargetView* dxRenderTargetView;

	// Create the cube texture
	HRESULT hr = g_theRenderer->m_device->m_dxDevice->CreateTexture2D(&texDesc, nullptr, &dxCubeTexture);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create DX11Texture2D for cube map");

	D3D11_TEXTURE2D_DESC mip_desc;
	dxCubeTexture->GetDesc(&mip_desc);
	UINT mip_levels = mip_desc.MipLevels;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	MemZero(&srvDesc);
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = mip_levels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	hr = g_theRenderer->m_device->m_dxDevice->CreateRenderTargetView(dxCubeTexture, nullptr, &dxRenderTargetView);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create shader resource view for cube map");

	// Create the shader resource view
	hr = g_theRenderer->m_device->m_dxDevice->CreateShaderResourceView(dxCubeTexture, &srvDesc, &dxShaderResourceView);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create shader resource view for cube map");

	// update subresource
	for(int cubeMapFaceIndex = 0; cubeMapFaceIndex < NUM_CUBE_FACES; cubeMapFaceIndex++)
	{
		Image* im = loadedImages[cubeMapFaceIndex];
		UINT sub_resource_id = D3D11CalcSubresource(0, cubeMapFaceIndex, mip_levels);

		g_theRenderer->m_device->m_immediateContext->m_dxDeviceContext->UpdateSubresource(dxCubeTexture,
																		 sub_resource_id,
																		 NULL,
																		 im->GetImageTexelBytes(),
																		 im->GetWidth() * 4,
																		 0);
	}

	// actually generate mips
	g_theRenderer->m_device->m_immediateContext->m_dxDeviceContext->GenerateMips(dxShaderResourceView);

	// Unload the images
	for(int imageIndex = 0; imageIndex < NUM_CUBE_FACES; ++imageIndex){
		SAFE_DELETE(loadedImages[imageIndex]);
	}

	free(loadedImages);

	cm->m_texture = dxCubeTexture;
	cm->m_srv = dxShaderResourceView;

	cm->m_resolution = texDesc.Width;

	cm->finished_loading();
}

CubeMap::CubeMap()
	:m_texture(nullptr)
	,m_srv(nullptr)
	,m_rtvs(nullptr)
	,m_dsvs(nullptr)
	,m_is_loaded(false)
	,m_listener_cb(nullptr)
	,m_listener_args(nullptr)
	,m_resolution(0)
	,m_uses_mips(false)
{
}

CubeMap::CubeMap(uint resolution, ImageFormat format, bool setup_mips)
	:m_texture(nullptr)
	,m_srv(nullptr)
	,m_rtvs(nullptr)
	,m_dsvs(nullptr)
	,m_is_loaded(false)
	,m_listener_cb(nullptr)
	,m_listener_args(nullptr)
	,m_resolution(resolution)
	,m_format(format)
	,m_uses_mips(setup_mips)
{
	create_base_resource();
	create_views();
}

CubeMap::~CubeMap()
{
	uint num_subresources = get_num_subresources();
	for(uint i = 0; i < num_subresources; ++i){
		if(nullptr != m_dsvs){
			DX_SAFE_RELEASE(m_dsvs[i]);
		}

		if(nullptr != m_rtvs){
			DX_SAFE_RELEASE(m_rtvs[i]);
		}
	}

	if(nullptr != m_dsvs){
		delete[] m_dsvs;
	}

	if(nullptr != m_rtvs){
		delete[] m_rtvs;
	}

	DX_SAFE_RELEASE(m_texture);
}

bool CubeMap::is_srv()
{
	return nullptr != m_srv;
}

bool CubeMap::is_rtv()
{
	return nullptr != m_rtvs;
}

bool CubeMap::is_dsv()
{
	return nullptr != m_dsvs;
}


void CubeMap::create_base_resource()
{
	D3D11_TEXTURE2D_DESC texDesc;
	MemZero(&texDesc);

	texDesc.Width = m_resolution;
	texDesc.Height = m_resolution;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = NUM_CUBE_FACES;
	texDesc.Format = DXGetImageFormat(m_format);
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	if(IMAGE_FORMAT_D24S8 == m_format){
		texDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	} else{
		texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}

	if(m_uses_mips && IMAGE_FORMAT_D24S8 != m_format){
		texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		texDesc.MipLevels = 0;
		texDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	// Create the cube texture
	HRESULT hr = g_theRenderer->m_device->m_dxDevice->CreateTexture2D(&texDesc, nullptr, &m_texture);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create DX11Texture2D for cube map");
}

void CubeMap::create_views()
{
	uint num_mip_levels = get_num_mip_levels();

	// Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = (IMAGE_FORMAT_D24S8 == m_format) ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGetImageFormat(m_format);
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = num_mip_levels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	HRESULT hr = g_theRenderer->m_device->m_dxDevice->CreateShaderResourceView(m_texture, &srvDesc, &m_srv);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create shader resource view for cube map");

	uint num_subresources = get_num_subresources();

	// dsv
	if(IMAGE_FORMAT_D24S8 == m_format){
		m_dsvs = new ID3D11DepthStencilView*[NUM_CUBE_FACES];
		for(uint face = 0; face < NUM_CUBE_FACES; ++face){
			D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
			memset(&dsv_desc, 0, sizeof(dsv_desc));
			dsv_desc.Flags = 0;
			dsv_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsv_desc.Texture2DArray.MipSlice = 0;
			dsv_desc.Texture2DArray.FirstArraySlice = face;
			dsv_desc.Texture2DArray.ArraySize = 1;

			hr = g_theRenderer->m_device->m_dxDevice->CreateDepthStencilView(m_texture, &dsv_desc, &m_dsvs[face]);
			ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create depth stencil view for cube map");
		}
	} 
	// rtv
	else{
		m_rtvs = new ID3D11RenderTargetView*[num_subresources];
		for(uint face = 0; face < NUM_CUBE_FACES; ++face){
			for(uint mip = 0; mip < num_mip_levels; ++mip){
				uint subresource_idx = get_subresource_index(face, mip);

				D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
				memset(&rtv_desc, 0, sizeof(rtv_desc));
				rtv_desc.Format = DXGetImageFormat(m_format);
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtv_desc.Texture2DArray.ArraySize = 1;
				rtv_desc.Texture2DArray.MipSlice = mip;
				rtv_desc.Texture2DArray.FirstArraySlice = face;

				hr = g_theRenderer->m_device->m_dxDevice->CreateRenderTargetView(m_texture, &rtv_desc, &m_rtvs[subresource_idx]);
				ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create render target view for cube map");
			}
		}
	}
}

void CubeMap::on_loaded(loaded_cb cb, void* args)
{
	if(m_is_loaded){
		cb(this, args);
		return;
	}

	m_listener_cb = cb;
	m_listener_args = args;
}

void CubeMap::finished_loading()
{
	m_is_loaded = true;
	if(m_listener_cb){
		m_listener_cb(this, m_listener_args);
		m_listener_cb = nullptr;
		m_listener_args = nullptr;
	}
}

bool CubeMap::load_from_seperate_images(RHIDevice* device, CubeMapImagePaths cubeMapImages)
{
	UNUSED(device);

	Image** loadedImages = (Image**)malloc(sizeof(Image*) * NUM_CUBE_FACES);

	job_load_cubemap_face_image(&loadedImages[CUBE_FACE_POS_X], std::string(cubeMapImages.posX));
	job_load_cubemap_face_image(&loadedImages[CUBE_FACE_NEG_X], std::string(cubeMapImages.negX));
	job_load_cubemap_face_image(&loadedImages[CUBE_FACE_POS_Y], std::string(cubeMapImages.posY));
	job_load_cubemap_face_image(&loadedImages[CUBE_FACE_NEG_Y], std::string(cubeMapImages.negY));
	job_load_cubemap_face_image(&loadedImages[CUBE_FACE_POS_Z], std::string(cubeMapImages.posZ));
	job_load_cubemap_face_image(&loadedImages[CUBE_FACE_NEG_Z], std::string(cubeMapImages.negZ));

	job_init_texture(this, loadedImages);

	return true;
}

bool CubeMap::load_from_seperate_images_async(RHIDevice* device, CubeMapImagePaths cubeMapImages)
{
	UNUSED(device);

	Image** loadedImages = (Image**)malloc(sizeof(Image*) * NUM_CUBE_FACES);

	Job* pos_x_job = job_create(JOB_TYPE_GENERIC, job_load_cubemap_face_image, &loadedImages[CUBE_FACE_POS_X], std::string(cubeMapImages.posX));
	Job* neg_x_job = job_create(JOB_TYPE_GENERIC, job_load_cubemap_face_image, &loadedImages[CUBE_FACE_NEG_X], std::string(cubeMapImages.negX));
	Job* pos_y_job = job_create(JOB_TYPE_GENERIC, job_load_cubemap_face_image, &loadedImages[CUBE_FACE_POS_Y], std::string(cubeMapImages.posY));
	Job* neg_y_job = job_create(JOB_TYPE_GENERIC, job_load_cubemap_face_image, &loadedImages[CUBE_FACE_NEG_Y], std::string(cubeMapImages.negY));
	Job* pos_z_job = job_create(JOB_TYPE_GENERIC, job_load_cubemap_face_image, &loadedImages[CUBE_FACE_POS_Z], std::string(cubeMapImages.posZ));
	Job* neg_z_job = job_create(JOB_TYPE_GENERIC, job_load_cubemap_face_image, &loadedImages[CUBE_FACE_NEG_Z], std::string(cubeMapImages.negZ));

	Job* init_job = job_create(JOB_TYPE_RENDERING, job_init_texture, this, loadedImages);

	init_job->depends_on(pos_x_job);
	init_job->depends_on(neg_x_job);
	init_job->depends_on(pos_y_job);
	init_job->depends_on(neg_y_job);
	init_job->depends_on(pos_z_job);
	init_job->depends_on(neg_z_job);

	job_dispatch_and_release(pos_x_job, neg_x_job, pos_y_job, neg_y_job, pos_z_job, neg_z_job, init_job);

	return true;
}

bool CubeMap::save_to_seperate_images(const char* filename)
{
	UNUSED(filename);
	//g_theRenderer->m_deviceContext->save_texture2d_to_file(m_texture, Stringf("%s_pos_x.png", filename).c_str(), CUBE_FACE_POS_X);
	//g_theRenderer->m_deviceContext->save_texture2d_to_file(m_texture, Stringf("%s_neg_x.png", filename).c_str(), CUBE_FACE_NEG_X);
	//g_theRenderer->m_deviceContext->save_texture2d_to_file(m_texture, Stringf("%s_pos_y.png", filename).c_str(), CUBE_FACE_POS_Y);
	//g_theRenderer->m_deviceContext->save_texture2d_to_file(m_texture, Stringf("%s_neg_y.png", filename).c_str(), CUBE_FACE_NEG_Y);
	//g_theRenderer->m_deviceContext->save_texture2d_to_file(m_texture, Stringf("%s_pos_z.png", filename).c_str(), CUBE_FACE_POS_Z);
	//g_theRenderer->m_deviceContext->save_texture2d_to_file(m_texture, Stringf("%s_neg_z.png", filename).c_str(), CUBE_FACE_NEG_Z);

	return true;
}

void CubeMap::render_setup_face(uint face, const Vector3& view_center, uint mip_level)
{
	switch(face){
		case CUBE_FACE_POS_X: render_setup_pos_x(view_center, mip_level); break;
		case CUBE_FACE_NEG_X: render_setup_neg_x(view_center, mip_level); break;

		case CUBE_FACE_POS_Y: render_setup_pos_y(view_center, mip_level); break;
		case CUBE_FACE_NEG_Y: render_setup_neg_y(view_center, mip_level); break;

		case CUBE_FACE_POS_Z: render_setup_pos_z(view_center, mip_level); break;
		case CUBE_FACE_NEG_Z: render_setup_neg_z(view_center, mip_level); break;

		default: break;
	}
}

void CubeMap::render_setup_pos_x(const Vector3& view_center, uint mip_level)
{
	Camera c = configure_cubemap_camera();

	g_theRenderer->EnableDepth(false, false);
	uint mip_res = calculate_mip_res(mip_level);
	g_theRenderer->SetViewport(0, 0, mip_res, mip_res);

	c.look_at(Vector3::X_AXIS);
	c.m_transform.set_world_position(view_center);

	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(c.get_view());
	g_theRenderer->SetProjection(c.get_projection());
}

void CubeMap::render_setup_neg_x(const Vector3& view_center, uint mip_level)
{
	Camera c = configure_cubemap_camera();

	g_theRenderer->EnableDepth(false, false);
	uint mip_res = calculate_mip_res(mip_level);
	g_theRenderer->SetViewport(0, 0, mip_res, mip_res);

	c.look_at(-Vector3::X_AXIS);
	c.m_transform.set_world_position(view_center);

	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(c.get_view());
	g_theRenderer->SetProjection(c.get_projection());
}

void CubeMap::render_setup_pos_y(const Vector3& view_center, uint mip_level)
{
	Camera c = configure_cubemap_camera();

	g_theRenderer->EnableDepth(false, false);
	uint mip_res = calculate_mip_res(mip_level);
	g_theRenderer->SetViewport(0, 0, mip_res, mip_res);

	c.look_at(Vector3::Y_AXIS, -Vector3::Z_AXIS);
	c.m_transform.set_world_position(view_center);

	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(c.get_view());
	g_theRenderer->SetProjection(c.get_projection());
}

void CubeMap::render_setup_neg_y(const Vector3& view_center, uint mip_level)
{
	Camera c = configure_cubemap_camera();

	g_theRenderer->EnableDepth(false, false);
	uint mip_res = calculate_mip_res(mip_level);
	g_theRenderer->SetViewport(0, 0, mip_res, mip_res);

	c.look_at(-Vector3::Y_AXIS, Vector3::Z_AXIS);
	c.m_transform.set_world_position(view_center);

	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(c.get_view());
	g_theRenderer->SetProjection(c.get_projection());
}

void CubeMap::render_setup_pos_z(const Vector3& view_center, uint mip_level)
{
	Camera c = configure_cubemap_camera();

	g_theRenderer->EnableDepth(false, false);
	uint mip_res = calculate_mip_res(mip_level);
	g_theRenderer->SetViewport(0, 0, mip_res, mip_res);

	c.look_at(Vector3::Z_AXIS);
	c.m_transform.set_world_position(view_center);

	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(c.get_view());
	g_theRenderer->SetProjection(c.get_projection());
}

void CubeMap::render_setup_neg_z(const Vector3& view_center, uint mip_level)
{
	Camera c = configure_cubemap_camera();

	g_theRenderer->EnableDepth(false, false);
	uint mip_res = calculate_mip_res(mip_level);
	g_theRenderer->SetViewport(0, 0, mip_res, mip_res);

	c.look_at(-Vector3::Z_AXIS);
	c.m_transform.set_world_position(view_center);

	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(c.get_view());
	g_theRenderer->SetProjection(c.get_projection());
}

Camera CubeMap::configure_cubemap_camera()
{
	Camera c;
	c.m_fov = 90.0f;
	c.m_nz = 0.1f;
	c.m_fz = 100.0f;
	c.m_aspect = 1.0f;
	return c;
}

uint CubeMap::calculate_mip_res(uint mip_level)
{
	return m_resolution / (uint)pow(2, mip_level);
}

uint CubeMap::get_num_mip_levels() const
{
	if(!m_uses_mips){
		return 1;
	}

	D3D11_TEXTURE2D_DESC mip_desc;
	m_texture->GetDesc(&mip_desc);
	uint num_mips = mip_desc.MipLevels;
	return num_mips;
}

uint CubeMap::get_num_subresources() const
{
	return get_num_mip_levels() * NUM_CUBE_FACES;
}

uint CubeMap::get_subresource_index(uint face_index, uint mip_level) const
{
	return (mip_level * NUM_CUBE_FACES) + face_index;
}