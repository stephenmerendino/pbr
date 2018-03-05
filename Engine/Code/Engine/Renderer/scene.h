#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Material.hpp"

class RenderableMesh;
class RHITexture2D;
class Camera;
class CubeMap;
class PointLight;
class DirectionalLight;
class SpotLight;
class SkyBox;

#include <vector>

struct point_light_depth_info_t
{
	Vector4 world_pos;
	float far_plane;
	float _padding[3];
};

struct ibl_data_t
{
	uint total_mip_levels;
	uint dfg_texture_width;
	float _padding[2];
};

class Scene
{
    public:
        std::vector<RenderableMesh*> m_renderable_meshes;
        std::vector<PointLight*> m_point_lights;
        std::vector<SpotLight*> m_spot_lights;
        std::vector<DirectionalLight*> m_directional_lights;
        Camera* m_camera;

		SkyBox* m_skybox;
		bool m_skybox_enabled;

		ibl_data_t m_ibl_data;
		ConstantBuffer* m_ibl_cb;

		point_light_depth_info_t m_pl_depth_info;
		ConstantBuffer* m_pl_depth_cb;

        bool m_debug_mode;
		bool m_wireframe_mode_enabled;

    public:
        Scene();
        ~Scene();

		void init_constant_buffers();

        void update(float ds);
		void prerender();
        void render();
		void render_scene_geometry();

        bool export_mit(const char* filename);

    public:
        RenderableMesh* add_cube();
        RenderableMesh* add_sphere();
        RenderableMesh* add_plane();
		RenderableMesh* add_mesh(Mesh* mesh, const Material* mat = nullptr);

        PointLight* add_point_light();
		SpotLight* add_spot_light();
        DirectionalLight* add_directional_light();

        PointLight* add_point_light(const Vector3& position, const Rgba& color, float near_cutoff, float far_cutoff, float intensity);
		SpotLight* add_spot_light(const Vector3& position, const Vector3& direction, const Rgba& color, float near_cutoff, float far_cutoff, float intensity, float inner_angle, float outer_angle, float exp);
        DirectionalLight* add_directional_light(const Vector3& direction, const Rgba& color, float intensity, bool is_shadow_casting = false);

		void toggle_skybox_enabled();

		void set_skybox(SkyBox* skybox);
		void set_skybox_enabled(bool enabled);
		void render_skybox();

		void enable_wireframe_mode(bool enabled);
		void toggle_wireframe_mode();

		void setup_shadow_maps();
		void setup_punctual_lights();
		void setup_ibl();
		void render_renderable_meshes();
		void render_debug();

		void compute_directional_light_shadows();
		void compute_spot_light_shadows();
		void compute_point_light_shadows();
};