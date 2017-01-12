// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TYPES_
#define _ENGINE_RENDERER_TYPES_

struct ShaderType
{
	enum Type
	{
		Vertex,
		Fragment
	};
};

struct RendererOption
{
	enum Type
	{
#define STATE_SETTING(setting_name, setting_enum) \
	setting_enum, 
#include "Engine/Renderer/Renderer_RenderOptions.inc"
#undef STATE_SETTING
	};
};

struct PrimitiveType
{
	enum Type
	{
		None = 0,
		Triangle,
		Line,
		Quad
	};
};

// Can be combined into a buffer format
// descriptor. eg; Vertex|Color expects packed data like so;
//
//		Vertex
//		Color
//		Vertex
//		Color
//
struct PrimitiveBufferFormat
{
	enum Type
	{
		Vertex	 = 1,
		Color	 = 2,
		Normal	 = 4,
		TexCoord = 8,

		Vertex_Color_TexCoord	= Vertex|Color|TexCoord,
		Vertex_TexCoord			= Vertex|TexCoord,
		Vertex_Color			= Vertex|Color,
	};
};

#endif

