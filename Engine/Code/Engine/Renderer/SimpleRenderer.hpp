#pragma once

#include "Engine/Core/types.h"
#include "Engine/Core/Rgba.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/Renderer/CubeMap.hpp"
#include "Engine/Renderer/SkeletalTransformHierarchy.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/transform.h"
#include "Engine/Renderer/RenderableMesh.hpp"
#include "Engine/Renderer/DirectionalLight.h"
#include "Engine/Renderer/SpotLight.h"
#include "Engine/Renderer/PointLight.h"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

class RHIDevice;
class RHIDeviceContext;
class RHIOutput;
class RHITextureBase;
class RHITexture2D;
class ShaderProgram;
class Shader;
class Sampler;
class VertexBuffer;
class IndexBuffer;
class Window;
class RasterState;
class BlendState;
class ConstantBuffer;
class StructuredBuffer;
class Vertex3;
class Font;
class Material;
class Mesh;
class Skeleton;
class SkeletonInstance;
class ComputeJob;
class Scene;
class Camera;
class SkyBox;


#define MATRIX_BUFFER_INDEX (0)
#define TIME_BUFFER_INDEX (1)
#define LIGHT_BUFFER_INDEX (2)
#define MATERIAL_BUFFER_INDEX (3)
#define MIDI_BUFFER_INDEX (4)
#define AA_BUFFER_INDEX (5)
#define TESSELATION_BUFFER_INDEX (6)

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

// --------------------------------------------------------------------
// Institutionalized Constant Buffers that are used in every shader
// --------------------------------------------------------------------

// --------------------------------------------------------------------
struct TimeBufferData
{
	float gameTime;
	float systemTime;
	float gameFrameTime;
	float systemFrameTime;
};

// --------------------------------------------------------------------
struct MatrixBufferData
{
	Matrix4 model;
	Matrix4 view;
	Matrix4 projection;
};

// --------------------------------------------------------------------
struct SpecularSurface
{
	float spec_power;
	bool use_texture;
	char _padding[11];
};

struct LightBufferData
{
	Vector4 cameraEyePosition;
	
	Vector4 ambient;
	directional_light_gpu_data_t directionalLights[MAX_DIRECTIONAL_LIGHTS];
	point_light_gpu_data_t pointLights[MAX_POINT_LIGHTS];
	spot_light_gpu_data_t spotLights[MAX_SPOT_LIGHTS];
	SpecularSurface specularSurface;
	bool environmentMapEnabled;
	float _padding[3];
};

struct MidiData
{
	float knobs[9];
	float sliders[9];
	float buttons[9];

	float _padding[1];
};

struct AAData
{
	int ssaa_level;
	float _padding[3];
};

#define MAT_USE_ALBEDO_TEXTURE 1	
#define MAT_USE_SMOOTHNESS_TEXTURE 2	
#define MAT_USE_NORMAL_TEXTURE 4	
#define MAT_USE_REFLECTANCE_TEXTURE 8 
#define MAT_USE_METALNESS_TEXTURE 16
#define MAT_USE_AO_TEXTURE 32
#define MAT_IS_METAL 64

struct MaterialData
{
	Vector4 albedo;
	Vector4 reflectance;
	float smoothness;
	uint flags;
	float height_map_scale;
	float _padding;
};

// --------------------------------------------------------------------
class SimpleRenderer
{
public:
	RHIDevice*			m_device;
	RHIDeviceContext*	m_deviceContext;
	RHIOutput*			m_output;

    RHITextureBase*     m_currentTarget;

	Mesh*				m_global_mesh;
	VertexBuffer*		m_global_vbo;
	IndexBuffer*		m_global_ibo;

	Shader*				m_current_shader;
	const Material*	    m_current_material;

	RHITexture2D*		m_defaultDepthStencil;
	RHITextureBase*		m_currentDepthStencil;

	DepthStencilDesc	m_currentDepthDesc;
	DepthStencilState*	m_currentDepthStencilState;

	BlendState*			m_currentBlendState;
	BlendStateDesc		m_currentBlendStateDesc;

	Sampler*			m_linearSampler;
	Sampler*			m_pointSampler;
	Sampler*			m_comparisonSampler;

	ConstantBuffer*		m_matrixBuffer;
	ConstantBuffer*		m_timeBuffer;
	ConstantBuffer*		m_lightBuffer;
	ConstantBuffer*		m_materialBuffer;
	ConstantBuffer*		m_midiBuffer;
	ConstantBuffer*		m_aaBuffer;
	ConstantBuffer*		m_tessBuffer;

	TimeBufferData		m_timeBufferData;
	MatrixBufferData	m_matrixBufferData;
	LightBufferData		m_lightBufferData;
	MaterialData		m_materialBufferData;
	MidiData			m_midiBufferData;
	AAData				m_aaBufferData;

    Scene*              m_current_scene;
	Rgba				m_clear_color;

	bool				m_wireframe_enabled;

	std::vector<std::string> m_pending_screenshots;

	static RasterState*			DEFAULT_RASTER_STATE;
	static BlendState*			DEFAULT_BLEND_STATE;
	static DepthStencilState*	DEFAULT_DEPTH_STENCIL_STATE;
	static RHITexture2D*		WHITE_TEXTURE;

public:
	RasterState*		m_solid_rasterstate;
	RasterState*		m_wireframe_rasterstate;

	RHITexture2D*		m_defaultTexture;
	ShaderProgram*		m_defaultShader;

	ShaderProgram*      m_ssaa_shader;

	RHITexture2D*		m_dfg_texture;

	RHITexture2D*		m_ssaa_render_target;
	RHITexture2D*		m_ssaa_depth_stencil;

	RHITexture2D*		m_fxaa_render_target;
	Shader*				m_fxaa_shader;

	RHITexture2D*		m_msaa_render_target;

	RHITexture2D*		m_default_render_target;

public:
	RasterState*		m_noCullRasterState;
	ShaderProgram*		m_skyboxShader;
	CubeMap*			m_currentReflectionMap;

public:
	SimpleRenderer();
	~SimpleRenderer();

	// ---------------------------------
	// Lifecycle Methods
	// ---------------------------------
	// void Setup( RHIOutput *output );
	void Setup(unsigned int width, unsigned int height);
	// void Setup( Window *window );
	// void Setup( Texture2D *default_render_target );
	void create_default_render_target();
	void create_aa_render_targets();

	void Update(float deltaSeconds);

	void Destroy();



	// ---------------------------------
	// Display Functionality
	// ---------------------------------
	void SetDisplaySize(int width, int height);
	void SetDisplayMode(const RHIOutputMode rhiDisplayMode);
	float get_aspect_ratio() const;


	// ---------------------------------
	// RenderTargets, Clear
	// ---------------------------------
	void SetColorTarget(RHITextureBase* colorTarget = nullptr, RHITextureBase* depthStencilTarget = nullptr, bool use_default_depth_target = true);
	void SetColorTarget(CubeMap* color_target, CubeMap* depth_target, uint face_index, uint mip_level = 0);
	void SetColorTargets(RHITextureBase** colorTargets, int numColorTargets, RHITextureBase* depthStencilTarget);
	void ClearColor(const Rgba& color) const;
	void ClearTargetColor(RHITextureBase* target, const Rgba& color) const;
	void ClearDepth(float depth = 1.0f, uint8_t stencil = 0U);
	void draw_fullscreen(RHITexture2D* texture_to_draw, RHITexture2D* render_target = nullptr, RHITexture2D* depth_target = nullptr);



	// ---------------------------------
	// Movel, View, Projection Matrixes
	// ---------------------------------
	void set_model(const Transform& transform);
    void set_view(const Transform& transform);

	void SetModel(const Matrix4& model);
	void SetView(const Matrix4& view);

	void SetOrthoProjection(float minX, float maxX, float minY, float maxY, float minZ = 0.0f, float maxZ = 1.0f);
	void SetOrthoProjection(const Vector2& min, const Vector2& max);
	void SetOrthoProjection(const Vector3& min, const Vector3& max);
	void SetOrthoProjection(const AABB2& bounds);
	void SetAspectNormalizedOrtho();
	void SetPerspectiveProjection(float nz, float fz, float viewingAngleDegrees, float aspect);
	void SetProjection(const Matrix4& projection);


	// ---------------------------------
	// AA
	// ---------------------------------
	void set_ssaa(int ssaa_level);


	// ---------------------------------
	// Lighting
	// ---------------------------------
	void SetEyePosition(const Vector3& eye);
	void SetAmbient(float intensity = 1.0f, const Rgba& color = Rgba::WHITE);

	void SetDirectionalLight(unsigned int index, const Vector3& direction, float power, const Rgba& color);
	void DisableDirectionLight(unsigned int index);

	void SetPointLight(unsigned int index, const Vector3& position, const Rgba& color, const Vector2& cutoff, float power);
	void DisablePointLight(unsigned int index);

	void SetSpotLight(unsigned int index, const Vector3& position, const Vector3& direction, const Rgba& color, const Vector2& cutoff, float intensity, float inner_angle, float outer_angle, float exp);
	void DisableSpotLight(unsigned int index);

	void SetSpecularSurface(float specPower, bool use_texture);
	void DisableSpecularSurface();

    void SetPointLight(unsigned int index, PointLight* pl);
    void SetSpotLight(unsigned int index, SpotLight* sl);
    void SetDirectionalLight(unsigned int index, DirectionalLight* dl);

	void SetEnvironmentMapEnabled(bool enabled);



	// ---------------------------------
	// Blend State, Depth State, Raster State
	// ---------------------------------
	void set_blend_state(BlendState* blend_state) const;
	void set_depth_stencil_state(DepthStencilState* depth_state) const;
	void set_raster_state(RasterState* raster_state) const;

	void EnableWireframe();
	void DisableWireframe();

	void EnableBlend(BlendFactor srcFactor, BlendFactor dstFactor);
	void DisableBlend();

	void EnableDepth(bool enableTest, bool enableWrite, DepthTest depthTest = DEPTH_TEST_COMPARISON_LESS);
	void EnableDepthWrite(bool enabled);
	void EnableDepthTest(bool enabled);
	void SetDepthTest(DepthTest depthTest);



	// ---------------------------------
	// Viewport
	// ---------------------------------
	void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	void SetViewportAsPercent(float x, float y, float width, float height);



	// ---------------------------------
	// Shaders, Textures, Samplers
	// ---------------------------------
	Shader*		CreateShader(const char* shader_program_filepath);
	void		set_material(const Material* mat);
	void		update_material_buffer(const Material* mat);
	void		set_tesselation_factors(const tesselation_factors_t& tess_factors);

	void SetShader(Shader* shader) const;
	void SetShaderByName(const char* shader_name) const;
	void SetShaderProgram(ShaderProgram* shader_program);

	void		SetTexture(unsigned int textureIndex, RHITextureBase* texture);
	inline void SetTexture(RHITextureBase* texture) { SetTexture(0, texture); }

	void		SetCubeTexture(unsigned int textureIndex, CubeMap* texture);
	inline void SetCubeTexture(CubeMap* texture) { SetCubeTexture(0, texture); }

	void		SetSampler(unsigned int samplerIndex, Sampler* sampler);
	inline void SetSampler(Sampler* sampler) { SetSampler(0, sampler); }

	bool		save_texture2d_to_file(RHITexture2D* texture, const char* filename);
	bool		save_texture2d_to_binary_file(RHITexture2D* texture, const char* filename);
	void		request_screenshot(const char* filename);
	void		process_pending_screenshots();
	void		take_snapshot();


	// ---------------------------------
	// Compute 
	// ---------------------------------
    ComputeJob* create_compute_job(const char* compute_program_filepath);
    void        dispatch_job(ComputeJob* compute_job);



	// ---------------------------------
	// Constant Buffers
	// ---------------------------------
	void		SetConstantBuffer(unsigned int constantBufferIndex, ConstantBuffer* constantBuffer);
	inline void	SetConstantBuffer(ConstantBuffer* constantBuffer) {SetConstantBuffer(0, constantBuffer); };
	void		UpdateConstantBuffer(ConstantBuffer* constantBuffer, const void* data);

	void		SetStructuredBuffer(unsigned int structuredBufferIndex, StructuredBuffer* structuredBuffer);
	inline void	SetStructuredBuffer(StructuredBuffer* structuredBuffer) {SetStructuredBuffer(0, structuredBuffer); };
	void		UpdateStructuredBuffer(StructuredBuffer* constantBuffer, const void* data);


	// ---------------------------------
	// General Drawing
	// ---------------------------------
	void DrawVertexes(PrimitiveType topology, const Vertex3* vertexes, const unsigned int numVertexes);
	void DrawVertexesIndexed(PrimitiveType topology, const Vertex3* vertexes, const unsigned int numVertexes, const unsigned int* indexes, const unsigned int numIndexes);

	void DrawVBO(PrimitiveType topology, VertexBuffer *vbo);
	void DrawVBO(PrimitiveType topology, VertexBuffer *vbo, const unsigned int vertexCount, const unsigned int startIndex = 0);

	void DrawVBOIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo);
	void DrawVBOIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo, const unsigned int indexCount, const unsigned int startIndex = 0);

	void draw_with_meshbuilder(MeshBuilder& mb);
	void draw_mesh(Mesh* mesh);
    void draw_renderable_mesh(RenderableMesh* rm);

	void Present();



	// ---------------------------------
	// 2d Drawing
	// ---------------------------------
	void DrawQuad2d(float minX, float minY, float maxX, float maxY, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	void DrawQuad2d(const Vector2& mins, const Vector2& maxs, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	void DrawQuad2d(const AABB2& bounds, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);

	void DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const char* text, const Font& font);
	void DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const std::vector<char>& text, const Font& font);
	void DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const std::string& text, const Font& font);
	void DrawText2dCentered(const Vector2& centeredPosition, float scale, const Rgba& tint, const std::string& text, const Font& font);
	void DrawText2dCenteredAndInBounds(const AABB2& bounds, const Rgba& tint, const std::string& text, const Font& font);


	
	// ---------------------------------
	// 3d World Drawing
	// ---------------------------------
	void DrawWorldAxes(float axisLength = 100.f);

	void DrawWorldGridXZ(float gridSize = 200.f, 
						 float majorUnitLength = 1.0f, 
						 float minorUnitLength = 0.2f, 
						 const Rgba& majorUnitColor = Rgba(255, 255, 255, 80), 
						 const Rgba& minorUnitColor = Rgba(255, 255, 255, 8));

	CubeMap*	LoadCubeMap(CubeMapImagePaths imagePaths);
	void		SetCurrentReflectionMap(CubeMap* cubeMap);
	void		BindCurrentReflectionMap(unsigned int index);
	void		DrawSkyBox(SkyBox* skybox);
	void		DrawSkyBox(CubeMap* cm);



	// ---------------------------------
	// 3d Shapes Drawing
	// ---------------------------------
	void DrawQuad3d(const Vector3& center,
					const Vector3& right,
					const Vector3& up,
					float widthHalfExtents,
					float heightHalfExtents,
					const AABB2& texCoords = AABB2::ZERO_TO_ONE,
					const Rgba& tint = Rgba::WHITE);

	void DrawTwoSidedQuad3d(const Vector3& center,
							const Vector3& right,
							const Vector3& up,
							float widthHalfExtents,
							float heightHalfExtents,
							const AABB2& texCoords = AABB2::ZERO_TO_ONE,
							const Rgba& tint = Rgba::WHITE);

	void DrawCube3d(const Vector3& center, float halfSize, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	void DrawInvertedCube3d(const Vector3& center, float halfSize, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);

	void DrawUVSphere(const Vector3& center, float radius, const Rgba& color = Rgba::WHITE, unsigned int slices = 64);
	//void DrawIcoSphere();

	// ---------------------------------
	// 3d Debug Drawing
	// ---------------------------------

	void DebugDrawCross3d(const Vector3& position, const Rgba& color, float size);
	void DebugDrawDirectionalLights();
	void DebugDrawPointLights();

	void DebugDrawLine2d(const Vector2& start_pos, const Vector2& end_pos, float line_thickness = 1.0f, const Rgba start_color = Rgba::PINK, const Rgba end_color = Rgba::PINK);
	void DebugDrawLine3d(const Vector3& start_pos, const Vector3& direction, float length = 1.0f, float line_thickness = 1.0f, const Rgba start_color = Rgba::PINK, const Rgba end_color = Rgba::PINK);
	void DebugDrawLine3d(const Vector3& start_pos, const Vector3& end_pos, float line_thickness = 1.0f, const Rgba start_color = Rgba::PINK, const Rgba end_color = Rgba::PINK);

	void DebugDrawBox2d(const AABB2& bounds, float margin, float padding, const Rgba& edge_color, const Rgba& fill_color);

	void DebugDrawCircle2d(const Vector2& center, float radius, float margin, float padding, const Rgba& border_color, const Rgba& fill_color, int num_sides = 64);

	void draw_skeleton(const Skeleton* skeleton);
	void draw_skeleton_instance(const SkeletonInstance* skeleton_instance);

	// ---------------------------------
	// Scene
	// ---------------------------------
    Scene* get_current_scene();
	void set_camera(Camera* camera);
    void render_scene();

private:
	void RecreateDefaultDepthBuffer();
};