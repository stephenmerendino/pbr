#include "Engine/Renderer/scene.h"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/CubeMeshes.hpp"
#include "Engine/Renderer/SphereMeshes.hpp"
#include "Engine/Renderer/QuadMeshes.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RenderableMesh.hpp"
#include "Engine/Renderer/camera.h"
#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/Renderer/mitsuba_scene_exporter.h"
#include "Engine/Renderer/PointLight.h"
#include "Engine/Renderer/DirectionalLight.h"
#include "Engine/Renderer/SpotLight.h"
#include "Engine/Renderer/skybox.h"

#include "Engine/RHI/RHIInstance.hpp"
#include "Engine/RHI/RHITexture2D.hpp"

Scene::Scene()
	:m_camera(nullptr)
	,m_skybox(nullptr)
	,m_skybox_enabled(false)
	,m_debug_mode(false)
	,m_wireframe_mode_enabled(false)
	,m_ibl_cb(nullptr)
	,m_pl_depth_cb(nullptr)
{
	init_constant_buffers();
    m_camera = new Camera();
    m_camera->m_transform.set_world_position(Vector3(0.0f, 0.0f, -5.0f));
}

Scene::~Scene()
{
	for(uint i = 0; i < (uint)m_renderable_meshes.size(); i++){
		SAFE_DELETE(m_renderable_meshes[i]);
	}

	SAFE_DELETE(m_ibl_cb);
	SAFE_DELETE(m_pl_depth_cb);
	SAFE_DELETE(m_skybox);
    SAFE_DELETE(m_camera);
}

void Scene::init_constant_buffers()
{
	MemZero(&m_ibl_data);
	m_ibl_data.dfg_texture_width = g_theRenderer->m_dfg_texture->GetWidth();
	m_ibl_cb = new ConstantBuffer(g_theRenderer->m_device, &m_ibl_data, sizeof(m_ibl_data));

	MemZero(&m_pl_depth_info);
	m_pl_depth_cb = new ConstantBuffer(g_theRenderer->m_device, &m_pl_depth_info, sizeof(m_pl_depth_info));
}

void Scene::update(float ds)
{
	if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F1)){
		m_debug_mode = !m_debug_mode;
	}

    // update camera movement
    m_camera->update(ds);
}

void Scene::render_skybox()
{
	if(m_skybox_enabled && nullptr != m_skybox){
		g_theRenderer->set_camera(m_camera);

		// use model to transform skybox to be positioned at camera position
		Matrix4 model;
		model.set_translation(m_camera->get_world_position(), 1.0f);
		g_theRenderer->SetModel(model);

		//g_theRenderer->DrawSkyBox(m_skybox);
		g_theRenderer->DrawSkyBox(m_skybox->m_spec_integrated_target);
	}
}

void Scene::prerender()
{
	setup_shadow_maps();
	setup_punctual_lights();
	setup_ibl();
}

void Scene::render()
{
	render_skybox();


	//----------------------------------------------------
	// Hacking in light 0's shadow mapping
	//g_theRenderer->SetTexture(10 , m_directional_lights[0]->m_depth_dsv);
	g_theRenderer->SetTexture(10 , m_spot_lights[0]->m_depth_dsv);
	//g_theRenderer->SetCubeTexture(10, m_point_lights[0]->m_depth_dsv);
	//----------------------------------------------------



	render_renderable_meshes();
	render_debug();
}

void Scene::render_scene_geometry()
{
    for(unsigned int rmidx = 0; rmidx < m_renderable_meshes.size(); ++rmidx){
        RenderableMesh* rm = m_renderable_meshes[rmidx];
        if(nullptr == rm){
            continue;
        }

		g_theRenderer->SetModel(rm->m_transform.calc_world_matrix());
		g_theRenderer->draw_mesh(rm->m_mesh);
    }
}

bool Scene::export_mit(const char* filename)
{
	MitsubaSceneExporter exporter(filename, this);
	return exporter.export_to_file();
}

RenderableMesh* Scene::add_cube()
{
    MeshBuilder mb;
    Meshes::build_cube_3d(mb, Vector3(0.0f, 0.0f, 0.0f), 1.0f);

    Mesh* mesh = new Mesh();
    mb.copy_to_mesh(mesh);

	return add_mesh(mesh);
}

RenderableMesh* Scene::add_sphere()
{
    MeshBuilder mb;
    Meshes::build_uv_sphere(mb, Vector3::ZERO, 1.0f, Rgba::WHITE, 64);

    Mesh* mesh = new Mesh();
    mb.copy_to_mesh(mesh);

	return add_mesh(mesh);
}

RenderableMesh* Scene::add_plane()
{
	MeshBuilder mb;
	Meshes::build_quad_3d(mb, Vector3::ZERO, Vector3::X_AXIS, Vector3::Z_AXIS, 0.5f, 0.5f);

	Mesh* mesh = new Mesh();
	mb.copy_to_mesh(mesh);

	return add_mesh(mesh);
}

RenderableMesh* Scene::add_mesh(Mesh* mesh, const Material* mat)
{
	if(nullptr == mat){
		mat = Material::DEBUG_MATERIAL;
	}

	RenderableMesh* render_mesh = new RenderableMesh(mesh, mat);
	m_renderable_meshes.push_back(render_mesh);
	return render_mesh;
}

PointLight* Scene::add_point_light()
{
	PointLight* pl = new PointLight();
	m_point_lights.push_back(pl);
	return pl;
}

SpotLight* Scene::add_spot_light()
{
	SpotLight* sl = new SpotLight();
	m_spot_lights.push_back(sl);
	return sl;
}

DirectionalLight* Scene::add_directional_light()
{
	DirectionalLight* dl = new DirectionalLight();
	m_directional_lights.push_back(dl);
	return dl;
}

PointLight* Scene::add_point_light(const Vector3& position, const Rgba& color, float near_cutoff, float far_cutoff, float intensity)
{
	PointLight* pl = new PointLight(position, color, near_cutoff, far_cutoff, intensity);
	m_point_lights.push_back(pl);
    return pl;
}

SpotLight* Scene::add_spot_light(const Vector3& position, const Vector3& direction, const Rgba& color, float near_cutoff, float far_cutoff, float intensity, float inner_angle, float outer_angle, float exp)
{
	SpotLight* sl = new SpotLight(position, direction, color, intensity, near_cutoff, far_cutoff, inner_angle, outer_angle, exp);
	m_spot_lights.push_back(sl);
	return sl;
}

DirectionalLight* Scene::add_directional_light(const Vector3& direction, const Rgba& color, float intensity, bool is_shadow_casting)
{
    DirectionalLight* dl = new DirectionalLight(direction, color, intensity, is_shadow_casting);
	m_directional_lights.push_back(dl);
	return dl;
}

void Scene::set_skybox(SkyBox* skybox)
{
	m_skybox = skybox;
}

void Scene::set_skybox_enabled(bool enabled)
{
	m_skybox_enabled = enabled;
}

void Scene::toggle_skybox_enabled()
{
	m_skybox_enabled = !m_skybox_enabled;
}

void Scene::enable_wireframe_mode(bool enabled)
{
	m_wireframe_mode_enabled = enabled;
	for(RenderableMesh* rm : m_renderable_meshes){
		rm->set_wireframe_enabled(enabled);
	}
}

void Scene::toggle_wireframe_mode()
{
	enable_wireframe_mode(!m_wireframe_mode_enabled);
}

void Scene::setup_shadow_maps()
{
	compute_directional_light_shadows();
	compute_spot_light_shadows();
	compute_point_light_shadows();
}

void Scene::compute_directional_light_shadows()
{
	g_theRenderer->SetTexture(10 , nullptr); // unbind any textures that might have been set as a depth srv
	g_theRenderer->SetShaderByName("Data/Shaders/write_depth.shader");
	g_theRenderer->EnableDepth(true, true);

	for(size_t dl_idx = 0; dl_idx < m_directional_lights.size(); dl_idx++){
		DirectionalLight* dl = m_directional_lights[dl_idx];

		if(!dl->is_shadow_casting()){
			continue;
		}

		dl->setup_depth_write(); // set the light view, light projection, bind the light dsv
		render_scene_geometry();
	}
}

void Scene::compute_spot_light_shadows()
{
	g_theRenderer->SetTexture(10 , nullptr); // unbind any textures that might have been set as a depth srv
	g_theRenderer->SetShaderByName("Data/Shaders/write_depth.shader");
	g_theRenderer->EnableDepth(true, true);

	for(size_t sl_idx = 0; sl_idx < m_spot_lights.size(); sl_idx++){
		SpotLight* sl = m_spot_lights[sl_idx];

		if(!sl->is_shadow_casting()){
			continue;
		}

		sl->setup_depth_write(); // set the light view, light projection, bind the light dsv
		render_scene_geometry();
	}
}

void Scene::compute_point_light_shadows()
{
	g_theRenderer->SetTexture(10 , nullptr); // unbind any textures that might have been set as a depth srv
	g_theRenderer->SetShaderByName("Data/Shaders/write_omni_depth.shader");
	g_theRenderer->SetConstantBuffer(6, m_pl_depth_cb);

	for(size_t pl_idx = 0; pl_idx < m_point_lights.size(); pl_idx++){
		PointLight* pl = m_point_lights[pl_idx];

		if(!pl->is_shadow_casting()){
			continue;
		}

		for(uint face = 0; face < NUM_CUBE_FACES; ++face){
			pl->setup_depth_write(face);

			m_pl_depth_info.world_pos.xyz = pl->m_position;
			m_pl_depth_info.far_plane = pl->m_far_cutoff;
			m_pl_depth_cb->Update(g_theRenderer->m_deviceContext, &m_pl_depth_info);

			render_scene_geometry();
		}
	}
}

void Scene::setup_punctual_lights()
{
    for(unsigned int plidx = 0; plidx < m_point_lights.size(); plidx++){
        g_theRenderer->SetPointLight(plidx, m_point_lights[plidx]);
    }

    for(unsigned int slidx = 0; slidx < m_spot_lights.size(); slidx++){
        g_theRenderer->SetSpotLight(slidx, m_spot_lights[slidx]);
    }

    for(unsigned int dlidx = 0; dlidx < m_directional_lights.size(); dlidx++){
        g_theRenderer->SetDirectionalLight(dlidx, m_directional_lights[dlidx]);
    }
}

void Scene::setup_ibl()
{
	g_theRenderer->SetEnvironmentMapEnabled(m_skybox_enabled);
	if(m_skybox_enabled && nullptr != m_skybox){
		m_ibl_data.total_mip_levels = m_skybox->m_spec_integrated_target->get_num_mip_levels();
		m_ibl_cb->Update(g_theRenderer->m_deviceContext, &m_ibl_data);
		g_theRenderer->SetConstantBuffer(5, m_ibl_cb);

		g_theRenderer->SetCubeTexture(7, m_skybox->m_diffuse_integrated_target);
		g_theRenderer->SetCubeTexture(8, m_skybox->m_spec_integrated_target);
		g_theRenderer->SetTexture(9, g_theRenderer->m_dfg_texture);
	}
}

void Scene::render_renderable_meshes()
{
	g_theRenderer->set_camera(m_camera);
	g_theRenderer->EnableDepth(true, true);

    //draw each renderable mesh
    for(unsigned int rmidx = 0; rmidx < m_renderable_meshes.size(); ++rmidx){
        RenderableMesh* rm = m_renderable_meshes[rmidx];
        if(nullptr == rm){
            continue;
        }

        g_theRenderer->draw_renderable_mesh(rm);
    }
}

void Scene::render_debug()
{
	if(!m_debug_mode){
		return;
	}

	g_theRenderer->set_camera(m_camera);
	g_theRenderer->EnableDepth(true, true);

    g_theRenderer->DrawWorldGridXZ(50.0f, 1.0f, 0.2f, Rgba(200, 255, 200, 100), Rgba(255, 255, 255, 40));
    g_theRenderer->DrawWorldAxes();

    for(unsigned int plidx = 0; plidx < m_point_lights.size(); plidx++){
        PointLight* pl = m_point_lights[plidx];
        if(nullptr == pl){
            continue;
        }

        g_theRenderer->DebugDrawCross3d(pl->m_position, pl->m_color, 0.5f);

		g_theRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
		g_theRenderer->DrawUVSphere(pl->m_position, pl->m_far_cutoff, pl->m_color.GetScaledAlpha(0.25f), 32);
    }

    for(unsigned int slidx = 0; slidx < m_spot_lights.size(); slidx++){
        SpotLight* sl = m_spot_lights[slidx];
        if(nullptr == sl){
            continue;
        }

        g_theRenderer->DebugDrawCross3d(sl->m_position, Rgba::YELLOW, 0.5f);
		g_theRenderer->DebugDrawLine3d(sl->m_position, sl->m_direction, 2.5f, 1.0f, Rgba::PINK, Rgba::PINK);
    }
}