// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_GEOMETRY_
#define _ENGINE_RENDERER_GEOMETRY_

struct GeometryBufferDataType
{
	enum Type
	{
		Float,
		Float2,
		Float3,
		Float4,		
		Half,
		Half2,
		Half3,
		Half4,

		COUNT
	};
};

struct GeometryBufferElementType
{
	enum Type
	{
		Position,
		TexCoord,
		Color,

		COUNT
	};
};

struct GeometryBufferFormat
{
	enum
	{
		max_elements = 8
	};

	GeometryBufferElementType::Type element_types[max_elements];
	GeometryBufferDataType::Type    data_types[max_elements];
	int								element_count;

	GeometryBufferFormat()
		: element_count(0)
	{
	}

	void Add(GeometryBufferElementType::Type type, GeometryBufferDataType::Type data_type)
	{
		DBG_ASSERT(element_count < max_elements);
		element_types[element_count] = type;
		data_types[element_count] = data_type;
		element_count++;
	}
};

class Geometry
{
	MEMORY_ALLOCATOR(Geometry, "Rendering");

public:
	virtual ~Geometry() {};

	virtual void Render() = 0;
	virtual void Render_Partial(int offset, int count) = 0;

	virtual int Get_Stride() = 0;
	virtual int Get_Max_Primitives() = 0;
	
	virtual void* Lock_Vertices() = 0;
	virtual void  Unlock_Vertices(void* vertices_end) = 0;

};

#endif

