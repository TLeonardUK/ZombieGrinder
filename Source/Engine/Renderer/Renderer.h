// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_
#define _ENGINE_RENDERER_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Matrix4.h"
#include "Generic/Types/AABB.h"
#include "Generic/Types/Frustum.h"
#include "Generic/Types/Color.h"
#include "Generic/Types/Rect2D.h"

#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/RenderTarget.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/RendererTypes.h"
#include "Engine/Renderer/Geometry.h"

#include "Engine/Resources/Compiled/Shaders/CompiledShader.h"

#include <vector>

class GfxDisplay;
class Camera;
class ShaderProgram;
class Pixelmap;
class RenderQuery;

struct RendererFeature
{
	enum Type
	{
		Shaders,
		Querys,
		FBO,
//		VBO,
		COUNT,
	};
};

struct RenderQueryType
{
	enum Type
	{
		Time,
	};
};

class Renderer : public Singleton<Renderer>
{
	MEMORY_ALLOCATOR(Renderer, "Rendering");

public:
	static Renderer*				Create();
	virtual ~Renderer() {}

	virtual bool					Supports					(RendererFeature::Type feature) = 0;

	// Base functions.	
	virtual void					Flip						(const FrameTime& time) = 0;
	virtual void					Warmup_Textures				() =0;
	virtual bool					Warmup_Textures_Pending		() = 0;

	// Logging functionality.
	virtual bool					Is_Logging_Enabled			() = 0;
	virtual void					Set_Logging_Enabled			(bool bEnabled) = 0;

	// Display related settings.
	virtual bool					Set_Display					(GfxDisplay* display) = 0;
	virtual void					Set_VSync					(bool vsync) = 0;

	// Rendering! What we are all here for.
	virtual void					Bind_Render_Target			(RenderTarget* texture) = 0;
	virtual void					Bind_Material				(Material* material) = 0;
	virtual void					Bind_Texture				(const Texture* texture, int slot) = 0;
	virtual void					Bind_Shader_Program			(ShaderProgram* program) = 0;

	virtual Texture*				Create_Texture				(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags) = 0;
	virtual Texture*				Create_Texture				(Pixelmap* Pixelmap, TextureFlags::Type flags) = 0;
	virtual ShaderProgram*			Create_Shader_Program		(CompiledShaderHeader* data) = 0;

	virtual RenderTarget*			Create_Render_Target		() = 0;

	virtual Material*				Get_Material				() = 0;

	virtual void					Set_Output_Buffers			(std::vector<OutputBufferType::Type>& outputs) = 0;

	virtual void					Set_Clear_Color				(Color color) = 0;
	virtual Color					Get_Clear_Color				() = 0;
	virtual void					Set_Clear_Depth				(float depth) = 0;
	virtual float					Get_Clear_Depth				() = 0;
	virtual void					Set_Cull_Face				(RendererOption::Type option) = 0;
	virtual RendererOption::Type	Get_Cull_Face				() = 0;
	virtual void					Set_Depth_Function			(RendererOption::Type option) = 0;
	virtual RendererOption::Type	Get_Depth_Function			() = 0;
	virtual void					Set_Depth_Test				(bool depth) = 0;
	virtual bool					Get_Depth_Test				() = 0;	
	virtual void					Set_Depth_Write				(bool depth) = 0;
	virtual bool					Get_Depth_Write				() = 0;	
	virtual void					Set_Alpha_Test				(bool depth) = 0;
	virtual bool					Get_Alpha_Test				() = 0;
	virtual void					Set_Blend_Function			(RendererOption::Type option) = 0;
	virtual RendererOption::Type	Get_Blend_Function			() = 0;
	virtual void					Set_Blend					(bool blend) = 0;
	virtual bool					Get_Blend					() = 0;

	virtual void					Set_Viewport				(Rect2D viewport) = 0;
	virtual Rect2D					Set_Viewport				() = 0;

	virtual void					Clear_Buffer				(bool color = true, bool depth = true) = 0;

	virtual void					Set_World_Matrix			(Matrix4 matrix) = 0;
	virtual Matrix4					Get_World_Matrix			() = 0;
	virtual void					Set_View_Matrix				(Matrix4 matrix) = 0;
	virtual Matrix4					Get_View_Matrix				() = 0;
	virtual void					Set_Projection_Matrix		(Matrix4 matrix) = 0;
	virtual Matrix4					Get_Projection_Matrix		() = 0;

	virtual Geometry*				Create_Geometry				(GeometryBufferFormat format, PrimitiveType::Type type, int primitives, bool dynamic) = 0;
	
	virtual Pixelmap*				Capture_Backbuffer			() = 0;

	virtual RenderQuery*			Create_Query				(RenderQueryType::Type type) = 0;

	// Immediate rendering (mainly used for debugging).
	virtual void					Begin_Batch					(PrimitiveType::Type type) = 0;
	virtual void					End_Batch					() = 0;
	virtual void					Set_Line_Size				(float size) = 0;
	virtual void					Set_Vertex_Color			(Color color) = 0;
	virtual void					Set_Global_Vertex_Color		(Color color) = 0;
	virtual void					Set_Scissor_Test			(bool test) = 0;
	virtual void					Set_Scissor_Rectangle			(Rect2D rect) = 0;
	virtual bool					Get_Scissor_Test			() = 0;
	virtual Rect2D					Get_Scissor_Rectangle			() = 0;
	virtual float					Get_Line_Size				() = 0;
	virtual void					Draw_Line					(float x1, float y1, float z1, float x2, float y2, float z2) = 0;
	virtual void					Draw_Quad					(Rect2D bounds, Rect2D uv, float depth) = 0;
	virtual void					Draw_Quad					(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4) = 0;
	virtual void					Draw_Quad					(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3, Vector3 v4, Vector2 uv4) = 0;
	virtual void					Draw_Triangle				(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3) = 0;
	virtual void					Draw_Buffer					(void* data, PrimitiveBufferFormat::Type format, PrimitiveType::Type prim_type, int prim_count) = 0;

};

#endif

