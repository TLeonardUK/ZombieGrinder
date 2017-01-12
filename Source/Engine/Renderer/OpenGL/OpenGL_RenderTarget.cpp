// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Renderer/OpenGL/OpenGL_RenderTarget.h"
#include "Engine/Renderer/OpenGL/OpenGL_Texture.h"

OpenGL_RenderTarget::OpenGL_RenderTarget(GLuint target_id) 
	: RenderTarget()
	, m_target_id(target_id)
	, m_depth_texture(NULL)
	, m_stencil_texture(NULL)
{
	m_supported = Renderer::Get()->Supports(RendererFeature::FBO);
	DBG_LOG("[OpenGL RenderTarget] New render target (id=%u).", target_id);
}
	
OpenGL_RenderTarget::~OpenGL_RenderTarget()
{
	if (m_supported && m_target_id >= 0)
	{
		DBG_LOG("[OpenGL RenderTarget] Deleted render target (id=%u).", m_target_id);
		glDeleteFramebuffers(1, &m_target_id);
		m_target_id = -1;
	}
}

GLuint OpenGL_RenderTarget::Get_ID()
{
	return m_target_id;
}

void OpenGL_RenderTarget::Bind_Texture(RenderTargetBufferType::Type type, const Texture* texture)
{
	if (!m_supported)
	{
		return;
	}

	const OpenGL_Texture* glTexture = dynamic_cast<const OpenGL_Texture*>(texture);
	DBG_ASSERT(glTexture != NULL);

	Renderer::Get()->Bind_Render_Target(this);

	switch (type)
	{
	case RenderTargetBufferType::Color:
		{
			DBG_LOG("[OpenGL RenderTarget] Binding color texture to render target (id=%u).", m_target_id);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_color_textures.size(), GL_TEXTURE_2D, glTexture->Get_ID(), 0);
			CHECK_GL_ERROR();

	       // GLenum DrawBuffers[2] = {GL_BACK_RIGHT, GL_COLOR_ATTACHMENT0};
		   // glDrawBuffers(2, DrawBuffers); 

			m_color_textures.push_back(glTexture);
			break;
		}
	case RenderTargetBufferType::Depth:
		{
			DBG_LOG("[OpenGL RenderTarget] Binding depth texture to render target (id=%u).", m_target_id);

			DBG_ASSERT(m_depth_texture == NULL);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glTexture->Get_ID(), 0);
			CHECK_GL_ERROR();

			m_depth_texture = glTexture;
			break;
		}
	case RenderTargetBufferType::Stencil:
		{
			DBG_LOG("[OpenGL RenderTarget] Binding stencil texture to render target (id=%u).", m_target_id);

			DBG_ASSERT(m_stencil_texture == NULL);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTexture->Get_ID(), 0);
			CHECK_GL_ERROR();

			m_stencil_texture = glTexture;
			break;
		}
	default:
		{
			// Format not supported.
			DBG_ASSERT(false);
			break;
		}
	}
}

void OpenGL_RenderTarget::Validate()
{
	if (!m_supported)
	{
		return;
	}

	DBG_ASSERT(Renderer::Get()->Supports(RendererFeature::FBO));

	Renderer::Get()->Bind_Render_Target(this);

	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	CHECK_GL_ERROR();

	DBG_LOG("[OpenGL RenderTarget] Render target (id=%u) status is %u.", m_target_id, status);

	DBG_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
}

#endif