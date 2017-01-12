// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_RENDERER_OPENGL_RENDERQUERY_
#define _ENGINE_RENDERER_OPENGL_RENDERQUERY_

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderQuery.h"

#include "Engine/Renderer/OpenGL/OpenGL_Renderer.h"

class OpenGL_RenderQuery : public RenderQuery
{
	MEMORY_ALLOCATOR(OpenGL_RenderQuery, "Rendering");

private:
	unsigned int m_query_ids[2];

#ifdef PLATFORM_MACOS
	GLint m_query_results[2];
#else
	GLint64 m_query_results[2];
#endif

	bool m_got_query_result;

	bool m_bSupported;

public:

	OpenGL_RenderQuery(RenderQueryType::Type type);
	~OpenGL_RenderQuery();

	void Begin();
	void Finish();
	float Get_Result();

};

#endif

#endif