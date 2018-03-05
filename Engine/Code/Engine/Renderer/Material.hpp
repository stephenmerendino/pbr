#pragma once

#include "Engine/RHI/Shader.hpp"
#include "Engine/Core/xml.hpp"

#include <map>
#include <string>

#define MAX_NUM_TEXTURES D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT
#define MAX_NUM_CONSTANT_BUFFERS D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT

#define ALBEDO_TEXTURE_INDEX 0
#define DIFFUSE_TEXTURE_INDEX 0
#define SMOOTHNESS_TEXTURE_INDEX 1
#define NORMAL_TEXTURE_INDEX 2
#define SPEC_TEXTURE_INDEX 3
#define METALNESS_TEXTURE_INDEX 4
#define AO_TEXTURE_INDEX 5
#define HEIGHT_TEXTURE_INDEX 6

enum MatMetalType
{
	MAT_TITANIUM,
	MAT_CHROMIUM,
	MAT_IRON,
	MAT_NICKEL,
	MAT_PLATINUM,
	MAT_COPPER,
	MAT_PALLADIUM,
	MAT_GOLD,
	MAT_ALUMINUM,
	MAT_SILVER,
	NUM_MAT_METAL_TYPES
};

class MaterialInstance;

class Material
{
friend class SimpleRenderer;

public:
    static void init();
	static void shutdown();
	static const Material* find_or_create(const char* mat_xml_file);
    static bool validate_xml(const char* xml_filename);

	static std::map<std::string, Material*> s_database;
    static const Material* DEBUG_MATERIAL;

public:
	Material(Shader* shader);
	Material(const Material& mat_to_copy);
	virtual ~Material();

	MaterialInstance*	create_instance() const;

	RHITexture2D*		get_texture(int index) const;
	RHITexture2D*		get_diffuse_texture() const;
	RHITexture2D*		get_albedo_texture() const;
	RHITexture2D*		get_smoothness_texture() const;
	RHITexture2D*		get_normal_texture() const;
	RHITexture2D*		get_spec_texture() const;
	RHITexture2D*		get_metalness_texture() const;
	RHITexture2D*		get_ao_texture() const;
	RHITexture2D*		get_height_texture() const;

	float				get_smoothness() const;
	float				get_roughness() const;
	Vector3				get_spec_reflectance() const;
	Vector3				get_diffuse_reflectance() const;
	bool				is_metal() const;
	bool				is_tesselated() const;
	float				get_height_map_scale() const;

	ConstantBuffer*		get_constant_buffer(int index) const;

protected:
	Shader*				m_shader;

	RHITexture2D*		m_textures[MAX_NUM_TEXTURES];
	ConstantBuffer*		m_constant_buffers[MAX_NUM_CONSTANT_BUFFERS];

	bool				m_is_metal;
	float				m_smoothness;
	Vector3				m_specular_reflectance;
	Vector3				m_diffuse_reflectance;
	float				m_height_map_scale;

	void set_texture(RHITexture2D* texture);
	void set_texture(int index, RHITexture2D* texture);

	void set_diffuse_texture(RHITexture2D* diffuse);
	void set_albedo_texture(RHITexture2D* albedo);
	void set_smoothness_texture(RHITexture2D* smoothness);
	void set_normal_texture(RHITexture2D* normal);
	void set_spec_texture(RHITexture2D* spec);
	void set_metalness_texture(RHITexture2D* metal);
	void set_ao_texture(RHITexture2D* ao);
	void set_height_texture(RHITexture2D* height);

	void set_constant_buffer(ConstantBuffer* constant_buffer);
	void set_constant_buffer(int index, ConstantBuffer* constant_buffer);

	void set_smoothness(float smoothness);
	void set_roughness(float roughness);
	void set_specular_reflectance(const Vector3& spec);
	void set_specular_reflectance(float r, float g, float b);
	void set_diffuse_reflectance(const Vector3& diffuse);
	void set_diffuse_reflectance(float r, float g, float b);
	void set_metalness(bool metalness);
	void set_metal_type(MatMetalType type);
	void set_height_map_scale(float height_map_scale);

private:
	Material(const char* mat_xml_file);

	RHITexture2D* create_texture_from_xml(const XMLElement& texture_xml);
};

class MaterialInstance : public Material
{
public:
	MaterialInstance(const Material& mat);
	~MaterialInstance();

	using Material::set_texture;
	using Material::set_diffuse_texture;
	using Material::set_albedo_texture;
	using Material::set_normal_texture;
	using Material::set_spec_texture;
	using Material::set_metalness_texture;
	using Material::set_ao_texture;
	using Material::set_height_texture;
	using Material::set_constant_buffer;
	using Material::set_smoothness;
	using Material::set_roughness;
	using Material::set_specular_reflectance;
	using Material::set_diffuse_reflectance;
	using Material::set_metalness;
	using Material::set_metal_type;
	using Material::set_height_map_scale;

	void set_texture(const char* bind_name, RHITexture2D* texture);
};