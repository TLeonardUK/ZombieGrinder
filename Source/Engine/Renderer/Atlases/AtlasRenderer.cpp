// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/Atlas.h"

#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/RenderBatch.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Camera.h"

#include "Generic/Math/Math.h"

#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/TextureFactory.h"

AtlasRenderer::AtlasRenderer(AtlasHandle* atlas)
	: m_atlas(atlas)
	, m_batched(false)
	, m_in_batch(false)
	, m_batch_texture(false)
{
}

AtlasRenderer::AtlasRenderer()
	: m_atlas(NULL)
	, m_batched(false)
	, m_in_batch(false)
	, m_batch_texture(false)
{
}

AtlasRenderer::~AtlasRenderer()
{
}

void AtlasRenderer::Begin(Texture* texture, PrimitiveType::Type type)
{
	RenderPipeline*	pipeline = RenderPipeline::Get();
	m_batch = pipeline->Get_Render_Batch(texture, type);

	m_batch->Set_Alpha_Test(true);
	m_batch->Set_Blend(true);
	m_batch->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
}

void AtlasRenderer::End()
{
	Renderer* renderer = Renderer::Get();

	m_batch->Set_Alpha_Test(true);
	m_batch->Set_Blend(false);
	/*
	m_batch->Set_Alpha_Test(true);
	m_batch->Set_Blend(true);
	m_batch->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	*/
}

void AtlasRenderer::Begin_Batch()
{
	if (m_batched == false)
	{
		m_batched = true;
		m_in_batch = false;
	}
}

void AtlasRenderer::End_Batch()
{
	if (m_batched == true)
	{
		if (m_in_batch == true)
		{
			End();
		}

		m_batched = false;
		m_in_batch = false;
	}
}

void AtlasRenderer::Draw(Texture* texture, 
						 Rect2D uv, 
						 Vector2 origin, 
						 Rect2D  point, 
						 float depth, 
						 Color color, 
						 bool flip_horizontal, 
						 bool flip_vertical, 
						 RendererOption::Type blend_mode, 
						 Vector2 scale, 
						 float rotation,
						 Vector4 object_mask,
						 bool tiled, 
						 Vector2 tile_repeats)
{
	// Create batch if required.
	if (m_batched == false || m_in_batch == false || m_batch_texture != texture)
	{
		if (m_in_batch == true)
		{
			End();
		}
		Begin(texture, PrimitiveType::Quad);
		m_in_batch = true;
		m_batch_texture = texture;
	}

	// Flip UV's if the frame is flipped.
	if (flip_horizontal == true)
	{
		uv.X = uv.X + uv.Width;
		uv.Width = -uv.Width;
	}	
	if (flip_vertical == true)
	{
		uv.Y = uv.Y + uv.Height;
		uv.Height = -uv.Height;
	}

	// Tiled?
	if (tiled == true)
	{
		m_batch->Set_Uniform_Bool("g_tiled", true);
		m_batch->Set_Uniform_Vector("g_tiled_uv_limits", Vector4(uv.X, 1.0f - uv.Y, uv.Width, -uv.Height));
		m_batch->Set_Uniform_Vector("g_tiled_repeats", Vector3(tile_repeats.X, tile_repeats.Y, 0.0f));
	}

	// Apply color/blend.
	m_batch->Set_Blend_Function(blend_mode);
	m_batch->Set_Color(color);
	m_batch->Set_Uniform_Vector("g_object_mask", object_mask);
	//m_batch->Set_Object_Mask(object_mask);

	// Draw a rotated rectangle.
	if (rotation != 0)
	{
		// Transformation.
		float s = sinf(DegToRad(rotation));
		float c = cosf(DegToRad(rotation));
		float ix = c * scale.X;
		float iy =-s * scale.Y;
		float jx = s * scale.X;
		float jy = c * scale.Y;

		// Position
		float x0 = -origin.X;
		float y0 = -origin.Y;
		float x1 = -origin.X + point.Width;
		float y1 = -origin.Y + point.Height;
		float tx = point.X;
		float ty = point.Y;

		// Calculate UV's (with V inverted) TODO: Sort this shit out, we do this in a bunch of places and its opengl specific D:
		float uv_left	= uv.X;
		float uv_right	= uv.X + uv.Width;
		float uv_top	= 1.0f - uv.Y;
		float uv_bottom = uv_top - uv.Height;

		// Top-Left
		Vector3 v1
			(
				x0 * ix + y0 * iy + tx, 
				x0 * jx + y0 * jy + ty, 
				depth
			);
		Vector2 uv1(uv_left, uv_top);

		// Top-Right
		Vector3 v2
			(
				x1 * ix + y0 * iy + tx, 
				x1 * jx + y0 * jy + ty, 
				depth
			);
		Vector2 uv2(uv_right, uv_top);

		// Bottom-Right
		Vector3 v3
			(
				x1 * ix + y1 * iy + tx, 
				x1 * jx + y1 * jy + ty, 
				depth
			);
		Vector2 uv3(uv_right, uv_bottom);

		// Bottom-Left
		Vector3 v4
			(
				x0 * ix + y1 * iy + tx, 
				x0 * jx + y1 * jy + ty, 
				depth
			);
		Vector2 uv4(uv_left, uv_bottom);

		m_batch->Draw_Quad(v1, uv1, 
						   v4, uv4, 
						   v3, uv3, 
						   v2, uv2);
	}

	// Draw a normal rectangle.
	else
	{
		// Calculate AABB of quad to render.
		Rect2D rect = Rect2D(point.X - (origin.X * scale.X), point.Y - (origin.Y * scale.Y), point.Width * scale.X, point.Height * scale.Y);

		m_batch->Draw_Quad(rect, uv, depth);
	}

	// Disable tiling?
	if (tiled == true)
	{
		m_batch->Set_Uniform_Bool("g_tiled", false);
	}

	//m_batch->Set_Object_Mask(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	m_batch->Set_Uniform_Vector("g_object_mask", Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	// End batch if neccessary.
	if (m_batched == false)
	{
		End();
	}
}

void AtlasRenderer::Draw_Frame(AtlasFrame* frame, Rect2D point, float depth, Color color, bool flip_horizontal, bool flip_vertical, RendererOption::Type blend_mode, Vector2 scale, float rotation, Vector4 object_mask)
{
	Draw(frame->TexturePtr->TexturePtr, frame->UV, frame->Origin, point, depth, color, flip_horizontal, flip_vertical, blend_mode, scale, rotation, object_mask);
}

void AtlasRenderer::Draw_Frame(AtlasFrame* frame, Vector2 point, float depth, Color color, bool flip_horizontal, bool flip_vertical, RendererOption::Type blend_mode, Vector2 scale, float rotation, Vector4 object_mask)
{
	Draw_Frame(frame, Rect2D(point.X, point.Y, frame->Rect.Width, frame->Rect.Height), depth, color, flip_horizontal, flip_vertical, blend_mode, scale, rotation, object_mask);
}

void AtlasRenderer::Draw_Frame(const char* frame, Vector2 point, float depth, Color color, bool flip_horizontal, bool flip_vertical, RendererOption::Type blend_mode, Vector2 scale, float rotation, Vector4 object_mask)
{
	AtlasFrame* f = ResourceFactory::Get()->Get_Atlas_Frame(frame);//m_atlas->Get()->Get_Frame(frame);
	if (f == NULL)
	{
	//	DBG_LOG("Cannot draw unknown frame '%s'.", frame);
		return;
	}
	Draw_Frame(f, Rect2D(point.X, point.Y, f->Rect.Width, f->Rect.Height), depth, color, flip_horizontal, flip_vertical, blend_mode, scale, rotation, object_mask);
}

void AtlasRenderer::Draw_Frame(const char* frame, Rect2D  point, float depth, Color color, bool flip_horizontal, bool flip_vertical, RendererOption::Type blend_mode, Vector2 scale, float rotation, Vector4 object_mask)
{
	AtlasFrame* f = ResourceFactory::Get()->Get_Atlas_Frame(frame);//m_atlas->Get()->Get_Frame(frame);
	if (f == NULL)
	{
	//	DBG_LOG("Cannot draw unknown frame '%s'.", frame);
		return;
	}
	Draw_Frame(f, point, depth, color, flip_horizontal, flip_vertical, blend_mode, scale, rotation, object_mask);
}

void AtlasRenderer::Tile_Frame(AtlasFrame* frame, Rect2D point, float depth, Color color, bool flip_horizontal, bool flip_vertical, RendererOption::Type blend_mode, Vector2 scale, float rotation, Vector4 object_mask)
{
	Rect2D uv = frame->UV;
	Vector2 repeats = Vector2(
		point.Width / (float)frame->Rect.Width,
		point.Height / (float)frame->Rect.Height
	);

	Draw(frame->TexturePtr->TexturePtr, uv, frame->Origin, point, 0.0f, color, flip_horizontal, flip_vertical, blend_mode, scale, rotation, object_mask, true, repeats);
}