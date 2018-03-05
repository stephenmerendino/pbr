#pragma once
#pragma warning(disable: 4201)

#include "Engine/Core/types.h"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/camera.h"

class RHIDevice;
class RHITexture2D;
class Image;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

enum CubeMapFaceOrder : uint
{
	CUBE_FACE_POS_X, 
	CUBE_FACE_NEG_X,
	CUBE_FACE_POS_Y, 
	CUBE_FACE_NEG_Y,
	CUBE_FACE_POS_Z, 
	CUBE_FACE_NEG_Z,
	NUM_CUBE_FACES
};

union CubeMapImagePaths
{
	struct{
		const char* posX;
		const char* negX;
		const char* posY;
		const char* negY;
		const char* posZ;
		const char* negZ;
	};
	const char* imagePaths[6];
};

class CubeMap;

typedef void(*loaded_cb)(CubeMap* loaded_cm, void* args);

class CubeMap
{
public:
	uint m_resolution;
	ImageFormat m_format;
	bool m_uses_mips;

	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_srv;
	ID3D11RenderTargetView** m_rtvs;
	ID3D11DepthStencilView** m_dsvs;

	bool m_is_loaded;
	loaded_cb m_listener_cb;
	void* m_listener_args;

public:
	CubeMap();
	CubeMap(uint resolution, ImageFormat format, bool setup_mips = false);

	~CubeMap();

	bool is_srv();
	bool is_rtv();
	bool is_dsv();

	void create_base_resource();
	void create_views();

	void on_loaded(loaded_cb cb, void* args);
	void finished_loading();

	// only creates base resource and srv
	bool load_from_seperate_images(RHIDevice* device, CubeMapImagePaths cubeMapImages);
	bool load_from_seperate_images_async(RHIDevice* device, CubeMapImagePaths cubeMapImages);
	bool save_to_seperate_images(const char* filename);

	void render_setup_face(uint face, const Vector3& view_center = Vector3::ZERO, uint mip_level = 0);
	void render_setup_pos_x(const Vector3& view_center = Vector3::ZERO, uint mip_level = 0);
	void render_setup_neg_x(const Vector3& view_center = Vector3::ZERO, uint mip_level = 0);
	void render_setup_pos_y(const Vector3& view_center = Vector3::ZERO, uint mip_level = 0);
	void render_setup_neg_y(const Vector3& view_center = Vector3::ZERO, uint mip_level = 0);
	void render_setup_pos_z(const Vector3& view_center = Vector3::ZERO, uint mip_level = 0);
	void render_setup_neg_z(const Vector3& view_center = Vector3::ZERO, uint mip_level = 0);

	Camera configure_cubemap_camera();
	uint calculate_mip_res(uint mip_level);
	uint get_num_mip_levels() const;
	uint get_num_subresources() const;
	uint get_subresource_index(uint face_index, uint mip_level) const;
};