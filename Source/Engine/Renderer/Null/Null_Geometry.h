// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_NULL_GEOMETRY_
#define _ENGINE_RENDERER_NULL_GEOMETRY_

#include "Engine/Renderer/Geometry.h"

#include "Engine/Renderer/Null/Null_Renderer.h"

class Null_Geometry : public Geometry
{
	MEMORY_ALLOCATOR(Null_Geometry, "Rendering");

private:
	friend class Null_Renderer;

private:

	// Constructor!
	Null_Geometry(GeometryBufferFormat format, PrimitiveType::Type type, int primitives, bool dynamic)
	{
	}

public:

	// Destructor!
	~Null_Geometry()
	{
	}
	
	void Render()
	{
	}
	
	int Get_Stride()
	{
		return 0;
	}

	void Render_Partial(int offset, int count)
	{
	}

	int Get_Max_Primitives()
	{
		return 0;
	}

	void* Lock_Vertices()
	{
		return NULL;
	}

	void Unlock_Vertices(void* vertices_end)
	{
	}

};

#endif

