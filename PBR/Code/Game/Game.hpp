#pragma once

#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Material.hpp"
#include <string>

class RenderableMesh;
class ShaderProgram;
class PointLight;
class SpotLight;
class SkyBox;

class UIElement;

class Game
{
    public:
    	float	                        m_sleep_for_seconds;
        PointLight*						m_pl;
        PointLight*						m_pl2;
		DirectionalLight*				m_dl;
		SpotLight*						m_sl;
        float                           m_pl_rad;
		float							m_pl_arm_length;
		float							m_sl_rad;

		Mesh*							m_mitsuba_ball_mesh;
		Mesh*							m_cerberus_mesh;
		std::vector<RenderableMesh*>	m_meshes;

		SkyBox*							m_skybox;
		ShaderProgram*					m_dfg_shader;
		RHITexture2D*					m_dfg_texture;
		RHITexture2D*					m_dfg_dsv;

		MaterialInstance*				m_mat_instance;
		MaterialInstance*				m_mat_instance2;
		float							m_roughness;
		float							m_reflectance;

		ShaderProgram*					m_tess_shader;

	public:
		UIElement* m_base_element;
		bool m_render_ui;

    public:
    	Game();
    	~Game();

    	void update(float ds);
    	void render() const;

    	void sleep_for_seconds(float seconds);
    	void update_sleep(float& ds);

        void quit();

    public:
        void render_init();
		void ui_render_init();
		void render_test_ui() const;

		void load_skybox(const std::string& env_name);
};