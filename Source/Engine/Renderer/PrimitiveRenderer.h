// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_QUADRENDERER_
#define _ENGINE_QUADRENDERER_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Color.h"

#include "Engine/Engine/FrameTime.h"

class Texture;
class TextureHandle;

// Just wraps up functionality for rendering several basic primitives.
class PrimitiveRenderer
{
	MEMORY_ALLOCATOR(PrimitiveRenderer, "Rendering");

public:
	PrimitiveRenderer();

	void Draw_Solid_Quad		(Rect2D xy, Color color);
	void Draw_Wireframe_Quad	(Rect2D xy, float depth, Color color, float size);
	void Draw_Solid_Oval		(Rect2D xy, Color color);
	void Draw_Line				(Vector3 start_zyz, Vector3 end_xyz, float size, Color color);
	void Draw_Cube				(Vector3 size, Color color);
	void Draw_Wireframe_Cube	(Vector3 size, float line_size, Color color);
	void Draw_Wireframe_Sphere	(float r, float line_size, Color color);
	void Draw_Arrow				(Vector3 direction, float line_length = 0.2f, float pip_length = 0.04f, float line_size = 1.0f, Color color = Color::White);

};

#endif

