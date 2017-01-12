// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Renderer/OpenGL/OpenGL_RenderQuery.h"

#include "Engine/Platform/Platform.h"

#ifndef GL_TIME_ELAPSED
#define GL_TIME_ELAPSED 0x88BF
#endif

#ifndef GL_TIMESTAMP
#define GL_TIMESTAMP 0x8E28
#endif

OpenGL_RenderQuery::OpenGL_RenderQuery(RenderQueryType::Type type)
	: m_got_query_result(false)
{
#ifndef MASTER_BUILD
	m_bSupported = Renderer::Get()->Supports(RendererFeature::Querys);
#else
	m_bSupported = false;
#endif

#ifndef PLATFORM_MACOS

	if (m_bSupported)
	{	
		switch (type)
		{
		case RenderQueryType::Time:
			{
				glGenQueries(2, m_query_ids);
				break;
			}
		}
	}

#endif
}

OpenGL_RenderQuery::~OpenGL_RenderQuery()
{
#ifndef PLATFORM_MACOS

	if (m_bSupported)
	{
		glDeleteQueries(2, m_query_ids);
	}

#endif
}

void OpenGL_RenderQuery::Begin()
{
#ifndef PLATFORM_MACOS

	if (m_bSupported)
	{
		glQueryCounter(m_query_ids[0], GL_TIMESTAMP);
		m_got_query_result = false;
		CHECK_GL_ERROR();
	}

#endif
}

void OpenGL_RenderQuery::Finish()
{
#ifndef PLATFORM_MACOS

	if (m_bSupported)
	{
		glQueryCounter(m_query_ids[1], GL_TIMESTAMP);
		CHECK_GL_ERROR();
	}

#endif
}

float OpenGL_RenderQuery::Get_Result()
{
#ifndef PLATFORM_MACOS

	if (m_bSupported)
	{
		if (m_got_query_result == false)
		{
			// Wait till result is available.
			unsigned int bResultAvailable = 0;
			bool bStalling = false;
			double start_ticks = Platform::Get()->Get_Ticks();

			while (!bResultAvailable) 
			{
				glGetQueryObjectuiv(m_query_ids[0], GL_QUERY_RESULT_AVAILABLE, &bResultAvailable);
				if (!bResultAvailable)
				{
					bStalling = true;
				}
			}

			bResultAvailable = false;

			while (!bResultAvailable) 
			{
				glGetQueryObjectuiv(m_query_ids[1], GL_QUERY_RESULT_AVAILABLE, &bResultAvailable);
				if (!bResultAvailable)
				{
					bStalling = true;
				}
			}

			if (bStalling)
			{
				double elapsed_ticks = Platform::Get()->Get_Ticks() - start_ticks;
				DBG_LOG("Stalled game thread %.2f ms waiting for GPU query result.", elapsed_ticks);
			}

			// Get the result of query.
#ifdef PLATFORM_MACOS
			// Sigh macos aliases the 64bit versions of these to 32bit.
			glGetQueryObjectiv(m_query_ids[0], GL_QUERY_RESULT, &m_query_results[0]);
			glGetQueryObjectiv(m_query_ids[1], GL_QUERY_RESULT, &m_query_results[1]);
#else
			glGetQueryObjecti64v(m_query_ids[0], GL_QUERY_RESULT, &m_query_results[0]);
			glGetQueryObjecti64v(m_query_ids[1], GL_QUERY_RESULT, &m_query_results[1]);
#endif

			m_got_query_result = true;
		}

		return (m_query_results[1] - m_query_results[0]) / 1000000.0f;
	}
	else
	{
		return 0.0f;
	}

#else
	return 0.0f;

#endif
}

#endif