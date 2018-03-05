#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/GameConfig.hpp"

#include "Engine/Engine.hpp"
#include "Engine/Profile/profiler.h"
#include "Engine/Renderer/RenderableMesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/CubeMeshes.hpp"
#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/Renderer/scene.h"
#include "Engine/Renderer/sprite.h"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/camera.h"
#include "Engine/Renderer/PointLight.h"
#include "Engine/Renderer/SpotLight.h"
#include "Engine/Renderer/skybox.h"

#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/RHIInstance.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Core/log.h"
#include "Engine/Core/Console.hpp"
#include "Engine/Core/Config.hpp"

#include "Engine/Input/midi.h"

#include "Engine/UI/ui_element.h"
#include "Engine/UI/ui_canvas.h"
#include "Engine/UI/ui_panel.h"
#include "Engine/UI/ui_sprite.h"

#include "Engine/Tools/fbx.hpp"

COMMAND(mit_export_scene, "[string:scene_name] Exports current scene to mitsuba format")
{
	std::string scene_name = args.next_string_arg();
	bool exported = g_theRenderer->get_current_scene()->export_mit(scene_name.c_str());

	if(exported){
		log_printf("Exported scene to file");
	} else{
		log_warningf("Failed to export scene");
	}
}

Game::Game()
	:m_sleep_for_seconds(0.f)
	,m_pl_rad(0.0f)
	,m_pl_arm_length(12.5f)
	,m_sl_rad(0.0f)
	,m_render_ui(false)
	,m_sl(nullptr)
	,m_pl(nullptr)
	,m_roughness(0.1f)
	,m_reflectance(0.9f)
	,m_mat_instance(nullptr)
	,m_mat_instance2(nullptr)
	,m_skybox(nullptr)
	,m_mitsuba_ball_mesh(nullptr)
	,m_cerberus_mesh(nullptr)
	,m_dfg_shader(nullptr)
	,m_dfg_texture(nullptr)
	,m_dfg_dsv(nullptr)
	,m_tess_shader(nullptr)
{
	PROFILE_LOG_SCOPE("Game::Init");
    render_init();
	ui_render_init();
}

Game::~Game()
{
	SAFE_DELETE(m_tess_shader);
	SAFE_DELETE(m_dfg_dsv);
	SAFE_DELETE(m_dfg_texture);
	SAFE_DELETE(m_dfg_shader);
	SAFE_DELETE(m_cerberus_mesh);
	SAFE_DELETE(m_mitsuba_ball_mesh);
	SAFE_DELETE(m_base_element);
	SAFE_DELETE(m_mat_instance);
	SAFE_DELETE(m_mat_instance2);
}

void Game::render_init()
{
	// load environment map
	load_skybox("sanfran");
	//m_environment_map->load_from_panorama("Data/Images/hdri/piazza_martin_lutero_4k.png", 2048);

	//m_dfg_shader = new ShaderProgram(g_theRenderer->m_device, "Data/HLSL/mvp.vert", nullptr, "Data/HLSL/brdf/dfg.frag");
	//m_dfg_dsv = new RHITexture2D(g_theRenderer->m_device, 128, 128, IMAGE_FORMAT_D24S8);
	//m_dfg_texture = new RHITexture2D(g_theRenderer->m_device, 128, 128, IMAGE_FORMAT_R16G16B16A16);

	//m_dfg_texture = new RHITexture2D(g_theRenderer->m_device);
	//m_dfg_texture->load_from_binary_file("Data/Images/dfg.r16g16b16a16.texture", 128, 128, IMAGE_FORMAT_R16G16B16A16);

	Camera* cam = g_theRenderer->get_current_scene()->m_camera;
	
	// debug camera positioned 
	Vector3 start_pos = Vector3::ZERO;
	Vector3 look_at = Vector3::X_AXIS;

	ConfigGetVec3(&start_pos, "camera_start_pos");
	ConfigGetVec3(&look_at, "camera_look_at");

	cam->m_transform.set_world_position(start_pos);
	cam->look_at(look_at);

	m_cerberus_mesh = new Mesh();
	//m_cerberus_mesh->load_from_fbx("Data/FBX/cerberus.fbx", 0.05f);
	m_cerberus_mesh->load_from_file_async("Data/Meshes/cerberus.mesh");

	m_mitsuba_ball_mesh = new Mesh();
	//m_mitsuba_ball_mesh->load_from_fbx("Data/FBX/ue4_shaderball.fbx", 0.01f);

	RenderableMesh* plane = g_theRenderer->get_current_scene()->add_plane();
	plane->set_material("Data/Materials/mahogany.mat");
	plane->m_transform.set_world_scale(30.0f, 1.0f, 30.0f);
	plane->set_tesselation_factors(1, 64, 15.0f, 50.0f);

	Mesh* coffee_pot_mesh = new Mesh();
	//coffee_pot_mesh->load_from_fbx("Data/FBX/coffee_pot.fbx", 0.05f);
	coffee_pot_mesh->load_from_file_async("Data/Meshes/coffee_pot.mesh");
	RenderableMesh* coffee_pot = g_theRenderer->get_current_scene()->add_mesh(coffee_pot_mesh);
	coffee_pot->set_material("Data/Materials/coffee_pot.mat");
	coffee_pot->m_transform.set_world_position(5.0f, 7.0f, 5.0f);

	Mesh* table_mesh = new Mesh();
	//table_mesh->load_from_fbx("Data/FBX/table.fbx", 0.05f);
	table_mesh->load_from_file_async("Data/Meshes/table.mesh");
	RenderableMesh* table = g_theRenderer->get_current_scene()->add_mesh(table_mesh);
	table->set_material("Data/Materials/table.mat");
	table->m_transform.set_world_position(-7.0f, 4.0f, 5.0f);

	//uint grid_size = 12;

	//for(uint x = 0; x < grid_size; x++){
	//	//for(uint y = 0; y < grid_size; y++){
	//		for(uint z = 0; z < grid_size; ++z){
	//			RenderableMesh* mesh0 = g_theRenderer->get_current_scene()->add_cube();
	//			MaterialInstance* mat_inst = mesh0->set_material_instance(0, "Data/Materials/standard.mat");
	//			mat_inst->set_metalness(false);

	//			float level = (x * z) / (float)(grid_size * grid_size);

	//			mat_inst->set_diffuse_reflectance(Vector3(level, level, level));
	//			mat_inst->set_specular_reflectance(Vector3(level, level, level));
	//			mat_inst->set_roughness(level);

	//			float pos_scale = 4.0f;
	//			mesh0->m_transform.set_world_position(Vector3(x * pos_scale, 0.0f, z * pos_scale));
	//		}
	//	//}
	//}

	//RenderableMesh* mesh0 = g_theRenderer->get_current_scene()->add_sphere();
	//RenderableMesh* mesh0 = g_theRenderer->get_current_scene()->add_mesh(m_cerberus_mesh);
	//RenderableMesh* mesh0 = g_theRenderer->get_current_scene()->add_mesh(m_mitsuba_ball_mesh);
	//m_mat_instance = mesh0->set_material_instance(0, "Data/Materials/slippery_stone.mat");
	//m_mat_instance->set_metalness(true);
	//mesh0->m_transform.set_world_position(Vector3(0.0f, 0.0f, 0.0f));
	//mesh0->m_transform.set_world_scale(1.0f, 1.0, 1.0f);
	//mesh0->set_tesselation_factors(1, 12, 10.0f, 200.0f);

	RenderableMesh* mesh1 = g_theRenderer->get_current_scene()->add_sphere();
	//RenderableMesh* mesh1 = g_theRenderer->get_current_scene()->add_mesh(m_mitsuba_ball_mesh);
	//m_mat_instance2 = mesh1->set_material_instance(0, "Data/Materials/standard.mat");
	//m_mat_instance2->set_metalness(true);
	//m_mat_instance2->set_diffuse_reflectance(Vector3(1.0f, 1.0f, 1.0f));
	//m_mat_instance2->set_metal_type(MAT_GOLD);
	//m_mat_instance2->set_diffuse_reflectance(Vector3::ONE);
	mesh1->set_material(0, "Data/Materials/slippery_stone.mat");
	mesh1->m_transform.set_world_position(Vector3(0.0f, 3.0f, 3.0f));
	mesh1->set_material(1, "Data/Materials/standard.mat");
	//mesh1->m_transform.set_world_scale(1.0f, 1.0f, 1.0f);

	RenderableMesh* mesh2 = g_theRenderer->get_current_scene()->add_sphere();
	//RenderableMesh* mesh2 = g_theRenderer->get_current_scene()->add_mesh(m_mitsuba_ball_mesh);
	mesh2->set_material(0, "Data/Materials/rough_brick.mat");
	mesh2->set_material(1, "Data/Materials/standard.mat");
	mesh2->m_transform.set_world_position(Vector3(0.0f, 3.0f, 0.0f));
	//mesh2->m_transform.set_world_scale(1.0f, 1.0f, 1.0f);

	RenderableMesh* mesh3 = g_theRenderer->get_current_scene()->add_sphere();
	//RenderableMesh* mesh3 = g_theRenderer->get_current_scene()->add_mesh(m_mitsuba_ball_mesh);
	m_mat_instance = mesh3->set_material_instance(0, "Data/Materials/standard.mat");
	m_mat_instance->set_metalness(true);
	m_mat_instance->set_diffuse_reflectance(Vector3::ONE);
	m_mat_instance->set_specular_reflectance(Vector3(0.5f, 0.5f, 0.5f));
	mesh3->m_transform.set_world_position(Vector3(0.0f, 3.0f, -3.0f));
	//mesh3->m_transform.set_world_scale(1.0f, 1.0f, 1.0f);

	RenderableMesh* mesh4 = g_theRenderer->get_current_scene()->add_sphere();
	//RenderableMesh* mesh3 = g_theRenderer->get_current_scene()->add_mesh(m_mitsuba_ball_mesh);
	m_mat_instance2 = mesh4->set_material_instance(0, "Data/Materials/standard.mat");
	m_mat_instance2->set_metalness(false);
	m_mat_instance2->set_diffuse_reflectance(Vector3::ONE);
	m_mat_instance2->set_specular_reflectance(Vector3(0.5, 0.5, 0.5));
	mesh4->m_transform.set_world_position(Vector3(0.0f, 3.0f, -6.0f));
	//mesh3->m_transform.set_world_scale(1.0f, 1.0f, 1.0f);

	RenderableMesh* mesh5 = g_theRenderer->get_current_scene()->add_sphere();
	mesh5->set_material(0, "Data/Materials/gold_scuffed.mat");
	mesh5->m_transform.set_world_position(Vector3(3.0f, 3.0f, -3.0f));

	RenderableMesh* mesh6 = g_theRenderer->get_current_scene()->add_sphere();
	mesh6->set_material(0, "Data/Materials/grease_pan.mat");
	mesh6->m_transform.set_world_position(Vector3(3.0f, 3.0f, 0.0f));

	RenderableMesh* mesh7 = g_theRenderer->get_current_scene()->add_sphere();
	mesh7->set_material(0, "Data/Materials/stone_wall.mat");
	mesh7->m_transform.set_world_position(Vector3(3.0f, 3.0f, 3.0f));

	RenderableMesh* mesh8 = g_theRenderer->get_current_scene()->add_sphere();
	mesh8->set_material(0, "Data/Materials/rusted_iron2.mat");
	mesh8->m_transform.set_world_position(Vector3(3.0f, 3.0f, -6.0f));

	RenderableMesh* cerb = g_theRenderer->get_current_scene()->add_mesh(m_cerberus_mesh);
	cerb->set_material(0, "Data/Materials/cerberus.mat");
	cerb->m_transform.set_world_position(Vector3(0.0f, 7.0f, 0.0f));
	cerb->m_transform.set_world_scale(1.0f, 1.0f, 1.0f);

    //g_theRenderer->SetAmbient(0.0f, Rgba::WHITE);
    m_pl = g_theRenderer->get_current_scene()->add_point_light(Vector3(0.0f, 0.0f, 0.0f), Rgba::WHITE, 0.1f, 100.0f, 0.0f);
    //m_pl2 = g_theRenderer->get_current_scene()->add_point_light(Vector3(0.0f, 0.0f, 0.0f), Rgba::TRANSPARENT_BLACK, 0.1f, 20.0f, 0.0f);
	m_pl->set_is_shadow_casting(false);

	m_dl = g_theRenderer->get_current_scene()->add_directional_light(Vector3(-1.0f, -1.0f, -1.0f).Normalized(), Rgba::WHITE, 0.0f);
	m_dl->set_is_shadow_casting(false);

	m_sl = g_theRenderer->get_current_scene()->add_spot_light(Vector3(0.0f, 10.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f), Rgba::GREEN, 0.1f, 25.0f, 0.0f, 20.0f, 60.0f, 2.0f);
	m_sl->m_intensity = 50.0f;
	m_sl->set_is_shadow_casting(true);

	//midi_bindf_slider(3, &m_pl->m_color.r, 0, 255);
	//midi_bindf_slider(4, &m_pl->m_color.g, 0, 255);
	//midi_bindf_slider(5, &m_pl->m_color.b, 0, 255);
	midi_bindf_slider(6, &m_pl->m_power, 0.0f, 500.0f);
	//midi_bindf_slider(1, &m_dl->m_power, 0.0f, 40.0f);
	midi_bindf_slider(7, &m_pl_arm_length, 0.0f, 15.0f);
	midi_bindf_slider(8, &m_pl->m_position.y, -10.0f, 20.0f);
	//midi_bindf_slider(8, &m_pl2->m_position.y, 20.0f, -10.0f);

	midi_bindf_slider(4, &m_sl->m_position.y, 0.0f, 25.0f);
	midi_bindf_slider(5, &m_sl->m_intensity, 0.0f, 1000.0f);
	midi_bindf_slider(6, &m_sl->m_inner_angle, 1.0f, 90.0f);
	midi_bindf_slider(7, &m_sl->m_outer_angle, 1.0f, 90.0f);
	midi_bindf_slider(8, &m_sl->m_exp, 1.0f, 4.0f);

	midi_bindf_knob(6, &m_roughness, 0.0f, 1.0f);
	midi_bindf_knob(7, &m_reflectance, 0.0f, 1.0f);

	midi_bindf_knob(8, &m_pl_rad, 0.0f, TWO_M_PI);
	midi_bindf_knob(8, &m_sl_rad, 0.0f, TWO_M_PI);

	m_pl_rad = M_PI * 0.25f;
	m_pl->m_position.y = 5.0f;
	//m_pl2->m_position.y = -5.0f;
}


void Game::update(float ds)
{
    PROFILE_SCOPE_FUNCTION();

	update_sleep(ds);

	if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F2)){
		g_theRenderer->get_current_scene()->toggle_skybox_enabled();
	}

	if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F3)){
		g_theRenderer->get_current_scene()->toggle_wireframe_mode();
	}

	if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F4)){
		Camera* cam = g_theRenderer->get_current_scene()->m_camera;
		Vector3 pos = cam->m_transform.calc_world_position();
		Vector3 look_at = pos + cam->m_transform.calc_world_forward();
		ConfigSet("camera_start_pos", pos);
		ConfigSet("camera_look_at", look_at);
	}

	if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F10)){
		g_theRenderer->take_snapshot();
	}

    m_pl->m_position.x = cosf(m_pl_rad) * m_pl_arm_length;
    m_pl->m_position.z = sinf(m_pl_rad) * m_pl_arm_length;

    //m_pl2->m_position.x = cosf(-m_pl_rad) * m_pl_arm_length;
    //m_pl2->m_position.z = sinf(-m_pl_rad) * m_pl_arm_length;

	m_dl->m_direction = -Vector3(cosf(m_pl_rad), 1.0f, sinf(m_pl_rad)).Normalized();

	if(nullptr != m_sl){
	    m_sl->m_position.x = cosf(m_sl_rad) * 2.5f;
	    m_sl->m_position.z = sinf(m_sl_rad) * 2.5f;
	}
}

void Game::sleep_for_seconds(float seconds)
{
	m_sleep_for_seconds += seconds;
}

void Game::update_sleep(float& ds)
{
	if(m_sleep_for_seconds <= 0.f){
		return;
    }

	if(m_sleep_for_seconds >= ds){
		ds = 0.f;
		m_sleep_for_seconds -= ds;
	} else{
		ds -= m_sleep_for_seconds;
		m_sleep_for_seconds = 0.f;
	}
}

void Game::render() const
{
	if(nullptr != m_mat_instance){
		m_mat_instance->set_specular_reflectance(Vector3(m_reflectance, m_reflectance, m_reflectance));
		m_mat_instance->set_roughness(m_roughness);

		//m_mat_instance2->set_diffuse_reflectance(Vector3(m_reflectance, m_reflectance, m_reflectance));
		m_mat_instance2->set_specular_reflectance(Vector3(m_reflectance, m_reflectance, m_reflectance));
		m_mat_instance2->set_roughness(m_roughness);
	}
}

void Game::quit()
{
   g_app->set_is_quitting(true); 
}

void Game::ui_render_init()
{
	UICanvas* canvas = new UICanvas();
	canvas->set_target_res(1080.0f);
	canvas->set_render_target(g_theRenderer->m_output->m_renderTarget);
	m_base_element = canvas;

	UIPanel* panel = canvas->add_child<UIPanel>();
	panel->set_size(1.0f, 1.0f, 0.0f, 0.0f);
	panel->set_pivot(0.5f, 0.5f);

	m_base_element->set_debug_render(false, false);
}

void Game::render_test_ui() const
{
	if(!m_render_ui){
		return;
	}

	m_base_element->render();
}

void Game::load_skybox(const std::string& env_name)
{
	CubeMapImagePaths im;

	std::string neg_x = Stringf("Data/Images/cubemap/%s/negx.jpg", env_name.c_str());
	im.negX = neg_x.c_str();

	std::string pos_x = Stringf("Data/Images/cubemap/%s/posx.jpg", env_name.c_str()).c_str();
	im.posX = pos_x.c_str();

	std::string neg_z = Stringf("Data/Images/cubemap/%s/negz.jpg", env_name.c_str()).c_str();
	im.negZ = neg_z.c_str();

	std::string pos_z = Stringf("Data/Images/cubemap/%s/posz.jpg", env_name.c_str()).c_str();
	im.posZ = pos_z.c_str();

	std::string neg_y = Stringf("Data/Images/cubemap/%s/negy.jpg", env_name.c_str()).c_str();
	im.negY	= neg_y.c_str();

	std::string pos_y = Stringf("Data/Images/cubemap/%s/posy.jpg", env_name.c_str()).c_str();
	im.posY	 = pos_y.c_str();

	// load environment map
	SkyBox* sb = new SkyBox();
	sb->load_from_face_images(g_theRenderer->m_device, im);
	sb->bake_lighting(512, 1024);

	g_theRenderer->get_current_scene()->set_skybox(sb);
	g_theRenderer->get_current_scene()->set_skybox_enabled(true);

	SAFE_DELETE(m_skybox);
	m_skybox = sb;
}

COMMAND(export_cubemap, "Exports cubemap to six seperate images")
{
	std::string filename = args.next_string_arg();
	g_game->m_skybox->m_display_texture->save_to_seperate_images(filename.c_str());
}

COMMAND(load_env, "Loads an environment")
{
	std::string env_name = args.next_string_arg();
	g_game->load_skybox(env_name);
}