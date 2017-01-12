// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/RenderBatch.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"
#include "Engine/Renderer/Shaders/ShaderProgramHandle.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Helper/StringHelper.h"
#include "Generic/Stats/Stats.h"

DEFINE_FRAME_STATISTIC("Render/Draw Batches", int, g_stat_render_draw_batches,			true);

RenderBatch::RenderBatch()
	: m_in_render_batch(false)
	, m_uniform_value_count(0)
	, m_texture(NULL)
	, m_primitive_type(PrimitiveType::None)
{
	m_renderer = Renderer::Get();
	m_pipeline = RenderPipeline::Get();
}

void RenderBatch::Reset(Texture* texture, PrimitiveType::Type type)
{
	if (texture != m_texture || m_primitive_type != type)
	{	
		Clear();

		m_shader = m_pipeline->Get_Active_Shader();
		m_texture = texture;
		m_primitive_type = type;


		if (m_texture != NULL)
		{
			m_renderer->Bind_Texture(m_texture, 0);
		}
		else
		{
			m_renderer->Bind_Texture(NULL, 0);
		}

		// Bind the texture.
		if (m_shader != NULL)
		{
			m_shader->Shader_Program->Get()->Bind_Texture("g_texture", 0);
			m_shader->Shader_Program->Get()->Bind_Bool("g_texture_enabled", m_texture != NULL);	
		}
	}
}

RenderBatch::~RenderBatch()
{
}

void RenderBatch::Clear()
{
	Finish_Batch();

	m_texture = NULL;
	m_primitive_type = PrimitiveType::None;

	m_uniform_values.Clear();
	m_uniform_value_count = 0;
}

void RenderBatch::Ensure_Batch()
{
	if (!m_in_render_batch)
	{
		g_stat_render_draw_batches.Set(1);

		Renderer::Get()->Begin_Batch(m_primitive_type);
		m_in_render_batch = true;
	}
}

void RenderBatch::Finish_Batch()
{
	if (m_in_render_batch)
	{
		Renderer::Get()->End_Batch();
		m_in_render_batch = false;
	}
}

void RenderBatch::Draw(RenderPipeline* pipeline)
{
	Clear();
}

// State change instructions.
void RenderBatch::Set_Color(Color color)
{
	m_renderer->Set_Vertex_Color(color);
}

void RenderBatch::Set_Object_Mask(Vector4 object_mask)
{
	RenderPipeline::Get()->Set_Object_Mask(object_mask);
}

void RenderBatch::Set_Global_Color(Color color)
{
	m_renderer->Set_Global_Vertex_Color(color);
}

void RenderBatch::Set_Global_Scissor_Test(bool test)
{
	if (m_in_render_batch == true && test != m_renderer->Get_Scissor_Test())
	{
		Finish_Batch();
	}
	m_renderer->Set_Scissor_Test(test);
}

void RenderBatch::Set_Global_Scissor_Rectangle(Rect2D rect)
{
	if (m_in_render_batch == true && rect != m_renderer->Get_Scissor_Rectangle())
	{
		Finish_Batch();
	}
	m_renderer->Set_Scissor_Rectangle(rect);
}

void RenderBatch::Set_Line_Size(float size)
{
	if (m_in_render_batch == true && size != m_renderer->Get_Line_Size())
	{
		Finish_Batch();
	}
	m_renderer->Set_Line_Size(size);
}

void RenderBatch::Set_Uniform_Matrix(const char* name, Matrix4 value)
{
	unsigned int			 hash			= StringHelper::Hash(name);
	RenderBatchUniformValue* current_value;
	bool					 value_exists	= m_uniform_values.Get_Ptr(hash, current_value);

	if (!value_exists || current_value->Matrix4_Value != value)
	{
		Finish_Batch();
		if (m_shader != NULL)
		{
			m_shader->Shader_Program->Get()->Bind_Matrix(name, value);
		}

		if (value_exists == true)
		{
			current_value->Matrix4_Value = value;
		}
		else
		{
			RenderBatchUniformValue new_value;
			new_value.Matrix4_Value = value;

			m_uniform_values.Set(hash, new_value);
			m_uniform_value_count++;
		}
	}
}

void RenderBatch::Set_Uniform_Vector(const char* name, Vector3 value)
{
	unsigned int			 hash			= StringHelper::Hash(name);
	RenderBatchUniformValue* current_value;
	bool					 value_exists	= m_uniform_values.Get_Ptr(hash, current_value);

	if (!value_exists || current_value->Vector3_Value != value)
	{
		Finish_Batch();

		if (m_shader != NULL)
		{
			m_shader->Shader_Program->Get()->Bind_Vector(name, value);
		}

		if (value_exists == true)
		{
			current_value->Vector3_Value = value;
		}
		else
		{
			RenderBatchUniformValue new_value;
			new_value.Vector3_Value = value;

			m_uniform_values.Set(hash, new_value);
			m_uniform_value_count++;
		}
	}
}

void RenderBatch::Set_Uniform_Vector(const char* name, Vector4 value)
{
	unsigned int			 hash			= StringHelper::Hash(name);
	RenderBatchUniformValue* current_value;
	bool					 value_exists	= m_uniform_values.Get_Ptr(hash, current_value);

	if (!value_exists || current_value->Vector4_Value != value)
	{
		Finish_Batch();

		if (m_shader != NULL)
		{
			m_shader->Shader_Program->Get()->Bind_Vector(name, value);
		}

		if (value_exists == true)
		{
			current_value->Vector4_Value = value;
		}
		else
		{
			RenderBatchUniformValue new_value;
			new_value.Vector4_Value = value;

			m_uniform_values.Set(hash, new_value);
			m_uniform_value_count++;
		}
	}
}

void RenderBatch::Set_Uniform_Float(const char* name, float value)
{
	unsigned int			 hash			= StringHelper::Hash(name);
	RenderBatchUniformValue* current_value;
	bool					 value_exists	= m_uniform_values.Get_Ptr(hash, current_value);

	if (!value_exists || current_value->Float_Value != value)
	{
		Finish_Batch();

		if (m_shader != NULL)
		{
			m_shader->Shader_Program->Get()->Bind_Float(name, value);
		}

		if (value_exists == true)
		{
			current_value->Float_Value = value;
		}
		else
		{
			RenderBatchUniformValue new_value;
			new_value.Float_Value = value;

			m_uniform_values.Set(hash, new_value);
			m_uniform_value_count++;
		}
	}

}

void RenderBatch::Set_Uniform_Int(const char* name, int value)
{
	unsigned int			 hash			= StringHelper::Hash(name);
	RenderBatchUniformValue* current_value;
	bool					 value_exists	= m_uniform_values.Get_Ptr(hash, current_value);

	if (!value_exists || current_value->Int_Value != value)
	{
		Finish_Batch();
		
		if (m_shader != NULL)
		{
			m_shader->Shader_Program->Get()->Bind_Int(name, value);
		}

		if (value_exists == true)
		{
			current_value->Int_Value = value;
		}
		else
		{
			RenderBatchUniformValue new_value;
			new_value.Int_Value = value;

			m_uniform_values.Set(hash, new_value);
			m_uniform_value_count++;
		}
	}
}

void RenderBatch::Set_Uniform_Bool(const char* name, bool value)
{
	unsigned int			 hash			= StringHelper::Hash(name);
	RenderBatchUniformValue* current_value;
	bool					 value_exists	= m_uniform_values.Get_Ptr(hash, current_value);

	if (!value_exists || current_value->Bool_Value != value)
	{
		Finish_Batch();
		
		if (m_shader != NULL)
		{
			m_shader->Shader_Program->Get()->Bind_Bool(name, value);
		}

		if (value_exists == true)
		{
			current_value->Bool_Value = value;
		}
		else
		{
			RenderBatchUniformValue new_value;
			new_value.Bool_Value = value;

			m_uniform_values.Set(hash, new_value);
			m_uniform_value_count++;
		}
	}
}

void RenderBatch::Set_Alpha_Test(bool value)
{
	if (m_in_render_batch == true && value != m_renderer->Get_Alpha_Test())
	{
		Finish_Batch();
	}

	m_renderer->Set_Alpha_Test(value);
}

void RenderBatch::Set_Depth_Test(bool value)
{
	if (m_in_render_batch == true && value != m_renderer->Get_Depth_Test())
	{
		Finish_Batch();
	}

	m_renderer->Set_Depth_Test(value);
}

void RenderBatch::Set_Depth_Write(bool value)
{
	if (m_in_render_batch == true && value != m_renderer->Get_Depth_Write())
	{
		Finish_Batch();
	}

	m_renderer->Set_Depth_Write(value);
}

void RenderBatch::Set_Blend(bool value)
{
	if (m_in_render_batch == true && value != m_renderer->Get_Blend())
	{
		Finish_Batch();
	}

	m_renderer->Set_Blend(value);
}

void RenderBatch::Set_Blend_Function(RendererOption::Type value)
{
	if (m_in_render_batch == true && value != m_renderer->Get_Blend_Function())
	{
		Finish_Batch();
	}

	m_renderer->Set_Blend_Function(value);
}

void RenderBatch::Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2)
{
	DBG_ASSERT_STR(m_primitive_type == PrimitiveType::Line, "Invalid render batch primitive.");
	
	Ensure_Batch();
	m_renderer->Draw_Line(x1, y1, z1, x2, y2, z2);
}

void RenderBatch::Draw_Quad(Rect2D bounds, Rect2D uv, float depth)
{
	DBG_ASSERT_STR(m_primitive_type == PrimitiveType::Quad, "Invalid render batch primitive.");

	Ensure_Batch();
	m_renderer->Draw_Quad(bounds, uv, depth);
}

void RenderBatch::Draw_Triangle(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3)
{
	DBG_ASSERT_STR(m_primitive_type == PrimitiveType::Triangle, "Invalid render batch primitive.");

	Ensure_Batch();
	m_renderer->Draw_Triangle(v1,uv1,v2,uv2,v3,uv3);
}

void RenderBatch::Draw_Quad(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4)
{
	DBG_ASSERT_STR(m_primitive_type == PrimitiveType::Quad, "Invalid render batch primitive.");

	Ensure_Batch();
	m_renderer->Draw_Quad(v1, v2, v3, v4);
}

void RenderBatch::Draw_Quad(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3, Vector3 v4, Vector2 uv4)
{
	DBG_ASSERT_STR(m_primitive_type == PrimitiveType::Quad, "Invalid render batch primitive.");
			
	Ensure_Batch();
	m_renderer->Draw_Quad(v1, uv1, v2, uv2, v3, uv3, v4, uv4);
}

