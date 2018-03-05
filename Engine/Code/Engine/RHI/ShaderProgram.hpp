#pragma once

#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/VertexShaderStage.hpp"
#include "Engine/RHI/HullShaderStage.hpp"
#include "Engine/RHI/DomainShaderStage.hpp"
#include "Engine/RHI/GeometryShaderStage.hpp"
#include "Engine/RHI/FragmentShaderStage.hpp"

class RHIDevice;

class ShaderProgram
{
public:
	VertexShaderStage* m_vertex_stage;
	HullShaderStage* m_hull_stage;
	DomainShaderStage* m_domain_stage;
    GeometryShaderStage* m_geometry_stage;
	FragmentShaderStage* m_fragment_stage;

	RHIDevice* m_owner;

public:
	ShaderProgram(RHIDevice* owner);
	ShaderProgram(RHIDevice* owner, const char* single_file_shader_filename, bool load_geometry_stage = false);
	ShaderProgram(RHIDevice* owner, const char* vertex_filename, const char* geometry_filename, const char* fragment_filename);
	ShaderProgram(RHIDevice* owner, const char* vertex_filename, const char* hull_filename, const char* domain_filename, const char* geometry_filename, const char* fragment_filename);
	~ShaderProgram();

	/*void load_from_raw_source(RHIDevice* device, const char* raw_vertex_text, const char* raw_fragment_text);*/
	inline bool is_valid() const { return m_vertex_stage->is_valid() && m_fragment_stage->is_valid(); }

	void load_all_from_single_file(const char* single_filename, bool load_geometry_stage = false);
	void load_vertex_stage_from_file(const char* vertex_filename);
	void load_hull_stage_from_file(const char* hull_filename);
	void load_domain_stage_from_file(const char* domain_filename);
	void load_geometry_stage_from_file(const char* geometry_filename);
	void load_fragment_stage_from_file(const char* fragment_filename);

	void load_all_stages_from_text(const char* vertex_text, const char* fragment_text, const char* opt_filename = nullptr);
	void load_vertex_stage_from_text(const char* vertex_text, const char* opt_filename = nullptr);
	void load_hull_stage_from_text(const char* hull_text, const char* opt_filename = nullptr);
	void load_domain_stage_from_text(const char* domain_text, const char* opt_filename = nullptr);
	void load_geometry_stage_from_text(const char* geometry_text, const char* opt_filename = nullptr);
	void load_fragment_stage_from_text(const char* fragment_text, const char* opt_filename = nullptr);

	int find_bind_index_for_name(const char* bind_name) const;
};