// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_RENDERER_OPENGL_GEOMETRY_
#define _ENGINE_RENDERER_OPENGL_GEOMETRY_

#include "Engine/Renderer/Geometry.h"

#include "Engine/Renderer/OpenGL/OpenGL_Renderer.h"

class OpenGL_Geometry : public Geometry
{
	MEMORY_ALLOCATOR(OpenGL_Geometry, "Rendering");

private:
	friend class OpenGL_Renderer;

	GeometryBufferFormat		m_format;
	PrimitiveType::Type			m_primitive_type;
	int							m_max_primitive_count;
	int							m_max_vertex_count;

	GLuint						m_buffer;
	int							m_data_size;
	char*						m_data;
	bool						m_locked;
	bool						m_data_store_created;
	
	int							m_primitive_count;
	int							m_vertex_count;

	int							m_stride;
	int							m_element_offset[GeometryBufferFormat::max_elements];

	bool						m_client_states_enabled[GeometryBufferElementType::COUNT];

	bool						m_dynamic;

private:

	// Constructor!
	OpenGL_Geometry(GeometryBufferFormat format, PrimitiveType::Type type, int primitives, bool dynamic);

public:

	// Destructor!
	~OpenGL_Geometry();
			
	int Get_Stride();
	int Get_Max_Primitives();

	void Render();
	void Render_Partial(int offset, int count);
	
	void* Lock_Vertices();
	void Unlock_Vertices(void* vertices_end);

};

#endif

#endif