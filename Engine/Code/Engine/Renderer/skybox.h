#pragma once

#include "Engine/Renderer/CubeMap.hpp"

class ConstantBuffer;

struct spec_mip_info_data_t
{
	uint current_mip_level;
	uint total_mip_levels;
	uint zero_mip_width;
	float _padding;
};

class SkyBox
{
	public:
		CubeMap* m_display_texture;

		CubeMap* m_diffuse_integrated_target;
		CubeMap* m_diffuse_depth_target;

		CubeMap* m_spec_integrated_target;
		CubeMap** m_spec_depth_targets;

		spec_mip_info_data_t m_spec_mip_data;
		ConstantBuffer* m_spec_mip_buffer;

	public:
		SkyBox();
		~SkyBox();

	bool load_from_face_images(RHIDevice* device, CubeMapImagePaths cubeMapImages);

	void bake_lighting(uint diffuse_res, uint spec_res);
	void integrate_diffuse(uint diffuse_res);
	void integrate_spec(uint spec_res);

	void create_spec_mips_cb();
};