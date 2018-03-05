#include "Engine/Renderer/RenderableMesh.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/Profile/auto_profile_log_scope.h"

RenderableMesh::RenderableMesh()
	:Renderable()
	,m_mesh(nullptr)
	,m_wireframe_mode_enabled(false)
{
	m_materials.resize(MAX_NUM_MATERIALS);
	set_tesselation_factors(1, 2, 20.0f, 200.0f);
}

RenderableMesh::RenderableMesh(Mesh* mesh, const Material* material)
	:Renderable()
	,m_mesh(mesh)
	,m_wireframe_mode_enabled(false)
{
	m_materials.resize(MAX_NUM_MATERIALS);
	m_materials[0] = material;
	set_tesselation_factors(1, 2, 20.0f, 200.0f);
}

RenderableMesh::~RenderableMesh()
{
	Renderable::~Renderable();
}

void RenderableMesh::set_mesh(Mesh* mesh)
{
	m_mesh = mesh;
}

void RenderableMesh::set_material(const Material* material)
{
	set_material(0, material);
}

void RenderableMesh::set_material(const char* material_name)
{
	set_material(0, material_name);
}

MaterialInstance* RenderableMesh::set_material_instance(const Material* material)
{
	return set_material_instance(0, material);
}

MaterialInstance* RenderableMesh::set_material_instance(const char* material_name)
{
	return set_material_instance(0, material_name);
}

void RenderableMesh::set_material(uint mat_id, const Material* material)
{
	m_materials[mat_id] = material;
}

void RenderableMesh::set_material(uint mat_id, const char* material_name)
{
    const Material* mat = Material::find_or_create(material_name);
	set_material(mat_id, mat);
}

MaterialInstance* RenderableMesh::set_material_instance(uint mat_id, const Material* material)
{
	MaterialInstance* mat_instance = nullptr;

    if(nullptr != material){
		mat_instance = material->create_instance();
    }else{
        mat_instance = Material::DEBUG_MATERIAL->create_instance();
    }

	set_material(mat_id, mat_instance);

	return mat_instance;
}

MaterialInstance* RenderableMesh::set_material_instance(uint mat_id, const char* material_name)
{
    const Material* mat = Material::find_or_create(material_name);
	return set_material_instance(mat_id, mat);
}

void RenderableMesh::set_wireframe_enabled(bool enabled)
{
	m_wireframe_mode_enabled = enabled;
}

void RenderableMesh::set_tesselation_factors(uint min_tess_factor, uint max_tess_factor, float min_lod_distance, float max_lod_distance)
{
	m_tess_factors.min_tess_factor = min_tess_factor;
	m_tess_factors.max_tess_factor = max_tess_factor;
	m_tess_factors.min_lod_distance = min_lod_distance;
	m_tess_factors.max_lod_distance = max_lod_distance;
}

void RenderableMesh::draw()
{
	g_theRenderer->draw_renderable_mesh(this);
}