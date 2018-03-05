#include "Engine/Renderer/Material.hpp"
#include "Engine/Engine.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/Profile/auto_profile_log_scope.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//----------------------------------------------------------------------
// Material
//----------------------------------------------------------------------

std::map<std::string, Material*> Material::s_database;
const Material* Material::DEBUG_MATERIAL = nullptr;

void Material::init()
{
    DEBUG_MATERIAL = find_or_create("Data/Materials/debug.mat");
}

void Material::shutdown()
{
	std::map<std::string, Material*>::iterator it = s_database.begin();
	while(it != s_database.end()){
		Material* m = it->second;
		SAFE_DELETE(m);
		it++;
	}
	s_database.clear();
}

const Material* Material::find_or_create(const char* mat_xml_file)
{
	std::map<std::string, Material*>::iterator found = s_database.find(mat_xml_file);
	if(found != s_database.end()){
		return found->second;
	}

    const Material* new_mat;
    if(Material::validate_xml(mat_xml_file)){
    	new_mat = new Material(mat_xml_file);
    }else{
        new_mat = Material::DEBUG_MATERIAL;
    }

	return new_mat;
}

bool Material::validate_xml(const char* xml_filename)
{
    FILE* f;
    errno_t error = fopen_s(&f, xml_filename, "r");

    if(0 == error){
        fclose(f);
        return true;
    }else{
        return false;
    }
}

Material::Material(const char* mat_xml_file)
	:m_smoothness(0.9f)
	,m_specular_reflectance(0.5f, 0.5f, 0.5f)
	,m_diffuse_reflectance(1.0f, 0.0f, 1.00f)
	,m_is_metal(false)
	,m_height_map_scale(0.25f)
{
	memset(&m_textures[0], 0, sizeof(RHITexture2D*) * MAX_NUM_TEXTURES);
	memset(&m_constant_buffers[0], 0, sizeof(ConstantBuffer*) * MAX_NUM_CONSTANT_BUFFERS);

	// validate xml
	XMLElement mat_xml = xml::parse_xml_file(mat_xml_file);
	xml::validate_xml_node(mat_xml, "shader, diffuse, albedo, normal, spec, smoothness, roughness, metalness, ao, height, texture, height_map_scale", "", "shader", "");

	// find or create shader
	XMLElement shader_xml = xml::get_element_child(mat_xml, "shader");
	xml::validate_xml_node(shader_xml, "", "src", "", "src");
	std::string shader_src = xml::parse_xml_attribute(shader_xml, "src", shader_src);
	m_shader = Shader::find_or_create(shader_src.c_str());
	
	// create and save diffuse
	if(xml::element_has_child(mat_xml, "diffuse")){
		XMLElement diffuse_xml = xml::get_element_child(mat_xml, "diffuse");
		RHITexture2D* diffuse = create_texture_from_xml(diffuse_xml);
		set_diffuse_texture(diffuse);
	}

	// create and save albedo
	if(xml::element_has_child(mat_xml, "albedo")){
		XMLElement albedo_xml = xml::get_element_child(mat_xml, "albedo");
		RHITexture2D* albedo = create_texture_from_xml(albedo_xml);
		set_albedo_texture(albedo);
	}

	// create and save smoothness
	if(xml::element_has_child(mat_xml, "smoothness")){
		XMLElement smoothness_xml = xml::get_element_child(mat_xml, "smoothness");
		RHITexture2D* smoothness = create_texture_from_xml(smoothness_xml);
		set_smoothness_texture(smoothness);
	}

	// create and save normal
	if(xml::element_has_child(mat_xml, "normal")){
		XMLElement normal_xml = xml::get_element_child(mat_xml, "normal");
		RHITexture2D* normal = create_texture_from_xml(normal_xml);
		set_normal_texture(normal);
	}

	// create and save spec
	if(xml::element_has_child(mat_xml, "spec")){
		XMLElement spec_xml = xml::get_element_child(mat_xml, "spec");
		RHITexture2D* spec = create_texture_from_xml(spec_xml);
		set_spec_texture(spec);
	}

	// create and save spec
	if(xml::element_has_child(mat_xml, "metalness")){
		XMLElement metalness_xml = xml::get_element_child(mat_xml, "metalness");
		RHITexture2D* metalness = create_texture_from_xml(metalness_xml);
		set_metalness_texture(metalness);
	}

	// create and save spec
	if(xml::element_has_child(mat_xml, "ao")){
		XMLElement ao_xml = xml::get_element_child(mat_xml, "ao");
		RHITexture2D* ao = create_texture_from_xml(ao_xml);
		set_ao_texture(ao);
	}

	// create and save spec
	if(xml::element_has_child(mat_xml, "height")){
		XMLElement height_xml = xml::get_element_child(mat_xml, "height");
		RHITexture2D* height = create_texture_from_xml(height_xml);
		set_height_texture(height);

		if(xml::element_has_attribute(height_xml, "scale")){
			m_height_map_scale = xml::parse_xml_attribute(height_xml, "scale", m_height_map_scale);
		}
	}

	// loop through other textures
	int num_children = xml::get_element_num_children(mat_xml);
	for(int child_index = 0; child_index < num_children; ++child_index){
		XMLElement child = xml::get_element_child_by_index(mat_xml, child_index);
		if(xml::get_element_name(child) == "texture"){
			xml::validate_xml_node(child, "", "name, index, src, color", "", "");

			RHITexture2D* texture = create_texture_from_xml(child);

			int texture_index = INVALID_BIND_INDEX;
			if(xml::element_has_attribute(child, "name")){
				std::string name = xml::parse_xml_attribute(child, "name", name);
				texture_index = m_shader->find_bind_index_for_name(name.c_str());
			}
			else if(xml::element_has_attribute(child, "index")){
				texture_index = xml::parse_xml_attribute(child, "index", texture_index);
			}

			if(texture_index == INVALID_BIND_INDEX){
				continue;
			}
			
			set_texture(texture_index, texture);
		}
	}

	s_database[mat_xml_file] = this;
}

Material::Material(Shader* shader)
	:m_shader(shader)
{
	memset(&m_textures[0], 0, sizeof(RHITexture2D*) * MAX_NUM_TEXTURES);
	memset(&m_constant_buffers[0], 0, sizeof(ConstantBuffer*) * MAX_NUM_CONSTANT_BUFFERS);
}

Material::Material(const Material& mat_to_copy)
	:m_shader(mat_to_copy.m_shader)
	,m_is_metal(mat_to_copy.m_is_metal)
	,m_smoothness(mat_to_copy.m_smoothness)
	,m_specular_reflectance(mat_to_copy.m_specular_reflectance)
	,m_diffuse_reflectance(mat_to_copy.m_diffuse_reflectance)
	,m_height_map_scale(mat_to_copy.m_height_map_scale)
{
	memcpy(&m_textures[0], &mat_to_copy.m_textures[0], sizeof(RHITexture2D*) * MAX_NUM_TEXTURES);
	memcpy(&m_constant_buffers[0], &mat_to_copy.m_constant_buffers[0], sizeof(ConstantBuffer*) * MAX_NUM_CONSTANT_BUFFERS);
}

Material::~Material()
{
    for(int i = 0; i < MAX_NUM_CONSTANT_BUFFERS; i++){
        SAFE_DELETE(m_constant_buffers[i]);
    }
}

MaterialInstance* Material::create_instance() const 
{ 
	return new MaterialInstance(*this); 
}

RHITexture2D* Material::get_texture(int index) const
{
	return m_textures[index];
}

RHITexture2D* Material::get_diffuse_texture() const { return get_texture(DIFFUSE_TEXTURE_INDEX); }
RHITexture2D* Material::get_albedo_texture() const { return get_texture(ALBEDO_TEXTURE_INDEX); }
RHITexture2D* Material::get_smoothness_texture() const { return get_texture(SMOOTHNESS_TEXTURE_INDEX); }
RHITexture2D* Material::get_normal_texture() const { return get_texture(NORMAL_TEXTURE_INDEX); }
RHITexture2D* Material::get_spec_texture() const { return get_texture(SPEC_TEXTURE_INDEX); }
RHITexture2D* Material::get_metalness_texture() const { return get_texture(METALNESS_TEXTURE_INDEX); }
RHITexture2D* Material::get_ao_texture() const { return get_texture(AO_TEXTURE_INDEX); }
RHITexture2D* Material::get_height_texture() const{ return get_texture(HEIGHT_TEXTURE_INDEX); }

float Material::get_smoothness() const { return m_smoothness; }
float Material::get_roughness() const { return 1.0f - get_smoothness(); }
Vector3 Material::get_spec_reflectance() const { return m_specular_reflectance; }
Vector3 Material::get_diffuse_reflectance() const { return m_diffuse_reflectance; }
bool Material::is_metal() const { return m_is_metal; }
bool Material::is_tesselated() const{ return  nullptr != m_shader->m_shader_program->m_hull_stage; }
float Material::get_height_map_scale() const{ return m_height_map_scale; }

ConstantBuffer* Material::get_constant_buffer(int index) const
{
	return m_constant_buffers[index];
}

void Material::set_texture(RHITexture2D* texture){ set_texture(0, texture); }
void Material::set_texture(int index, RHITexture2D* texture){ m_textures[index] = texture; }

void Material::set_diffuse_texture(RHITexture2D* diffuse){ set_texture(DIFFUSE_TEXTURE_INDEX, diffuse); }
void Material::set_albedo_texture(RHITexture2D* albedo){ set_texture(ALBEDO_TEXTURE_INDEX, albedo); }
void Material::set_smoothness_texture(RHITexture2D* smoothness){ set_texture(SMOOTHNESS_TEXTURE_INDEX, smoothness); }
void Material::set_normal_texture(RHITexture2D* normal){ set_texture(NORMAL_TEXTURE_INDEX, normal); }
void Material::set_spec_texture(RHITexture2D* spec){ set_texture(SPEC_TEXTURE_INDEX, spec); }
void Material::set_metalness_texture(RHITexture2D* metal){ set_texture(METALNESS_TEXTURE_INDEX, metal); }
void Material::set_ao_texture(RHITexture2D* ao){ set_texture(AO_TEXTURE_INDEX, ao); }
void Material::set_height_texture(RHITexture2D* height){ set_texture(HEIGHT_TEXTURE_INDEX, height); }

void Material::set_constant_buffer(ConstantBuffer* constant_buffer){ set_constant_buffer(0, constant_buffer); }
void Material::set_constant_buffer(int index, ConstantBuffer* constant_buffer){ m_constant_buffers[index] = constant_buffer; }

void Material::set_smoothness(float smoothness){ m_smoothness = smoothness; }
void Material::set_roughness(float roughness){ m_smoothness = 1.0f - roughness; }
void Material::set_specular_reflectance(const Vector3& spec){ m_specular_reflectance = spec; }
void Material::set_specular_reflectance(float r, float g, float b){ set_specular_reflectance(Vector3(r, g, b)); }
void Material::set_diffuse_reflectance(const Vector3& diffuse){ m_diffuse_reflectance = diffuse; }
void Material::set_diffuse_reflectance(float r, float g, float b){ set_diffuse_reflectance(Vector3(r, g, b)); }
void Material::set_metalness(bool metalness){ m_is_metal = metalness; }
void Material::set_height_map_scale(float height_map_scale){ m_height_map_scale = height_map_scale; }

void Material::set_metal_type(MatMetalType type)
{
	switch(type)
	{
		case MAT_TITANIUM:		set_diffuse_reflectance(0.542f, 0.497f, 0.449f); break;
		case MAT_CHROMIUM:		set_diffuse_reflectance(0.549f, 0.556f, 0.554f); break;
		case MAT_IRON:			set_diffuse_reflectance(0.562f, 0.565f, 0.578f); break;
		case MAT_NICKEL:		set_diffuse_reflectance(0.660f, 0.609f, 0.526f); break;
		case MAT_PLATINUM:		set_diffuse_reflectance(0.673f, 0.637f, 0.585f); break;
		case MAT_COPPER:		set_diffuse_reflectance(0.955f, 0.638f, 0.538f); break;
		case MAT_PALLADIUM:		set_diffuse_reflectance(0.733f, 0.697f, 0.652f); break;
		case MAT_GOLD:			set_diffuse_reflectance(1.0f, 0.782f, 0.344f); break;
		case MAT_ALUMINUM:		set_diffuse_reflectance(0.913f, 0.922f, 0.924f); break;
		case MAT_SILVER:		set_diffuse_reflectance(0.972f, 0.960f, 0.915f); break;
	}

	set_metalness(true);
}

#include "Engine/Profile/auto_profile_log_scope.h"

RHITexture2D* Material::create_texture_from_xml(const XMLElement& texture_xml)
{
	xml::validate_xml_node(texture_xml, "", "name, src, color, scale", "", "");

	if(xml::element_has_attribute(texture_xml, "color")){
		std::string color_string = xml::parse_xml_attribute(texture_xml, "color", color_string);
		Rgba color(color_string);
		return g_theRenderer->m_device->CreateRHITexture2DFromColor(color);
	}
	else{
		std::string texture_filename = xml::parse_xml_attribute(texture_xml, "src", texture_filename);
		return g_theRenderer->m_device->load_rhitexture2d_async(texture_filename);
	}
}


//----------------------------------------------------------------------
// Material Instance
//----------------------------------------------------------------------

MaterialInstance::MaterialInstance(const Material& mat)
	:Material(mat)
{
}

MaterialInstance::~MaterialInstance()
{
}

void MaterialInstance::set_texture(const char* bind_name, RHITexture2D* texture)
{
	int bind_index = m_shader->find_bind_index_for_name(bind_name);

	std::string error = Stringf("Error: Could not find a bind index for resource with name [%s]", bind_name);
	ASSERT_OR_DIE(bind_index != INVALID_BIND_INDEX, error.c_str());

	set_texture(bind_index, texture);
}