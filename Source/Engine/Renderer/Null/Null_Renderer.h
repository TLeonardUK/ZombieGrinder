// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_NULL_NULL_RENDERER_
#define _ENGINE_RENDERER_NULL_NULL_RENDERER_

#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Matrix4.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Resources/Compiled/Shaders/CompiledShader.h"

#include <vector>

class GfxDisplay;
class Null_Texture;
class Null_ShaderProgram;
class Null_RenderTarget;
class Thread;

// Maximum number of textures to bind at once.
#define MAX_BINDED_TEXTURES					8

class Null_Renderer : public Renderer
{
	MEMORY_ALLOCATOR(Null_Renderer, "Rendering");

private:
	
	GfxDisplay*				m_display;
	bool					m_initialized;
	
	// Maticies
	Matrix4					m_world_matrix;
	Matrix4					m_view_matrix;
	Matrix4					m_projection_matrix;

	const Null_Texture*		m_binded_textures[MAX_BINDED_TEXTURES];
	Null_RenderTarget*		m_binded_render_target;
	Null_ShaderProgram*		m_binded_shader_program;
	Material*				m_binded_material;

	Color					m_clear_color;
	float					m_clear_depth;
	RendererOption::Type	m_cull_face;
	RendererOption::Type	m_depth_function;
	bool					m_depth_test;
	bool					m_depth_write;
	bool					m_alpha_test;
	Rect2D					m_viewport;
	RendererOption::Type	m_blend_function;
	bool					m_blend;

	bool					m_scissor_test;
	Rect2D					m_scissor_rect;

	friend class Renderer;

	Null_Renderer();
	~Null_Renderer();

	// Extensions!
	void Initialize();

public:

	bool Supports(RendererFeature::Type feature);

	// Base functions.	
	void Flip(const FrameTime& time);
	void Warmup_Textures();
	bool Warmup_Textures_Pending();
	
	// Display related settings.
	bool Set_Display(GfxDisplay* display);
	void Set_VSync(bool value);

	// Logging.
	bool Is_Logging_Enabled();
	void Set_Logging_Enabled(bool bEnabled);

	// Rendering.		
	void Bind_Texture(const Texture* texture, int index);
	void Bind_Material(Material* material);
	void Bind_Shader_Program(ShaderProgram* texture);
	void Bind_Render_Target(RenderTarget* texture);

	Texture*		Create_Texture			(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags);
	Texture*		Create_Texture			(Pixelmap* Pixelmap, TextureFlags::Type flags);

	ShaderProgram*  Create_Shader_Program	(CompiledShaderHeader* data);
	RenderTarget*	Create_Render_Target	();
	
	Material*		Get_Material			();
	
	void			Set_Output_Buffers		(std::vector<OutputBufferType::Type>& outputs);
			
	void					Set_Clear_Color				(Color color);
	Color					Get_Clear_Color				();
	void					Set_Clear_Depth				(float depth);
	float					Get_Clear_Depth				();
	void					Set_Cull_Face				(RendererOption::Type option);
	RendererOption::Type	Get_Cull_Face				();
	void					Set_Depth_Function			(RendererOption::Type option);
	RendererOption::Type	Get_Depth_Function			();
	void					Set_Depth_Test				(bool read);
	bool					Get_Depth_Test				();
	void					Set_Depth_Write				(bool write);
	bool					Get_Depth_Write				();
	void					Set_Alpha_Test				(bool depth);
	bool					Get_Alpha_Test				();
	void					Set_Blend_Function			(RendererOption::Type option);
	RendererOption::Type	Get_Blend_Function			();
	void					Set_Blend					(bool blend);
	bool					Get_Blend					();

	void					Set_Viewport				(Rect2D viewport);
	Rect2D					Set_Viewport				();

	void					Clear_Buffer				(bool color, bool depth);

	void		Set_World_Matrix			(Matrix4 matrix);
	Matrix4		Get_World_Matrix			();
	void		Set_View_Matrix				(Matrix4 matrix);
	Matrix4		Get_View_Matrix				();
	void		Set_Projection_Matrix		(Matrix4 matrix);
	Matrix4		Get_Projection_Matrix		();

	Geometry*	Create_Geometry				(GeometryBufferFormat format, PrimitiveType::Type type, int primitives, bool dynamic);

	Pixelmap*	Capture_Backbuffer			();

	RenderQuery* Create_Query				(RenderQueryType::Type type);

	// Immediate rendering.	
	void Begin_Batch(PrimitiveType::Type type);
	void End_Batch();
	void Set_Line_Size(float size);
	void Set_Vertex_Color(Color color);
	void Set_Global_Vertex_Color(Color color);
	void Set_Scissor_Test(bool test);
	void Set_Scissor_Rectangle(Rect2D rect);
	bool Get_Scissor_Test();
	Rect2D Get_Scissor_Rectangle();
	float Get_Line_Size();
	void Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2);
	void Draw_Quad(Rect2D bounds, Rect2D uv, float depth);
	void Draw_Quad(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4);
	void Draw_Quad(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3, Vector3 v4, Vector2 uv4);
	void Draw_Triangle(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3);
	void Draw_Buffer(void* data, PrimitiveBufferFormat::Type data_format, PrimitiveType::Type prim_type, int prim_count);

};

#endif

