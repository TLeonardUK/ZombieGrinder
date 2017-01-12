// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Null/Null_Renderer.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Actor.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/Light.h"
#include "Engine/Renderer/Drawable.h"
#include "Engine/Engine/GameEngine.h"

#include "Generic/Threads/Thread.h"

#include "Engine/Engine/FrameStats.h"

#include "Generic/Math/Math.h"
#include "Generic/Types/Matrix4.h"
#include "Generic/Types/Rect2D.h"

#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/Textures/TextureFactory.h"

#include "Engine/Renderer/Null/Null_Texture.h"
#include "Engine/Renderer/Null/Null_ShaderProgram.h"
#include "Engine/Renderer/Null/Null_RenderTarget.h"
#include "Engine/Renderer/Null/Null_Geometry.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/Stream.h"

#include <vector>

Null_Renderer::Null_Renderer()
	: m_initialized(false)
	, m_display(NULL)
	, m_binded_shader_program(NULL)
	, m_binded_render_target(NULL)
	, m_binded_material(NULL)
	, m_viewport(0, 0, 0, 0)
	, m_scissor_test(false)
	, m_depth_write(false)
{
	memset(m_binded_textures, 0, sizeof(Null_Texture*) * MAX_BINDED_TEXTURES);
}

Null_Renderer::~Null_Renderer()
{
}

bool Null_Renderer::Supports(RendererFeature::Type feature)
{
	return false;
}

void Null_Renderer::Initialize()
{
	Set_Clear_Color(Color::Black);
	Set_Clear_Depth(0.0f);
	Set_Depth_Test(true);
	Set_Depth_Write(true);
	Set_Depth_Function(RendererOption::E_Greater_Or_Equal);
	Set_Cull_Face(RendererOption::E_Back);
	Set_Scissor_Test(false);
}

void Null_Renderer::Flip(const FrameTime& time)
{
	// Swap front/back buffers.
	m_display->Swap_Buffers();
}

void Null_Renderer::Warmup_Textures()
{
}

bool Null_Renderer::Warmup_Textures_Pending()
{
	return false;
}

bool Null_Renderer::Is_Logging_Enabled()
{
	return false;
}

void Null_Renderer::Set_Logging_Enabled(bool bEnabled)
{
}

bool Null_Renderer::Set_Display(GfxDisplay* display)
{
	m_display = display;

	if (m_initialized == false)
	{
		Initialize();
		m_initialized = true;
	}

	return true;
}

void Null_Renderer::Set_VSync(bool value)
{
	
}

void Null_Renderer::Bind_Shader_Program(ShaderProgram* shader)
{
	Null_ShaderProgram* glShader = dynamic_cast<Null_ShaderProgram*>(shader);
	m_binded_shader_program = glShader;
}

ShaderProgram* Null_Renderer::Create_Shader_Program(CompiledShaderHeader* data)
{
	return new Null_ShaderProgram(data);
}

void Null_Renderer::Bind_Render_Target(RenderTarget* target)
{
	Null_RenderTarget* glTexture = const_cast<Null_RenderTarget*>(dynamic_cast<const Null_RenderTarget*>(target));
	m_binded_render_target = glTexture;
}

RenderTarget* Null_Renderer::Create_Render_Target()
{
	return new Null_RenderTarget();
}

void Null_Renderer::Bind_Texture(const Texture* texture, int index)
{
	Null_Texture* glTexture = const_cast<Null_Texture*>(dynamic_cast<const Null_Texture*>(texture));
	m_binded_textures[index] = glTexture;
}

void Null_Renderer::Bind_Material(Material* material)
{
	Bind_Texture(material->Get_Texture()->Get(), 0);
	m_binded_material = material;
}
	
Material* Null_Renderer::Get_Material()
{
	return m_binded_material;
}
	
void Null_Renderer::Set_Output_Buffers(std::vector<OutputBufferType::Type>& outputs)
{
}

void Null_Renderer::Set_Clear_Color(Color color)
{
	m_clear_color = color;
}

Color Null_Renderer::Get_Clear_Color()
{
	return m_clear_color;
}

void Null_Renderer::Set_Clear_Depth(float depth)
{
	m_clear_depth = depth;
}

float Null_Renderer::Get_Clear_Depth()
{
	return m_clear_depth;
}

void Null_Renderer::Set_Cull_Face(RendererOption::Type option)
{
	m_cull_face = option;
}

RendererOption::Type Null_Renderer::Get_Cull_Face()
{
	return m_cull_face;
}

void Null_Renderer::Set_Depth_Function(RendererOption::Type option)
{
	m_depth_function = option;
}

RendererOption::Type Null_Renderer::Get_Depth_Function()
{
	return m_depth_function;
}

void Null_Renderer::Set_Depth_Test(bool depth)
{
	m_depth_test = depth;
}

bool Null_Renderer::Get_Depth_Test()
{
	return m_depth_test;
}

void Null_Renderer::Set_Depth_Write(bool depth)
{
	m_depth_write = depth;
}

bool Null_Renderer::Get_Depth_Write()
{
	return m_depth_write;
}

void Null_Renderer::Set_Alpha_Test(bool depth)
{
	m_alpha_test = depth;
}

bool Null_Renderer::Get_Alpha_Test()
{
	return m_alpha_test;
}

void Null_Renderer::Set_Blend_Function(RendererOption::Type option)
{
	m_blend_function = option;
}

RendererOption::Type Null_Renderer::Get_Blend_Function()
{
	return m_blend_function;
}

void Null_Renderer::Set_Blend(bool blend)
{
	m_blend = blend;
}

bool Null_Renderer::Get_Blend()
{
	return m_blend;
}

float Null_Renderer::Get_Line_Size()
{
	return 0.0f;
}

void Null_Renderer::Set_Viewport(Rect2D viewport)
{
	GfxDisplay* display = GfxDisplay::Get();

	m_viewport			= viewport; 
	m_viewport.X		= viewport.X;
	m_viewport.Y		= (display->Get_Height() - (viewport.Y + viewport.Height));
	m_viewport.Width	= Max(viewport.Width, 0);
	m_viewport.Height	= Max(viewport.Height, 0);
}

Rect2D Null_Renderer::Set_Viewport()
{
	return m_viewport;
}

void Null_Renderer::Set_Scissor_Test(bool test)
{
	m_scissor_test = test;
}

void Null_Renderer::Set_Scissor_Rectangle(Rect2D rect)
{
	GfxDisplay* display = GfxDisplay::Get();

	m_scissor_rect			= rect; 
	m_scissor_rect.X		= rect.X;
	m_scissor_rect.Y		= (display->Get_Height() - (rect.Y + rect.Height));
	m_scissor_rect.Width	= Max(rect.Width, 0);
	m_scissor_rect.Height	= Max(rect.Height, 0);
}

bool Null_Renderer::Get_Scissor_Test()
{
	return m_scissor_test;
}

Rect2D Null_Renderer:: Get_Scissor_Rectangle()
{
	return m_scissor_rect;
}

void Null_Renderer::Clear_Buffer(bool color, bool depth)
{	
}

Texture* Null_Renderer::Create_Texture(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags)
{	
	return new Null_Texture(width, height, pitch, format, flags);
}

Texture* Null_Renderer::Create_Texture(Pixelmap* Pixelmap, TextureFlags::Type flags)
{	
	// Null renderer dosen't give a shit about pixelmap data.
	if (Pixelmap != NULL && (flags & TextureFlags::PersistSourceData) == 0)
	{
		Pixelmap->Free_Data();
	}

	return new Null_Texture(Pixelmap, flags);
}

RenderQuery* Null_Renderer::Create_Query(RenderQueryType::Type type)
{
	return NULL;
}

void Null_Renderer::Set_World_Matrix(Matrix4 matrix)
{
	m_world_matrix = matrix;
}

Matrix4 Null_Renderer::Get_World_Matrix()
{
	return m_world_matrix;
}

void Null_Renderer::Set_View_Matrix(Matrix4 matrix)
{
	m_view_matrix = matrix;
}

Matrix4 Null_Renderer::Get_View_Matrix()
{
	return m_view_matrix;
}

void Null_Renderer::Set_Projection_Matrix(Matrix4 matrix)
{
	m_projection_matrix = matrix;
}

Matrix4 Null_Renderer::Get_Projection_Matrix()
{
	return m_projection_matrix;
}

Geometry* Null_Renderer::Create_Geometry(GeometryBufferFormat format, PrimitiveType::Type type, int primitives, bool dynamic)
{
	return new Null_Geometry(format, type, primitives, dynamic);
}


Pixelmap* Null_Renderer::Capture_Backbuffer()
{
	return new Pixelmap(GfxDisplay::Get()->Get_Width(), GfxDisplay::Get()->Get_Height(), PixelmapFormat::R8G8B8A8);
}

void Null_Renderer::Begin_Batch(PrimitiveType::Type type)
{	
}	

void Null_Renderer::End_Batch()
{
}

void Null_Renderer::Set_Line_Size(float size)
{
}

void Null_Renderer::Set_Vertex_Color(Color color)
{
}

void Null_Renderer::Set_Global_Vertex_Color(Color color) 
{
}

void Null_Renderer::Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2)
{	
}

void Null_Renderer::Draw_Quad(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4)
{
}

void Null_Renderer::Draw_Quad(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3, Vector3 v4, Vector2 uv4)
{
}

void Null_Renderer::Draw_Quad(Rect2D bounds, Rect2D uv, float depth)
{
}

void Null_Renderer::Draw_Triangle(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3)
{
}

void Null_Renderer::Draw_Buffer(void* data, PrimitiveBufferFormat::Type data_format, PrimitiveType::Type prim_type, int prim_count)
{
}