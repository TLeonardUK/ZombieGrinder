// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERBATCH_
#define _ENGINE_RENDERBATCH_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Matrix4.h"
#include "Generic/Types/Vector2.h"
#include "Generic/Types/ByteStack.h"
#include "Generic/Types/Variant.h"
#include "Generic/Types/HashTable.h"

#include "Engine/Renderer/RenderPipelineTypes.h"

class Renderer;
class Texture;
class TextureHandle;
class RenderPipeline;

struct RenderBatchUniformValue
{
	MEMORY_ALLOCATOR(RenderBatchUniformValue, "Rendering");
public:

	union
	{
		float	Float_Value;
		int		Int_Value;
		bool	Bool_Value;
	};
	Matrix4 Matrix4_Value;
	Vector3 Vector3_Value;
	Vector4 Vector4_Value;
};

struct RenderBatchInstructionType
{
	enum Type
	{
		Draw_Line,
		Draw_Quad,
		Draw_Triangle,
		Draw_Quad_Verticies,
		Draw_Quad_Verticies_UV,
		Draw_Buffer,
		Set_Color,
		Set_Global_Color,
		Set_Global_Scissor_Test,
		Set_Global_Scissor_Rect,
		Set_Line_Size,
		Set_Uniform_Float,
		Set_Uniform_Matrix4,
		Set_Uniform_Vector3,
		Set_Uniform_Vector4,
		Set_Uniform_Int,
		Set_Uniform_Bool,
		Set_Alpha_Test,
		Set_Depth_Test,
		Set_Depth_Write,
		Set_Blend,
		Set_Blend_Function,
	};
};

// Batches up operations on a group of primitives of the same type/texture into 
// the smallest amount of render calls it can. All drawing should be done through this,
// you should never directly call methods on the native Renderer.
class RenderBatch
{
	MEMORY_ALLOCATOR(RenderBatch, "Rendering");

	Renderer*											m_renderer;
	RenderPipeline*										m_pipeline;
	RenderPipeline_Shader*								m_shader;
	Texture*											m_texture;
	PrimitiveType::Type									m_primitive_type;		
	HashTable<RenderBatchUniformValue, unsigned int>	m_uniform_values;
	int													m_uniform_value_count;

	bool												m_in_render_batch;

protected:
	friend class RenderPipeline;
	
	RenderBatch					();
	RenderBatch					(Texture* texture, PrimitiveType::Type type);
	~RenderBatch				();

	void Reset					(Texture* texture, PrimitiveType::Type type);

	INLINE void Ensure_Batch	();
	INLINE void Finish_Batch	();

	// Management of batch.
	void Clear					();
	void Draw					(RenderPipeline* pipeline);

public:

	// State change instructions.
	void Set_Object_Mask		(Vector4 color);
	void Set_Color				(Color color);
	void Set_Global_Color		(Color color);
	void Set_Global_Scissor_Test(bool test);
	void Set_Global_Scissor_Rectangle(Rect2D rect);
	void Set_Line_Size			(float size);	
	void Set_Uniform_Matrix		(const char* name, Matrix4 matrix);
	void Set_Uniform_Vector		(const char* name, Vector3 vector);
	void Set_Uniform_Vector		(const char* name, Vector4 vector);
	void Set_Uniform_Float		(const char* name, float value);
	void Set_Uniform_Int		(const char* name, int value);
	void Set_Uniform_Bool		(const char* name, bool value);
	void Set_Alpha_Test			(bool value);
	void Set_Depth_Test			(bool value);
	void Set_Depth_Write		(bool value);
	void Set_Blend				(bool value);
	void Set_Blend_Function		(RendererOption::Type value);

	// Batch instructions.
	void Draw_Line				(float x1, float y1, float z1, float x2, float y2, float z2);
	void Draw_Triangle			(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3);
	void Draw_Quad				(Rect2D bounds, Rect2D uv, float depth);
	void Draw_Quad				(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4);
	void Draw_Quad				(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3, Vector3 v4, Vector2 uv4);
	
	//void Begin_Buffer			(PrimitiveBufferFormat::Type format, int primitives);
	//void Finish_Buffer			();

};

#endif

