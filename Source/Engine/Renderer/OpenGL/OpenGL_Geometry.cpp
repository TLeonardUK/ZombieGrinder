// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Renderer/OpenGL/OpenGL_Geometry.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

int g_opengl_data_type_sizes[] = {
	4,	// Float
	8,	// Float2
	12,	// Float3
	16,	// Float4
	2,	// Half
	4,	// Half2
	6,	// Half3
	8,	// Half4
};

int g_opengl_data_type_element_count[] = {
	1,	// Float
	2,	// Float2
	3,	// Float3
	4,	// Float4
	1,	// Half
	2,	// Half2
	3,	// Half3
	4,	// Half4
};

int g_opengl_data_type_gl_format[] = {
	GL_FLOAT,		// Float
	GL_FLOAT,		// Float2
	GL_FLOAT,		// Float3
	GL_FLOAT,		// Float4
	GL_HALF_FLOAT,	// Half
	GL_HALF_FLOAT,	// Half2
	GL_HALF_FLOAT,	// Half3
	GL_HALF_FLOAT,	// Half4
};

int g_opengl_verts_per_primitive[] = {
	0,	// None
	3,	// Triangle
	2,	// Line
	4,	// Quad
};

int g_opengl_element_client_state[] = {
	GL_VERTEX_ARRAY,			// Position,
	GL_TEXTURE_COORD_ARRAY,		// TexCoord,
	GL_COLOR_ARRAY,				// Color
};

OpenGL_Geometry::OpenGL_Geometry(GeometryBufferFormat format, PrimitiveType::Type type, int primitives, bool dynamic)
	: m_format(format)
	, m_primitive_type(type)
	, m_max_primitive_count(primitives)
	, m_buffer(0)
	, m_stride(0)
	, m_data_size(0)
	, m_max_vertex_count(0)
	, m_primitive_count(0)
	, m_vertex_count(0)
	, m_locked(false)
	, m_data_store_created(false)
	, m_dynamic(dynamic)
{
	glGenBuffers(1, &m_buffer);
	CHECK_GL_ERROR();

	for (int i = 0; i < format.element_count; i++)
	{
		m_element_offset[i] = m_stride;
		m_stride += g_opengl_data_type_sizes[format.data_types[i]];
	}

	m_data_size = m_stride * g_opengl_verts_per_primitive[type] * m_max_primitive_count;
	m_data = new char[m_data_size];

	m_max_vertex_count = g_opengl_verts_per_primitive[type] * m_max_primitive_count;

	// Work out which client states we need to enable for this geometry.
	for (int i = 0; i < GeometryBufferElementType::COUNT; i++)
	{
		bool enabled = false;		
		for (int j = 0; j < m_format.element_count; j++)
		{
			if (m_format.element_types[j] == i)
			{
				enabled = true;
				break;
			}
		}

		m_client_states_enabled[i] = enabled;
	}

	DBG_LOG("Allocated new geometry of size %i kb (%i primitives, %i verts, %i stride)", m_data_size / 1024, m_max_primitive_count, m_max_vertex_count, m_stride);
}

OpenGL_Geometry::~OpenGL_Geometry()
{
	if (m_buffer > 0)
	{
		OpenGL_Renderer* renderer = static_cast<OpenGL_Renderer*>(OpenGL_Renderer::Get());
		renderer->DeferBufferDelete(m_buffer);
		m_buffer = 0;
	}

	SAFE_DELETE_ARRAY(m_data);
}

int OpenGL_Geometry::Get_Stride()
{
	return m_stride;
}

int OpenGL_Geometry::Get_Max_Primitives()
{
	return m_max_primitive_count;
}

void OpenGL_Geometry::Render()
{
	Render_Partial(0, m_primitive_count);
}

void OpenGL_Geometry::Render_Partial(int offset, int count)
{
	OpenGL_Renderer* renderer = static_cast<OpenGL_Renderer*>(OpenGL_Renderer::Get());

	DBG_ASSERT(!m_locked);
	DBG_ASSERT(offset + count <= m_max_primitive_count && offset >= 0);

	if (count == 0)
	{
		return;
	}

	// Enable correct client states.
	for (int i = 0; i < GeometryBufferElementType::COUNT; i++)
	{
		renderer->Set_Client_State_Enabled(g_opengl_element_client_state[i], m_client_states_enabled[i]);
	}

	// Setup buffer pointers.
	int element_offset = 0;
		
	renderer->Bind_Array_Buffer(m_buffer);
//	glBindBuffer(GL_ARRAY_BUFFER_ARB, m_buffer);
//	CHECK_GL_ERROR();

	for (int i = 0; i < m_format.element_count; i++)
	{
		int data_type = m_format.data_types[i];
		int gl_element_count = g_opengl_data_type_element_count[data_type];
		int gl_format = g_opengl_data_type_gl_format[data_type];

		switch (m_format.element_types[i])
		{
		case GeometryBufferElementType::Position:		renderer->Bind_Vertex_Pointer(gl_element_count, gl_format, m_stride, reinterpret_cast<GLvoid*>(element_offset));		break;
		case GeometryBufferElementType::TexCoord:		renderer->Bind_TexCoord_Pointer(gl_element_count, gl_format, m_stride, reinterpret_cast<GLvoid*>(element_offset));		break;
		case GeometryBufferElementType::Color:			renderer->Bind_Color_Pointer(gl_element_count, gl_format, m_stride, reinterpret_cast<GLvoid*>(element_offset));			break;
		}
		CHECK_GL_ERROR();

		element_offset += g_opengl_data_type_sizes[data_type];
	}

	// Work out verts to render.
	int vert_offset = offset * g_opengl_verts_per_primitive[m_primitive_type];
	int vert_count  = count * g_opengl_verts_per_primitive[m_primitive_type];

	// Draw the primitives!
	switch (m_primitive_type)
	{
	case PrimitiveType::Quad:			glDrawArrays(GL_QUADS,		vert_offset,	vert_count);			break;
	case PrimitiveType::Triangle:		glDrawArrays(GL_TRIANGLES,	vert_offset,	vert_count);			break;
	case PrimitiveType::Line:			glDrawArrays(GL_LINES,		vert_offset,	vert_count);			break;
	default:							DBG_ASSERT_STR(false, "Invalid geometry primitive type.");
	}
	CHECK_GL_ERROR();

	// Unbind our buffer.
	renderer->Bind_Array_Buffer(NULL);
//	glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
//	CHECK_GL_ERROR();
}

void* OpenGL_Geometry::Lock_Vertices()
{
	DBG_ASSERT(m_locked == false);
	m_locked = true;
	return m_data;
}

void OpenGL_Geometry::Unlock_Vertices(void* vertices_end)
{
	DBG_ASSERT(m_locked == true);
	m_locked = false;
	
	// Calculate vertex count based on the end point of the vertex buffer written.
	m_vertex_count = ((char*)vertices_end - (char*)m_data) / m_stride;
	m_primitive_count = m_vertex_count / g_opengl_verts_per_primitive[m_primitive_type];

	DBG_ASSERT((m_vertex_count % g_opengl_verts_per_primitive[m_primitive_type]) == 0);

	if (m_vertex_count == 0)
		return;
	
	// TODO: If double buffered, flip buffers here.

	// Upload buffer data.
	glBindBuffer(GL_ARRAY_BUFFER_ARB, m_buffer);
	CHECK_GL_ERROR();

	// Create entire data store (or just do this everytime if we don't have glBufferSubData for some fucked reason).
	if (m_data_store_created == false || glBufferSubData == NULL)
	{
		glBufferData(GL_ARRAY_BUFFER_ARB, m_data_size, m_data, m_dynamic ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
		m_data_store_created = true;
	}
	// Or just update the verticies that have changed.
	else
	{
		glBufferSubData(GL_ARRAY_BUFFER_ARB, 0, m_vertex_count * m_stride, m_data);
	}
	CHECK_GL_ERROR();
	
	//DBG_LOG("Updated geometry buffer - %i verts - %i size", m_vertex_count, m_data_size);

	// And unbind, as we no longer want to do anything with it.
	glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	CHECK_GL_ERROR();
}

#endif