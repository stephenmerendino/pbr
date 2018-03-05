#pragma once

#include "Engine/Renderer/Renderable.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Renderer/SkeletalTransformHierarchy.hpp"
#include "Engine/Renderer/transform.h"

#define MAX_NUM_MATERIALS 5

class Mesh;
class Material;
class MaterialInstance;

struct tesselation_factors_t
{
	uint min_tess_factor;
	uint max_tess_factor;
	float min_lod_distance;
	float max_lod_distance;
};

class RenderableMesh : public Renderable
{
    public:
    	Mesh* m_mesh;
		std::vector<const Material*> m_materials;
        Transform m_transform;
		bool m_wireframe_mode_enabled;
		tesselation_factors_t m_tess_factors;

    public:
    	RenderableMesh();
    	RenderableMesh(Mesh* mesh, const Material* material);
    	~RenderableMesh();

    	void set_mesh(Mesh* mesh);

    	void set_material(const Material* material);
    	void set_material(const char* material_name);
		MaterialInstance* set_material_instance(const Material* material);
		MaterialInstance* set_material_instance(const char* material_name);

    	void set_material(uint mat_id, const Material* material);
    	void set_material(uint mat_id, const char* material_name);
		MaterialInstance* set_material_instance(uint mat_id, const Material* material);
		MaterialInstance* set_material_instance(uint mat_id, const char* material_name);

		void set_wireframe_enabled(bool enabled);
		void set_tesselation_factors(uint min_tess_factor, uint max_tess_factor, float min_lod_distance, float max_lod_distance);

    	virtual void draw() override;
};