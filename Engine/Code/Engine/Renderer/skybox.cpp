#include "Engine/Renderer/skybox.h"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/Shader.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Engine.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

SkyBox::SkyBox()
	:m_display_texture(nullptr)
	,m_diffuse_integrated_target(nullptr)
	,m_diffuse_depth_target(nullptr)
	,m_spec_integrated_target(nullptr)
	,m_spec_depth_targets(nullptr)
	,m_spec_mip_buffer(nullptr)
{
	MemZero(&m_spec_mip_data);
	m_display_texture = new CubeMap();
}

SkyBox::~SkyBox()
{
	delete m_display_texture;
	delete m_diffuse_integrated_target;
	delete m_diffuse_depth_target;

	for(uint i = 0; i < m_spec_integrated_target->get_num_mip_levels(); i++){
		delete m_spec_depth_targets[i];
	}
	delete[] m_spec_depth_targets;

	delete m_spec_integrated_target;
}

bool SkyBox::load_from_face_images(RHIDevice* device, CubeMapImagePaths cubeMapImages)
{
	m_display_texture->load_from_seperate_images(device, cubeMapImages);
	return true;
}

void SkyBox::bake_lighting(uint diffuse_res, uint spec_res)
{
	integrate_diffuse(diffuse_res);
	integrate_spec(spec_res);
}

void SkyBox::integrate_diffuse(uint diffuse_res)
{
	if(nullptr == m_diffuse_integrated_target){
		m_diffuse_integrated_target = new CubeMap(diffuse_res, IMAGE_FORMAT_RGBA8, false);
	}
	
	if(nullptr == m_diffuse_depth_target){
		m_diffuse_depth_target = new CubeMap(diffuse_res, IMAGE_FORMAT_D24S8, false);
	}

	// bind pano_to_cubemap_shader 
	Shader* ld_diffuse_shader = Shader::find_or_create("Data/Shaders/ld_diffuse.shader");
	ASSERT_OR_DIE(nullptr != ld_diffuse_shader, "Failed to load ld diffuse shader");

	g_theRenderer->SetShader(ld_diffuse_shader);

	// bind environment map as input texture
	g_theRenderer->SetCubeTexture(0, m_display_texture);

	// render a quad for each face of the cube
	m_diffuse_integrated_target->render_setup_pos_x();
	g_theRenderer->SetColorTarget(m_diffuse_integrated_target, m_diffuse_depth_target, CUBE_FACE_POS_X, 0);
	g_theRenderer->DrawQuad3d(Vector3::X_AXIS, -Vector3::Z_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);

	m_diffuse_integrated_target->render_setup_neg_x();
	g_theRenderer->SetColorTarget(m_diffuse_integrated_target, m_diffuse_depth_target, CUBE_FACE_NEG_X, 0);
	g_theRenderer->DrawQuad3d(-Vector3::X_AXIS, Vector3::Z_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);

	m_diffuse_integrated_target->render_setup_pos_y();
	g_theRenderer->SetColorTarget(m_diffuse_integrated_target, m_diffuse_depth_target, CUBE_FACE_POS_Y, 0);
	g_theRenderer->DrawQuad3d(Vector3::Y_AXIS, Vector3::X_AXIS, -Vector3::Z_AXIS, 1.0f, 1.0f);

	m_diffuse_integrated_target->render_setup_neg_y();
	g_theRenderer->SetColorTarget(m_diffuse_integrated_target, m_diffuse_depth_target, CUBE_FACE_NEG_Y, 0);
	g_theRenderer->DrawQuad3d(-Vector3::Y_AXIS, Vector3::X_AXIS, Vector3::Z_AXIS, 1.0f, 1.0f);

	m_diffuse_integrated_target->render_setup_pos_z();
	g_theRenderer->SetColorTarget(m_diffuse_integrated_target, m_diffuse_depth_target, CUBE_FACE_POS_Z, 0);
	g_theRenderer->DrawQuad3d(Vector3::Z_AXIS, Vector3::X_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);

	m_diffuse_integrated_target->render_setup_neg_z();
	g_theRenderer->SetColorTarget(m_diffuse_integrated_target, m_diffuse_depth_target, CUBE_FACE_NEG_Z, 0);
	g_theRenderer->DrawQuad3d(-Vector3::Z_AXIS, -Vector3::X_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);
}

void SkyBox::integrate_spec(uint spec_res)
{
	if(nullptr == m_spec_integrated_target){
		m_spec_integrated_target = new CubeMap(spec_res, IMAGE_FORMAT_RGBA8, true);
	}

	if(nullptr == m_spec_depth_targets){
		uint num_mips = m_spec_integrated_target->get_num_mip_levels();
		m_spec_depth_targets = new CubeMap*[num_mips];

		for(uint i = 0; i < num_mips; i++){
			uint res = m_spec_integrated_target->calculate_mip_res(i);
			m_spec_depth_targets[i] = new CubeMap(res, IMAGE_FORMAT_D24S8, true);
		}
	}

	if(nullptr == m_spec_mip_buffer){
		create_spec_mips_cb();
	}

	// bind pano_to_cubemap_shader 
	Shader* ld_spec_shader = Shader::find_or_create("Data/Shaders/ld_spec.shader");
	ASSERT_OR_DIE(nullptr != ld_spec_shader, "Failed to load ld spec shader");

	g_theRenderer->SetShader(ld_spec_shader);

	// bind environment map as input texture
	g_theRenderer->SetCubeTexture(0, m_display_texture);

	g_theRenderer->SetConstantBuffer(9, m_spec_mip_buffer);

	// run shader for all mip levels
	uint total_mips = m_spec_integrated_target->get_num_mip_levels();
	for(uint i = 0; i < total_mips; i++){
		m_spec_mip_data.current_mip_level = i;
		m_spec_mip_buffer->Update(g_theRenderer->m_deviceContext, &m_spec_mip_data);

		m_spec_integrated_target->render_setup_pos_x(Vector3::ZERO, i);
		g_theRenderer->SetColorTarget(m_spec_integrated_target, m_spec_depth_targets[i], CUBE_FACE_POS_X, i);
		g_theRenderer->DrawQuad3d(Vector3::X_AXIS, -Vector3::Z_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);

		m_spec_integrated_target->render_setup_neg_x(Vector3::ZERO, i);
		g_theRenderer->SetColorTarget(m_spec_integrated_target, m_spec_depth_targets[i], CUBE_FACE_NEG_X, i);
		g_theRenderer->DrawQuad3d(-Vector3::X_AXIS, Vector3::Z_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);

		m_spec_integrated_target->render_setup_pos_y(Vector3::ZERO, i);
		g_theRenderer->SetColorTarget(m_spec_integrated_target, m_spec_depth_targets[i], CUBE_FACE_POS_Y, i);
		g_theRenderer->DrawQuad3d(Vector3::Y_AXIS, Vector3::X_AXIS, -Vector3::Z_AXIS, 1.0f, 1.0f);

		m_spec_integrated_target->render_setup_neg_y(Vector3::ZERO, i);
		g_theRenderer->SetColorTarget(m_spec_integrated_target, m_spec_depth_targets[i], CUBE_FACE_NEG_Y, i);
		g_theRenderer->DrawQuad3d(-Vector3::Y_AXIS, Vector3::X_AXIS, Vector3::Z_AXIS, 1.0f, 1.0f);

		m_spec_integrated_target->render_setup_pos_z(Vector3::ZERO, i);
		g_theRenderer->SetColorTarget(m_spec_integrated_target, m_spec_depth_targets[i], CUBE_FACE_POS_Z, i);
		g_theRenderer->DrawQuad3d(Vector3::Z_AXIS, Vector3::X_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);

		m_spec_integrated_target->render_setup_neg_z(Vector3::ZERO, i);
		g_theRenderer->SetColorTarget(m_spec_integrated_target, m_spec_depth_targets[i], CUBE_FACE_NEG_Z, i);
		g_theRenderer->DrawQuad3d(-Vector3::Z_AXIS, -Vector3::X_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);
	}
}

void SkyBox::create_spec_mips_cb()
{
	if(nullptr != m_spec_mip_buffer){
		return;
	}

	m_spec_mip_data.current_mip_level = 0;
	m_spec_mip_data.total_mip_levels = m_spec_integrated_target->get_num_mip_levels();
	m_spec_mip_data.zero_mip_width = m_spec_integrated_target->m_resolution;

	m_spec_mip_buffer = new ConstantBuffer(g_theRenderer->m_device, &m_spec_mip_data, sizeof(m_spec_mip_data));
}

//bool CubeMap::load_from_panorama(const char* panorama_filepath, uint resolution)
//{
//	m_texture = new RHITexture2D(g_theRenderer->m_device);
//	m_texture->m_width = resolution;
//	m_texture->m_height = resolution;
//
//	// create shader resources
//	create_empty_cube_texture(resolution);
//	create_render_targets();
//	m_depth_stencil = new RHITexture2D(g_theRenderer->m_device, resolution, resolution, IMAGE_FORMAT_D24S8);
//	create_srv(1);
//
//	// load the panoramic texture
//	RHITexture2D* pano_texture = new RHITexture2D(g_theRenderer->m_device, panorama_filepath);
//
//	// bind pano_to_cubemap_shader 
//	Shader* pano_to_cubemap_shader = Shader::find_or_create("Data/Shaders/panorama_to_cubemap.shader");
//	ASSERT_OR_DIE(nullptr != pano_to_cubemap_shader, "Failed to load pano_to_cubemap shader");
//
//	g_theRenderer->SetShader(pano_to_cubemap_shader);
//
//	// bind pano as input texture
//	g_theRenderer->SetTexture(0, pano_texture);
//
//	// set model to identity
//	g_theRenderer->SetModel(Matrix4::IDENTITY);
//	g_theRenderer->SetView(Matrix4::IDENTITY);
//	g_theRenderer->SetProjection(Matrix4::IDENTITY);
//
//	g_theRenderer->SetViewport(0, 0, resolution, resolution);
//	g_theRenderer->EnableDepth(false, false);
//
//	// draw a quad for each face
//		//D3D11_TEXTURECUBE_FACE_POSITIVE_X  = 0,
//		//D3D11_TEXTURECUBE_FACE_NEGATIVE_X  = 1,
//		//D3D11_TEXTURECUBE_FACE_POSITIVE_Y  = 2,
//		//D3D11_TEXTURECUBE_FACE_NEGATIVE_Y  = 3,
//		//D3D11_TEXTURECUBE_FACE_POSITIVE_Z  = 4,
//		//D3D11_TEXTURECUBE_FACE_NEGATIVE_Z  = 5
//
//	Camera c;
//	c.m_fov = 90.0f;
//	c.m_nz = 0.1f;
//	c.m_fz = 100.0f;
//	c.m_aspect = 1.0f;
//	c.m_transform.set_world_position(Vector3::ZERO);
//
//	// positive x
//	g_theRenderer->SetColorTarget(this, 0);
//	c.look_at(Vector3::X_AXIS);
//	g_theRenderer->SetView(c.get_view());
//	g_theRenderer->SetProjection(c.get_projection());
//	g_theRenderer->DrawQuad3d(Vector3::X_AXIS, -Vector3::Z_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);
//
//	// negative x
//	g_theRenderer->SetColorTarget(this, 1);
//	c.look_at(-Vector3::X_AXIS);
//	g_theRenderer->SetView(c.get_view());
//	g_theRenderer->SetProjection(c.get_projection());
//	g_theRenderer->DrawQuad3d(-Vector3::X_AXIS, Vector3::Z_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);
//
//	// positive y
//	g_theRenderer->SetColorTarget(this, 2);
//	c.look_at(Vector3::Y_AXIS, -Vector3::Z_AXIS);
//	g_theRenderer->SetView(c.get_view());
//	g_theRenderer->SetProjection(c.get_projection());
//	g_theRenderer->DrawQuad3d(Vector3::Y_AXIS, Vector3::X_AXIS, -Vector3::Z_AXIS, 1.0f, 1.0f);
//
//	// negative y 
//	g_theRenderer->SetColorTarget(this, 3);
//	c.look_at(-Vector3::Y_AXIS, Vector3::Z_AXIS);
//	g_theRenderer->SetView(c.get_view());
//	g_theRenderer->SetProjection(c.get_projection());
//	g_theRenderer->DrawQuad3d(-Vector3::Y_AXIS, Vector3::X_AXIS, Vector3::Z_AXIS, 1.0f, 1.0f);
//
//	// positive z
//	g_theRenderer->SetColorTarget(this, 4);
//	c.look_at(Vector3::Z_AXIS);
//	g_theRenderer->SetView(c.get_view());
//	g_theRenderer->SetProjection(c.get_projection());
//	g_theRenderer->DrawQuad3d(Vector3::Z_AXIS, Vector3::X_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);
//
//	// negative z
//	g_theRenderer->SetColorTarget(this, 5);
//	c.look_at(-Vector3::Z_AXIS);
//	g_theRenderer->SetView(c.get_view());
//	g_theRenderer->SetProjection(c.get_projection());
//	g_theRenderer->DrawQuad3d(-Vector3::Z_AXIS, -Vector3::X_AXIS, Vector3::Y_AXIS, 1.0f, 1.0f);
//	
//	g_theRenderer->SetColorTarget(nullptr);
//
//	SAFE_DELETE(pano_texture);
//
//	return true;
//}