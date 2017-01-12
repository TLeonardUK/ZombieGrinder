// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_RENDERER_OPENGL_OPENGL_RENDERER_
#define _ENGINE_RENDERER_OPENGL_OPENGL_RENDERER_

#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Matrix4.h"
#include "Engine/Renderer/Renderer.h"

#if defined(PLATFORM_WIN32)
#include <Windows.h>
#include "Engine/Display/Win32/Win32_GfxDisplay.h"

#elif defined(PLATFORM_LINUX)
#include "Engine/Display/Linux/Linux_GfxDisplay.h"

#elif defined(PLATFORM_MACOS)
#include "Engine/Display/MacOS/MacOS_GfxDisplay.h"

#endif

#include <vector>

class GfxDisplay;
class OpenGL_Texture;
class OpenGL_ShaderProgram;
class OpenGL_RenderTarget;
class OpenGL_Geometry;
class Thread;
class Mutex;

// Maximum number of textures to bind at once.
#define MAX_BINDED_TEXTURES					8

// Maximum number of primitives in a batch.
#define MAX_BATCH_PRIMITIVES				1024 * 8

// Maximum number of vertices per primitive.
#define MAX_BATCH_PRIMITIVE_VERTICES		4

#define MAX_CLIENT_STATES 32

// Gl error handling.
#if !defined(MASTER_BUILD) && defined(PLATFORM_WIN32)

//#define BREAK_ON_GL_ERROR 

#ifdef DEBUG_BUILD
#define WARN_ON_GL_ERROR 
#endif

#endif

#ifdef PLATFORM_WIN32
#define GetGLErrorString(x) gluErrorString(x)
#else
#define GetGLErrorString(x) "[gluErrorString Not Available]"
#endif

#ifdef BREAK_ON_GL_ERROR
	#define CHECK_GL_ERROR() \
		{ \
			GLenum error; \
			if ((error = glGetError()) != GL_NO_ERROR) \
 			{ \
				DBG_ASSERT_STR(false, "[OpenGL Renderer] OpenGL error occured '%i' (%s) @ " __FILE__ ":%i.", error, GetGLErrorString(error), __LINE__); \
			} \
		}
#elif defined WARN_ON_GL_ERROR
	#define CHECK_GL_ERROR() \
		{ \
			GLenum error; \
			if ((error = glGetError()) != GL_NO_ERROR) \
 			{ \
				DBG_LOG("[OpenGL Renderer] OpenGL error occured '%i' (%s) @ " __FILE__ ":%i.", error, GetGLErrorString(error), __LINE__); \
			} \
		}
#else
	#define CHECK_GL_ERROR() \
		if (Renderer::Get()->Is_Logging_Enabled()) { \
			GLenum error; \
			if ((error = glGetError()) != GL_NO_ERROR) \
 			{ \
				DBG_LOG("[OpenGL Renderer] OpenGL error occured '%i' (%s) @ " __FILE__ ":%i.", error, GetGLErrorString(error), __LINE__); \
			} \
		}
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#define GL_REQUIRED_EXTENSION(id)
#define GL_EXTENSION_FALLBACK(return_type, name, parameters, entry_point, required)	
#define GL_EXTENSION(return_type, name, parameters, entry_point, required) typedef return_type (APIENTRY * funcptr_##name) parameters; extern funcptr_##name name;
#include "Engine/Renderer/OpenGL/OpenGL_Extensions.inc"
#undef GL_EXTENSION
#undef GL_EXTENSION_FALLBACK
#undef GL_REQUIRED_EXTENSION

// Use DWM_Flush to vsync when using composited rendering.
//#define USE_DWM_FLUSH

#if defined(PLATFORM_WIN32) && defined(USE_DWM_FLUSH)
typedef void (WINAPI *PDwmFlush)(void);
#endif

struct OpenGL_BufferPointerState
{
	int element_count;
	int stride;
	int format;
	GLvoid* buffer;
};

// Opengl rendering fun times!
class OpenGL_Renderer : public Renderer
{
	MEMORY_ALLOCATOR(OpenGL_Renderer, "Rendering");

private:
	GfxDisplay*				m_display;
	bool					m_initialized;
	Thread*					m_render_thread;
	
	// Maticies
	Matrix4					m_world_matrix;
	Matrix4					m_view_matrix;
	Matrix4					m_projection_matrix;

	const OpenGL_Texture*	m_binded_textures[MAX_BINDED_TEXTURES];
	OpenGL_RenderTarget*	m_binded_render_target;
	OpenGL_ShaderProgram*	m_binded_shader_program;
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

	float					m_line_width;

	bool					m_scissor_test;
	Rect2D					m_scissor_rect;

	bool					m_in_batch;
	Vector4					m_global_primitive_color;
	PrimitiveType::Type	m_batch_primitive;
	int						m_batch_primitive_count;
	Vector4					m_batch_primitive_color;

	int						m_batch_primitive_colors_counter;
	int						m_batch_primitive_normals_counter;
	int						m_batch_primitive_vertices_counter;
	int						m_batch_primitive_texcoords_counter;

	float					m_batch_primitive_colors	[MAX_BATCH_PRIMITIVES * MAX_BATCH_PRIMITIVE_VERTICES * 4];
	float					m_batch_primitive_normals	[MAX_BATCH_PRIMITIVES * MAX_BATCH_PRIMITIVE_VERTICES * 3];
	float					m_batch_primitive_texcoords	[MAX_BATCH_PRIMITIVES * MAX_BATCH_PRIMITIVE_VERTICES * 2];
	float					m_batch_primitive_verticies	[MAX_BATCH_PRIMITIVES * MAX_BATCH_PRIMITIVE_VERTICES * 3];

	bool					m_client_states[MAX_CLIENT_STATES];

	Mutex*						m_warmup_textures_mutex;
	std::vector<OpenGL_Texture*> m_warmup_textures;

	std::vector<GLuint> m_defer_buffer_deletes;
	Mutex* m_defer_mutex;

	bool m_pending_vsync_set;
	bool m_pending_vsync_value;
	bool m_vsync_enabled;

	GfxDisplayMode::Type m_last_known_display_mode;

#if defined(PLATFORM_WIN32) && defined(USE_DWM_FLUSH)
	HMODULE m_dwm_lib;
	PDwmFlush DwmFlush;
#endif

	GLuint m_array_buffer;
	OpenGL_BufferPointerState m_vertex_pointer_state;
	OpenGL_BufferPointerState m_texcoord_pointer_state;
	OpenGL_BufferPointerState m_color_pointer_state;

	bool bSupportsFeatureIsCached[(int)RendererFeature::COUNT];
	bool bSupportsFeatureCache[(int)RendererFeature::COUNT];

	bool m_logging_enabled;

	friend class Renderer;
	friend class OpenGL_Geometry;

	OpenGL_Renderer();
	~OpenGL_Renderer();

	// Extensions!
	void Initialize_OpenGL();
	bool Is_Extension_Supported(const char* name);

	// Mesh stuff
	INLINE void Add_Batch_Vertex(float x, float y, float z,
						  float nx, float ny, float nz, 
						  float tx, float ty, 
						  float r, float g, float b, float a);

	void Set_Client_State_Enabled(GLint state, bool enabled);

	void Bind_Array_Buffer(GLuint buffer);
	void Bind_Vertex_Pointer(int element_count, int format, int stride, GLvoid* buffer);	
	void Bind_TexCoord_Pointer(int element_count, int format, int stride, GLvoid* buffer);
	void Bind_Color_Pointer(int element_count, int format, int stride, GLvoid* buffer);

	// Deferes deletion of a buffer until we are running on main-thread.
	void DeferBufferDelete(GLuint id); 

public:

	bool Supports(RendererFeature::Type feature);

	// Base functions.	
	void Flip(const FrameTime& time);
	void Warmup_Textures();
	bool Warmup_Textures_Pending();

	// Display related settings.
	bool Set_Display(GfxDisplay* display);
	void Set_VSync(bool vsync);

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
	void					Set_Depth_Test				(bool depth);
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

	RenderQuery* Create_Query				(RenderQueryType::Type type);

	Pixelmap*	Capture_Backbuffer			();

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

#endif