#include "Engine/Renderer/mitsuba_scene_exporter.h"
#include "Engine/Renderer/scene.h"
#include "Engine/Renderer/camera.h"
#include "Engine/Renderer/RenderableMesh.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/PointLight.h"
#include "Engine/Renderer/DirectionalLight.h"
#include "Engine/Renderer/SpotLight.h"
#include "Engine/Config/EngineConfig.hpp"
#include "Engine/Core/log.h"
#include "Engine/Core/directory.h"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Console.hpp"
#include <ctime>

#define EXPORT_TIMESTAMP_FORMAT "%Y%m%d_%H%M%S"

MitsubaSceneExporter::MitsubaSceneExporter(const char* filename, Scene* scene)
	:m_filename(filename)
	,m_scene(scene)
{
}

MitsubaSceneExporter::~MitsubaSceneExporter()
{
}

bool MitsubaSceneExporter::export_to_file()
{
	console_hide();

	// create timestamped export folder
	if(!create_export_folder()){
		return false;
	}

	// output .obj file for each mesh
	if(!export_meshes()){
		return false;
	}

	// output xml file for scene
	export_mitsuba_xml();

	export_screenshot();

	return true;
}

bool MitsubaSceneExporter::create_export_folder()
{
	std::time_t raw_time = std::time(nullptr);
	tm export_time;
	localtime_s(&export_time, &raw_time);

	char timestring[32];
	strftime(timestring, 32, EXPORT_TIMESTAMP_FORMAT, &export_time);

	char folder[32];
	sprintf_s(folder, 32, "%s_export_%s", m_filename, timestring);

	sprintf_s((char * const)m_export_full_path, EXPORT_PATH_NAME_LENGTH, "%s/%s/", DEFAULT_SCENE_MIT_EXPORT_DIR, folder);

	bool did_create = create_directory(m_export_full_path);
	if(!did_create){
		return false;
	}

	sprintf_s((char * const)m_export_meshes_full_path, EXPORT_PATH_NAME_LENGTH, "%s/%s/", m_export_full_path, "meshes/");
	sprintf_s((char * const)m_export_textures_full_path, EXPORT_PATH_NAME_LENGTH, "%s/%s/", m_export_full_path, "textures/");

	create_directory(m_export_meshes_full_path);
	create_directory(m_export_textures_full_path);

	return true;
}

bool MitsubaSceneExporter::export_meshes()
{
	for(unsigned int i = 0; i < m_scene->m_renderable_meshes.size(); ++i){
		RenderableMesh* m = m_scene->m_renderable_meshes[i];
		if(nullptr == m){
			continue;
		}

		if(!export_mesh_to_obj(m, i)){
			return false;
		}
	}

	return true;
}

bool MitsubaSceneExporter::export_mesh_to_obj(RenderableMesh* rm, unsigned int index)
{
	if(nullptr == rm){
		return false;
	}

	// store off a mapping of index to renderable mesh, this is used later for outputing other scene files
	m_index_mesh_mappings[index] = rm;

	// build obj filename
	std::string obj_filename = m_export_meshes_full_path;
	char mesh_filename[25];
	sprintf_s(mesh_filename, 25, "mesh%i.obj", index);
	obj_filename += mesh_filename;

	// open obj file
	FILE* obj_file;
	fopen_s(&obj_file, obj_filename.c_str(), "w");

	char* header_comment = "# This is an auto generated .obj file, do not edit\ng auto_gen\n";
	fwrite(header_comment, 1, strlen(header_comment), obj_file);

	// write out obj data

	// need to write out vertexes
	// v x y z

	for(unsigned int i = 0; i < rm->m_mesh->m_vertexes.size(); ++i){
		Vertex3 vert = rm->m_mesh->m_vertexes[i];
		vert.m_position.x *= -1.0f;
		char vout[64];
		sprintf_s(vout, 64, "v %f %f %f\n", vert.m_position.x, vert.m_position.y, vert.m_position.z);
		fwrite(vout, 1, strlen(vout), obj_file);
	}

	// need to write out texture coords
	// vt u v

	for(unsigned int i = 0; i < rm->m_mesh->m_vertexes.size(); ++i){
		Vertex3 vert = rm->m_mesh->m_vertexes[i];
		char vout[64];
		sprintf_s(vout, 64, "vt %f %f\n", vert.m_texCoords.x, vert.m_texCoords.y);
		fwrite(vout, 1, strlen(vout), obj_file);
	}

	// need to write out vertex normals
	// vn x y z

	for(unsigned int i = 0; i < rm->m_mesh->m_vertexes.size(); ++i){
		Vertex3 vert = rm->m_mesh->m_vertexes[i];
		vert.m_normal.x *= -1.0f;
		char vout[64];
		sprintf_s(vout, 64, "vn %f %f %f\n", vert.m_normal.x, vert.m_normal.y, vert.m_normal.z);
		fwrite(vout, 1, strlen(vout), obj_file);
	}

	// need to define faces
	// f v/vt/vn v/vt/vn v/vt/vn

	uint num_indexes = (uint)rm->m_mesh->m_indexes.size();

	if(num_indexes > 0){
		// use indexes to build faces
		for(unsigned int i = 0; i < num_indexes; i++){
			unsigned int v1 = rm->m_mesh->m_indexes[i++] + 1;
			unsigned int v2 = rm->m_mesh->m_indexes[i++] + 1;
			unsigned int v3 = rm->m_mesh->m_indexes[i] + 1;

			char vout[64];
			sprintf_s(vout, 64, "f %i/%i/%i %i/%i/%i %i/%i/%i\n", v1, v1, v1, v2, v2, v2, v3, v3, v3);
			fwrite(vout, 1, strlen(vout), obj_file);
		}
	} else{
		// assume the vertexes are in good order
		for(unsigned int i = 0; i < rm->m_mesh->m_vertexes.size(); i++){
			unsigned int v1 = i + 1;
			i++;

			unsigned int v2 = i + 1;
			i++;

			unsigned int v3 = i + 1;

			char vout[64];
			sprintf_s(vout, 64, "f %i/%i/%i %i/%i/%i %i/%i/%i\n", v1, v1, v1, v2, v2, v2, v3, v3, v3);
			fwrite(vout, 1, strlen(vout), obj_file);
		}
	}

	// close obj file
	fclose(obj_file);

	return true;
}

bool MitsubaSceneExporter::export_mitsuba_xml()
{
	// create root xml element
	XMLElement root = xml::create_root_element("scene");
	xml::set_attribute(root, "version", "0.5.0");

	// export the scene to xml
	export_meshes_to_xml(root);
	export_lights_to_xml(root);
	export_camera_to_xml(root);

	// save xml to file
	char mitsuba_xml_filename[96];
	sprintf_s(mitsuba_xml_filename, 96, "%s/%s_scene.xml", m_export_full_path, m_filename);
	xml::save_root_to_file(root, mitsuba_xml_filename);

	return true;
}

void MitsubaSceneExporter::export_meshes_to_xml(XMLElement& root)
{
	// output each renderable mesh
	std::map<unsigned int, RenderableMesh*>::iterator it;
	for(it = m_index_mesh_mappings.begin(); it != m_index_mesh_mappings.end(); ++it){
		unsigned int index = it->first;
		RenderableMesh* rm = it->second;
	
		// output shape
		XMLElement shape = xml::add_child(root, "shape");
		xml::set_attribute(shape, "type", "obj");

		XMLElement shape_file = xml::add_child(shape, "string");
		xml::set_attribute(shape_file, "name", "filename");

		char mesh_filename[32];
		sprintf_s(mesh_filename, 32, "meshes/mesh%i.obj", index);
		xml::set_attribute(shape_file, "value", mesh_filename);

		XMLElement transform = xml::add_child(shape, "transform");
		xml::set_attribute(transform, "name", "toWorld");

		XMLElement matrix = xml::add_child(transform, "matrix");

		Matrix4 t = rm->m_transform.calc_world_matrix();
		char transform_string[256];
		sprintf_s(transform_string, 256, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", t.data[0], -t.data[1], -t.data[2], -t.data[3],
			                                                                                -t.data[4], t.data[5], t.data[6], t.data[7],
			                                                                                -t.data[8], t.data[9], t.data[10], t.data[11],
			                                                                                t.data[12], t.data[13], t.data[14], t.data[15]);

		xml::set_attribute(matrix, "value", transform_string);

		export_bsdf_to_xml(shape, rm, index);
	}
}

bool MitsubaSceneExporter::has_written_texture(RHITexture2D* tex)
{
	std::map<RHITexture2D*, std::string>::iterator found = m_written_textures.find(tex);
	return found != m_written_textures.end();
}

const char* MitsubaSceneExporter::get_written_name_for_texture(RHITexture2D* tex)
{
	return m_written_textures[tex].c_str();
}

void MitsubaSceneExporter::save_texture_processed_info(RHITexture2D* normal_texture, const char* normal_filename)
{
	m_written_textures[normal_texture] = normal_filename;
}

void MitsubaSceneExporter::export_bsdf_to_xml(XMLElement& root, const RenderableMesh* rm, uint mesh_index)
{
	// set the normal map as the root if it has one
	RHITexture2D* normal_texture = rm->m_materials[0]->get_normal_texture();
	if(nullptr != normal_texture){

		char normal_filename[128];
		if(has_written_texture(normal_texture)){
			const char* existing_filename = get_written_name_for_texture(normal_texture);
			strcpy_s(normal_filename, existing_filename);
		} else{
			sprintf_s(normal_filename, 128, "%snormal%i.png", m_export_textures_full_path, mesh_index);
			g_theRenderer->save_texture2d_to_file(normal_texture, normal_filename);
			sprintf_s(normal_filename, 128, "textures/normal%i.png", mesh_index);
			save_texture_processed_info(normal_texture, normal_filename);
		}

		XMLElement bumpmap_root_bsdf = xml::add_child(root, "bsdf");
		xml::set_attribute(bumpmap_root_bsdf, "type", "bumpmap");

		XMLElement normal = xml::add_child(bumpmap_root_bsdf, "texture");
		xml::set_attribute(normal, "type", "bitmap");

		XMLElement normal_file = xml::add_child(normal, "string");
		xml::set_attribute(normal_file, "name", "filename");

		xml::set_attribute(normal_file, "value", normal_filename);

		root = bumpmap_root_bsdf;
	}

	XMLElement bsdf = xml::add_child(root, "bsdf");

	// set bsdf type
	// #TODO: This type should set based on the type of material rm is?
	xml::set_attribute(bsdf, "type", "roughplastic");

	// set distribution type
	XMLElement dist = xml::add_child(bsdf, "string");
	xml::set_attribute(dist, "name", "distribution");
	xml::set_attribute(dist, "value", "ggx");

	// set the roughness (for now a constant float, but later a texture)
	XMLElement alpha = xml::add_child(bsdf, "float");
	xml::set_attribute(alpha, "name", "alpha");

	float roughness = rm->m_materials[0]->get_roughness();
	xml::set_attribute(alpha, "value", Stringf("%f", roughness).c_str());

	XMLElement diffuse = xml::add_child(bsdf, "spectrum");
	xml::set_attribute(diffuse, "name", "diffuseReflectance");
	Vector3 diff = rm->m_materials[0]->get_diffuse_reflectance();
	xml::set_attribute(diffuse, "value", Stringf("%f, %f, %f", diff.x, diff.y, diff.z).c_str());

	// set the diffuse texture if it has one
	RHITexture2D* diffuse_texture = rm->m_materials[0]->get_diffuse_texture();
	if(nullptr != diffuse_texture){
		char diffuse_filename[128];
		if(has_written_texture(diffuse_texture)){
			const char* existing_filename = get_written_name_for_texture(diffuse_texture);
			strcpy_s(diffuse_filename, existing_filename);
		} else{
			sprintf_s(diffuse_filename, 128, "%sdiffuse%i.png", m_export_textures_full_path, mesh_index);
			g_theRenderer->save_texture2d_to_file(diffuse_texture, diffuse_filename);

			sprintf_s(diffuse_filename, 128, "textures/diffuse%i.png", mesh_index);
			save_texture_processed_info(diffuse_texture, diffuse_filename);
		}

		XMLElement diffuseTexture = xml::add_child(bsdf, "texture");
		xml::set_attribute(diffuseTexture, "name", "diffuseReflectance");
		xml::set_attribute(diffuseTexture, "type", "bitmap");

		XMLElement diffuse_file = xml::add_child(diffuseTexture, "string");
		xml::set_attribute(diffuse_file, "name", "filename");

		xml::set_attribute(diffuse_file, "value", diffuse_filename);
	}

	// set the spec texture if it has one
	RHITexture2D* spec_texture = rm->m_materials[0]->get_spec_texture();
	if(nullptr != spec_texture){
		char spec_filename[128];
		if(has_written_texture(spec_texture)){
			const char* existing_filename = get_written_name_for_texture(spec_texture);
			strcpy_s(spec_filename, existing_filename);
		} else{
			sprintf_s(spec_filename, 128, "%sspec%i.png", m_export_textures_full_path, mesh_index);
			g_theRenderer->save_texture2d_to_file(spec_texture, spec_filename);
			sprintf_s(spec_filename, 128, "textures/spec%i.png", mesh_index);
			save_texture_processed_info(spec_texture, spec_filename);
		}

		XMLElement spec = xml::add_child(bsdf, "texture");
		xml::set_attribute(spec, "name", "specularReflectance");
		xml::set_attribute(spec, "type", "bitmap");

		XMLElement spec_file = xml::add_child(spec, "string");
		xml::set_attribute(spec_file, "name", "filename");

		xml::set_attribute(spec_file, "value", spec_filename);
	}
}

void MitsubaSceneExporter::export_lights_to_xml(XMLElement& root)
{
	// write out each point light
	std::vector<PointLight*>::iterator pit;
	for(pit = m_scene->m_point_lights.begin(); pit != m_scene->m_point_lights.end(); ++pit){
		const PointLight* pl = *pit;

		XMLElement emitter = xml::add_child(root, "emitter");
		xml::set_attribute(emitter, "type", "point");

		XMLElement spectrum = xml::add_child(emitter, "spectrum");
		xml::set_attribute(spectrum, "name", "intensity");

		Vector4 color_norm = pl->m_color.GetAsVector4Normalized();
		float r = color_norm.x;
		float g = color_norm.y;
		float b = color_norm.z;
		float w = color_norm.w /* / (4.0f * M_PI) */;
		xml::set_attribute(spectrum, "value", Stringf("%f, %f, %f", r * w, g * w, b * w).c_str());
		//xml::set_attribute(spectrum, "value", Stringf("%f", w).c_str());
		
		XMLElement position = xml::add_child(emitter, "point");
		xml::set_attribute(position, "name", "position");
		xml::set_attribute(position, "x", Stringf("%f", -pl->m_position.x).c_str());
		xml::set_attribute(position, "y", Stringf("%f", pl->m_position.y).c_str());
		xml::set_attribute(position, "z", Stringf("%f", pl->m_position.z).c_str());
	}

	std::vector<SpotLight*>::iterator lit;
	for(lit = m_scene->m_spot_lights.begin(); lit != m_scene->m_spot_lights.end(); ++lit){
		const SpotLight* sl = *lit;

		XMLElement emitter = xml::add_child(root, "emitter");
		xml::set_attribute(emitter, "type", "spot");

		XMLElement spectrum = xml::add_child(emitter, "spectrum");
		xml::set_attribute(spectrum, "name", "intensity");

		Vector4 color_norm = sl->m_color.GetAsVector4Normalized();
		float r = color_norm.x;
		float g = color_norm.y;
		float b = color_norm.z;
		float w = color_norm.w;
		xml::set_attribute(spectrum, "value", Stringf("%f, %f, %f", r * w, g * w, b * w).c_str());
		//xml::set_attribute(spectrum, "value", Stringf("%f", w).c_str());
		
		// setup transform
		XMLElement transform = xml::add_child(emitter, "transform");
		xml::set_attribute(transform, "name", "toWorld");

		XMLElement look_at = xml::add_child(transform, "lookat");

		Vector3 p = sl->m_position;
		p.x *= -1.0f;

		Vector3 dir = sl->m_direction;
		dir.x *= -1.0f;

		Vector3 t = p + dir;

		xml::set_attribute(look_at, "origin", Stringf("%f, %f, %f", p.x, p.y, p.z).c_str());
		xml::set_attribute(look_at, "target", Stringf("%f, %f, %f", t.x, t.y, t.z).c_str());

		// outer angle
		XMLElement cutoff_angle = xml::add_child(emitter, "float");
		xml::set_attribute(cutoff_angle, "name", "cutoffAngle");
		xml::set_attribute(cutoff_angle, "value", Stringf("%f", sl->m_outer_angle).c_str());

		// inner angle
		XMLElement beam_width = xml::add_child(emitter, "float");
		xml::set_attribute(beam_width, "name", "beamWidth");
		xml::set_attribute(beam_width, "value", Stringf("%f", sl->m_inner_angle).c_str());
	}

	// write out each directional light
	std::vector<DirectionalLight*>::iterator dit;
	for(dit = m_scene->m_directional_lights.begin(); dit != m_scene->m_directional_lights.end(); ++dit){
		const DirectionalLight* dl = *dit;

		XMLElement emitter = xml::add_child(root, "emitter");
		xml::set_attribute(emitter, "type", "directional");

		XMLElement spectrum = xml::add_child(emitter, "spectrum");
		xml::set_attribute(spectrum, "name", "irradiance");
		xml::set_attribute(spectrum, "value", Stringf("%f", dl->m_power).c_str());
		
		XMLElement position = xml::add_child(emitter, "vector");
		xml::set_attribute(position, "name", "direction");
		xml::set_attribute(position, "x", Stringf("%f", dl->m_direction.x).c_str());
		xml::set_attribute(position, "y", Stringf("%f", dl->m_direction.y).c_str());
		xml::set_attribute(position, "z", Stringf("%f", dl->m_direction.z).c_str());
	}
}

void MitsubaSceneExporter::export_camera_to_xml(XMLElement& root)
{
	Camera* c = m_scene->m_camera;

	XMLElement sensor = xml::add_child(root, "sensor");
	xml::set_attribute(sensor, "type", "perspective");

	XMLElement far_clip = xml::add_child(sensor, "float");
	xml::set_attribute(far_clip, "name", "farClip");
	xml::set_attribute(far_clip, "value", Stringf("%f", c->m_fz).c_str());

	XMLElement near_clip = xml::add_child(sensor, "float");
	xml::set_attribute(near_clip, "name", "nearClip");
	xml::set_attribute(near_clip, "value", Stringf("%f", c->m_nz).c_str());

	XMLElement fov = xml::add_child(sensor, "float");
	xml::set_attribute(fov, "name", "fov");
	xml::set_attribute(fov, "value", Stringf("%f", c->m_fov).c_str());

	XMLElement fovAxis = xml::add_child(sensor, "string");
	xml::set_attribute(fovAxis, "name", "fovAxis");
	xml::set_attribute(fovAxis, "value", "y");

	XMLElement transform = xml::add_child(sensor, "transform");
	xml::set_attribute(transform, "name", "toWorld");

	XMLElement matrix = xml::add_child(transform, "matrix");

	Matrix4 t = c->m_transform.calc_world_matrix();
	char transform_string[256];
		sprintf_s(transform_string, 256, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", t.data[0], -t.data[1], -t.data[2], -t.data[3],
			                                                                                -t.data[4], t.data[5], t.data[6], t.data[7],
			                                                                                -t.data[8], t.data[9], t.data[10], t.data[11],
			                                                                                t.data[12], t.data[13], t.data[14], t.data[15]);
	xml::set_attribute(matrix, "value", transform_string);

	XMLElement sampler = xml::add_child(sensor, "sampler");
	xml::set_attribute(sampler, "type", "ldsampler");

	XMLElement sample_count = xml::add_child(sampler, "integer");
	xml::set_attribute(sample_count, "name", "sampleCount");
	xml::set_attribute(sample_count, "value", "64");

	XMLElement output_film = xml::add_child(sensor, "film");
	xml::set_attribute(output_film, "type", "hdrfilm");

	XMLElement height = xml::add_child(output_film, "integer");
	xml::set_attribute(height, "name", "height");
	xml::set_attribute(height, "value", "1080");

	XMLElement width = xml::add_child(output_film, "integer");
	xml::set_attribute(width, "name", "width");
	xml::set_attribute(width, "value", "1920");

	XMLElement filter = xml::add_child(output_film, "rfilter");
	xml::set_attribute(filter, "type", "gaussian");
}

void MitsubaSceneExporter::export_screenshot()
{
	std::string screenshot_path(m_export_full_path);
	screenshot_path += "in_engine_reference.png";
	g_theRenderer->request_screenshot(screenshot_path.c_str());
}