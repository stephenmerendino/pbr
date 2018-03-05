#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/VertexShaderStage.hpp"
#include "Engine/RHI/FragmentShaderStage.hpp"

#include "Engine/Renderer/Vertex3.hpp"

#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Common.hpp"

#include "Engine/Profile/auto_profile_log_scope.h"

#include <vector>

void load_shader_from_file(char** out_shader_text, const char* shader_filename)
{
	std::string error_message = Stringf("Failed to load shader [file: %s]\n", shader_filename);
	ASSERT_OR_DIE(LoadBinaryFileToBuffer(out_shader_text, shader_filename), error_message);
}

ShaderProgram::ShaderProgram(RHIDevice* owner)
	:m_owner(owner)
	,m_vertex_stage(nullptr)
	,m_hull_stage(nullptr)
	,m_domain_stage(nullptr)
    ,m_geometry_stage(nullptr)
	,m_fragment_stage(nullptr)
{
}

ShaderProgram::ShaderProgram(RHIDevice* owner, const char* single_file_shader_filename, bool load_geometry_stage)
	:m_owner(owner)
	,m_vertex_stage(nullptr)
	,m_hull_stage(nullptr)
	,m_domain_stage(nullptr)
    ,m_geometry_stage(nullptr)
	,m_fragment_stage(nullptr)
{
	load_all_from_single_file(single_file_shader_filename, load_geometry_stage);
}

ShaderProgram::ShaderProgram(RHIDevice* owner, const char* vertex_filename, const char* geometry_filename, const char* fragment_filename)
	:m_owner(owner)
	,m_vertex_stage(nullptr)
	,m_hull_stage(nullptr)
	,m_domain_stage(nullptr)
    ,m_geometry_stage(nullptr)
	,m_fragment_stage(nullptr)
{
	if(strcmp(vertex_filename, fragment_filename) == 0){
        if(nullptr != geometry_filename) {
            if(strcmp(vertex_filename, geometry_filename) == 0){
        		load_all_from_single_file(vertex_filename);
                return;
            }
        }else{
    		load_all_from_single_file(vertex_filename);
        }
	}

	load_vertex_stage_from_file(vertex_filename);
	load_geometry_stage_from_file(geometry_filename);
	load_fragment_stage_from_file(fragment_filename);
}

ShaderProgram::ShaderProgram(RHIDevice* owner, const char* vertex_filename, const char* hull_filename, const char* domain_filename, const char* geometry_filename, const char* fragment_filename)
	:m_owner(owner)
	,m_vertex_stage(nullptr)
	,m_hull_stage(nullptr)
	,m_domain_stage(nullptr)
    ,m_geometry_stage(nullptr)
	,m_fragment_stage(nullptr)
{
	load_vertex_stage_from_file(vertex_filename);
	load_hull_stage_from_file(hull_filename);
	load_domain_stage_from_file(domain_filename);
	load_geometry_stage_from_file(geometry_filename);
	load_fragment_stage_from_file(fragment_filename);
}

ShaderProgram::~ShaderProgram() 
{
	SAFE_DELETE(m_vertex_stage);
	SAFE_DELETE(m_hull_stage);
	SAFE_DELETE(m_domain_stage);
	SAFE_DELETE(m_geometry_stage);
	SAFE_DELETE(m_fragment_stage);
};

void ShaderProgram::load_all_from_single_file(const char* single_filename, bool load_geometry_stage)
{
	char* shader_source = nullptr;
	load_shader_from_file(&shader_source, single_filename);

	load_vertex_stage_from_text(shader_source, single_filename);
    if(load_geometry_stage){
        load_geometry_stage_from_text(shader_source, single_filename);
    }
	load_fragment_stage_from_text(shader_source, single_filename);
}

void ShaderProgram::load_vertex_stage_from_file(const char* vertex_filename)
{
    if(nullptr == vertex_filename || 0 == strlen(vertex_filename)){
        return;
    }

	char* vertex_shader_source = nullptr;
	load_shader_from_file(&vertex_shader_source, vertex_filename);
	load_vertex_stage_from_text(vertex_shader_source, vertex_filename);
}

void ShaderProgram::load_hull_stage_from_file(const char* hull_filename)
{
	if(nullptr == hull_filename || 0 == strlen(hull_filename)){
		return;
	}

	char* hull_shader_source = nullptr;
	load_shader_from_file(&hull_shader_source, hull_filename);
	load_hull_stage_from_text(hull_shader_source, hull_filename);
}

void ShaderProgram::load_domain_stage_from_file(const char* domain_filename)
{
	if(nullptr == domain_filename || 0 == strlen(domain_filename)){
		return;
	}

	char* domain_shader_source = nullptr;
	load_shader_from_file(&domain_shader_source, domain_filename);
	load_domain_stage_from_text(domain_shader_source, domain_filename);
}

void ShaderProgram::load_geometry_stage_from_file(const char* geometry_filename)
{
    if(nullptr == geometry_filename || 0 == strlen(geometry_filename)){
        return;
    }

	char* geometry_shader_source = nullptr;
	load_shader_from_file(&geometry_shader_source, geometry_filename);
	load_geometry_stage_from_text(geometry_shader_source, geometry_filename);
}

void ShaderProgram::load_fragment_stage_from_file(const char* fragment_filename)
{
    if(nullptr == fragment_filename){
        return;
    }

	char* fragment_shader_source = nullptr;
	load_shader_from_file(&fragment_shader_source, fragment_filename);
	load_fragment_stage_from_text(fragment_shader_source, fragment_filename);
}

void ShaderProgram::load_all_stages_from_text(const char* vertex_text, const char* fragment_text, const char* opt_filename)
{
	load_vertex_stage_from_text(vertex_text, opt_filename);
	load_fragment_stage_from_text(fragment_text, opt_filename);
}

void ShaderProgram::load_vertex_stage_from_text(const char* vertex_text, const char* opt_filename)
{
	size_t raw_vertex_size = strlen(vertex_text);
	m_vertex_stage = new VertexShaderStage(m_owner, vertex_text, raw_vertex_size, DEFAULT_VERTEX_SHADER_ENTRY_POINT, opt_filename);
}

void ShaderProgram::load_hull_stage_from_text(const char* hull_text, const char* opt_filename)
{
	size_t raw_hull_size = strlen(hull_text);
	m_hull_stage = new HullShaderStage(m_owner, hull_text, raw_hull_size, DEFAULT_HULL_SHADER_ENTRY_POINT, opt_filename);
}

void ShaderProgram::load_domain_stage_from_text(const char* domain_text, const char* opt_filename)
{
	size_t raw_domain_size = strlen(domain_text);
	m_domain_stage = new DomainShaderStage(m_owner, domain_text, raw_domain_size, DEFAULT_DOMAIN_SHADER_ENTRY_POINT, opt_filename);
}

void ShaderProgram::load_geometry_stage_from_text(const char* geometry_text, const char* opt_filename)
{
	size_t raw_geometry_size = strlen(geometry_text);
	m_geometry_stage = new GeometryShaderStage(m_owner, geometry_text, raw_geometry_size, DEFAULT_GEOMETRY_SHADER_ENTRY_POINT, opt_filename);
}

void ShaderProgram::load_fragment_stage_from_text(const char* fragment_text, const char* opt_filename)
{
	size_t raw_fragment_size = strlen(fragment_text);
	m_fragment_stage = new FragmentShaderStage(m_owner, fragment_text, raw_fragment_size, DEFAULT_FRAGMENT_SHADER_ENTRY_POINT, opt_filename);
}

int ShaderProgram::find_bind_index_for_name(const char* bind_name) const
{
	int vert_bind_index = INVALID_BIND_INDEX;
    int geom_bind_index = INVALID_BIND_INDEX;
	int frag_bind_index = INVALID_BIND_INDEX;

    if(nullptr != m_vertex_stage && m_vertex_stage->is_valid()){
    	vert_bind_index = m_vertex_stage->find_bind_index_for_name(bind_name);
    }

    if(nullptr != m_geometry_stage && m_geometry_stage->is_valid()){
        geom_bind_index = m_geometry_stage->find_bind_index_for_name(bind_name);
    }

    if(nullptr != m_fragment_stage && m_fragment_stage->is_valid()){
    	frag_bind_index = m_fragment_stage->find_bind_index_for_name(bind_name);
    }

    // Check if none of the stages have it
	if(vert_bind_index == INVALID_BIND_INDEX && 
       frag_bind_index == INVALID_BIND_INDEX && 
       geom_bind_index == INVALID_BIND_INDEX){
		return INVALID_BIND_INDEX;
	}

    // Check if only one of the stages has it
	if(geom_bind_index == INVALID_BIND_INDEX && frag_bind_index == INVALID_BIND_INDEX){
		return vert_bind_index;
	}

	if(vert_bind_index == INVALID_BIND_INDEX && frag_bind_index == INVALID_BIND_INDEX){
		return geom_bind_index;
	}

	if(vert_bind_index == INVALID_BIND_INDEX && geom_bind_index == INVALID_BIND_INDEX){
		return frag_bind_index;
	}

	std::string error = Stringf("Resource [%s] is bount at different indexes for different shader stages", bind_name);

    // Check if only two of the stages has it
    if(vert_bind_index == INVALID_BIND_INDEX){
    	ASSERT_OR_DIE(frag_bind_index == geom_bind_index, error.c_str());
        return frag_bind_index;
    }

    if(geom_bind_index == INVALID_BIND_INDEX){
    	ASSERT_OR_DIE(vert_bind_index == frag_bind_index, error.c_str());
        return vert_bind_index;
    }

    if(frag_bind_index == INVALID_BIND_INDEX){
    	ASSERT_OR_DIE(vert_bind_index == geom_bind_index, error.c_str());
        return vert_bind_index;
    }

    // All three stages have it, make sure the bind indexes match
	ASSERT_OR_DIE(vert_bind_index == geom_bind_index == frag_bind_index, error.c_str());
    return vert_bind_index;
}