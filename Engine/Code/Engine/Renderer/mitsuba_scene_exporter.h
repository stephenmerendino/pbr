#pragma once

#include "Engine/Core/types.h"
#include "Engine/Core/xml.hpp"
#include <map>

#define EXPORT_PATH_NAME_LENGTH 64

class Scene;
class RenderableMesh;
class RHITexture2D;

class MitsubaSceneExporter
{
	public:
		const char* m_filename;
		Scene* m_scene;
		char m_export_full_path[EXPORT_PATH_NAME_LENGTH];
		char m_export_meshes_full_path[EXPORT_PATH_NAME_LENGTH];
		char m_export_textures_full_path[EXPORT_PATH_NAME_LENGTH];
		std::map<unsigned int, RenderableMesh*> m_index_mesh_mappings;
		std::map<RHITexture2D*, std::string> m_written_textures;

	public:
		MitsubaSceneExporter(const char* filename, Scene* scene);
		~MitsubaSceneExporter();

		bool export_to_file();
		bool create_export_folder();
		bool export_meshes();

		bool export_mitsuba_xml();
		void export_meshes_to_xml(XMLElement& root);
		void export_bsdf_to_xml(XMLElement& root, const RenderableMesh* rm, uint mesh_index);
		void export_lights_to_xml(XMLElement& root);
		void export_camera_to_xml(XMLElement& root);
		bool export_mesh_to_obj(RenderableMesh* rm, unsigned int index);
		void export_screenshot();

		bool has_written_texture(RHITexture2D* tex);
		const char* get_written_name_for_texture(RHITexture2D* tex);
		void save_texture_processed_info(RHITexture2D* normal_texture, const char* normal_filename);
};