// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/PrimitiveRenderer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/RenderBatch.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"

#include "Generic/Math/Math.h"

PrimitiveRenderer::PrimitiveRenderer()
{
}

void PrimitiveRenderer::Draw_Solid_Quad(Rect2D xy, Color color)
{
	RenderPipeline* pipeline = RenderPipeline::Get();
	RenderBatch* batch = pipeline->Get_Render_Batch(NULL, PrimitiveType::Quad);

	batch->Set_Alpha_Test(true);
	batch->Set_Blend(true);
	batch->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);

	batch->Set_Color(color);
	batch->Draw_Quad(xy, Rect2D(0,0,1,1), 0.0f);
}

void PrimitiveRenderer::Draw_Wireframe_Quad(Rect2D rect, float depth, Color color, float size)
{
	Vector3 p1(rect.X,				rect.Y,					depth);
	Vector3 p2(rect.X + rect.Width,	rect.Y,					depth);
	Vector3 p3(rect.X + rect.Width,	rect.Y + rect.Height,	depth);
	Vector3 p4(rect.X,				rect.Y + rect.Height,	depth);
	
	Draw_Line(p1, p2, size, color);
	Draw_Line(p2, p3, size, color);
	Draw_Line(p3, p4, size, color);
	Draw_Line(p4, p1, size, color);
}

void PrimitiveRenderer::Draw_Solid_Oval(Rect2D xy, Color color)
{
	RenderPipeline* pipeline = RenderPipeline::Get();
	RenderBatch* batch = pipeline->Get_Render_Batch(NULL, PrimitiveType::Triangle);
	
	batch->Set_Alpha_Test(true);
	batch->Set_Blend(true);
	batch->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);

	batch->Set_Color(color);
	
	float xr = (xy.Width * 0.5f);
	float yr = (xy.Height * 0.5f);

	int segs = (int)ceilf(xr) + (int)ceilf(yr);
	segs = Max(segs, 12) & ~3;

	float center_x = xy.X + xr;
	float center_y = xy.Y + yr;

	float px = 0.0f;
	float py = 0.0f;

	for (int i = 0; i <= segs; i++)
	{
		float th = DegToRad(i * 360.0f / segs);
		float x = center_x + (cos(th)) * xr;
		float y = center_y - (sin(th)) * yr;

		if (i != 0)
		{
			batch->Draw_Triangle(Vector3(center_x, center_y, 0.0f), Vector2(0.0f, 0.0f),	// Center point.
								 Vector3(px, py, 0.0f), Vector2(0.0f, 0.0f),				// Previous point on outside of oval.
								 Vector3(x, y, 0.0f), Vector2(0.0f, 0.0f)					// Next point on outside of oval.
								 );
		}

		px = x;
		py = y;
	}
}

void PrimitiveRenderer::Draw_Line(Vector3 start_zyz, Vector3 end_xyz, float size, Color color)
{
	RenderPipeline* pipeline = RenderPipeline::Get();
	RenderBatch* batch = pipeline->Get_Render_Batch(NULL, PrimitiveType::Line);
	
	batch->Set_Alpha_Test(true);
	batch->Set_Blend(true);
	batch->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);

	batch->Set_Color(color);
	batch->Set_Line_Size(size);
	batch->Draw_Line(start_zyz.X, start_zyz.Y, start_zyz.Z, end_xyz.X, end_xyz.Y, end_xyz.Z);
}

void PrimitiveRenderer::Draw_Wireframe_Cube(Vector3 size, float line_size, Color color)
{
	RenderPipeline* pipeline = RenderPipeline::Get();
	RenderBatch* batch = pipeline->Get_Render_Batch(NULL, PrimitiveType::Line);

	float w = size.X;
	float h = size.Y;
	float d = size.Z;
	
	batch->Set_Alpha_Test(true);
	batch->Set_Blend(true);
	batch->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);

	batch->Set_Line_Size(line_size);
	batch->Set_Color(color);

	// Top
	batch->Draw_Line(0, 0, 0,   w, 0, 0);
	batch->Draw_Line(0, 0, 0,   0, 0, d);
	batch->Draw_Line(0, 0, d,   w, 0, d);
	batch->Draw_Line(w, 0, 0,   w, 0, d);

	// Bottom
	batch->Draw_Line(0, h, 0,   w, h, 0);
	batch->Draw_Line(0, h, 0,   0, h, d);
	batch->Draw_Line(0, h, d,   w, h, d);
	batch->Draw_Line(w, h, 0,   w, h, d);

	// Connectors.
	batch->Draw_Line(0, 0, 0,   0, h, 0);
	batch->Draw_Line(0, 0, d,   0, h, d);
	batch->Draw_Line(w, 0, d,   w, h, d);
	batch->Draw_Line(w, 0, 0,   w, h, 0);
}

void PrimitiveRenderer::Draw_Wireframe_Sphere(float r, float line_size, Color color)
{
	RenderPipeline* pipeline = RenderPipeline::Get();
	RenderBatch* batch = pipeline->Get_Render_Batch(NULL, PrimitiveType::Line);

	int lats  = 12;
	int longs = 12;
	
	batch->Set_Alpha_Test(true);
	batch->Set_Blend(true);
	batch->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);

	batch->Set_Line_Size(line_size);
	batch->Set_Color(color);

	for (int i = 0; i <= lats; i++) 
	{
		float lat0 = PI * (-0.5f + (float) (i - 1) / lats);
		float z0   = sinf(lat0);
		float zr0  =  cosf(lat0);
    
		float lat1 = PI * (-0.5f + (float) i / lats);
		float z1   = sinf(lat1);
		float zr1  = cosf(lat1);
    
		for (int j = 0; j <= longs; j++) 
		{
			float lng = 2 * PI * (float) (j - 1) / longs;
			float x = cosf(lng);
			float y = sinf(lng);
    	
			batch->Draw_Line
			(
				(x * zr0) * r, (y * zr0) * r, z0 * r,
				(x * zr1) * r, (y * zr1) * r, z1 * r
			);
		}
	}
}

void PrimitiveRenderer::Draw_Arrow(Vector3 direction, float line_length, float pip_length, float line_size, Color color)
{
	RenderPipeline* pipeline = RenderPipeline::Get();
	RenderBatch* batch = pipeline->Get_Render_Batch(NULL, PrimitiveType::Line);

	Vector3 start = Vector3(0.0f, 0.0f, 0.0f); 
	Vector3 end   = direction * line_length;
	
	batch->Set_Alpha_Test(true);
	batch->Set_Blend(true);
	batch->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);

	batch->Set_Line_Size(line_size);
	batch->Set_Color(color);
	batch->Draw_Line(start.X, start.Y, start.Z, end.X, end.Y, end.Z);

	// Draw the end "pips", I'm sure there is a far better way to do this,
	// but fuck maths. This is quick and it gets the job down for now.
	const int pip_count = 4;
	Vector3 pip_directions[pip_count] =
	{
		 start.Direction().Cross(direction),
		-start.Direction().Cross(direction),
		 start.Direction().Cross(direction).Cross(direction),
		-start.Direction().Cross(direction).Cross(direction),	
	};

	for (int i = 0; i < pip_count; i++)
	{
		Vector3 end_pip_dir = pip_directions[i];
		Vector3 end_pip		= (end + (end_pip_dir * pip_length)) - (direction * pip_length);
		batch->Draw_Line(end.X, end.Y, end.Z, end_pip.X, end_pip.Y, end_pip.Z);
	}
}
