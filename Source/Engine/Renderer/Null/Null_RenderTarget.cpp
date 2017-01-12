// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Null/Null_RenderTarget.h"
#include "Engine/Renderer/Null/Null_Texture.h"

Null_RenderTarget::Null_RenderTarget() 
	: RenderTarget()
	, m_depth_texture(NULL)
	, m_stencil_texture(NULL)
{
}
	
Null_RenderTarget::~Null_RenderTarget()
{
}

void Null_RenderTarget::Bind_Texture(RenderTargetBufferType::Type type, const Texture* texture)
{
	const Null_Texture* glTexture = dynamic_cast<const Null_Texture*>(texture);
	DBG_ASSERT(glTexture != NULL);

	switch (type)
	{
	case RenderTargetBufferType::Color:
		{
			m_color_textures.push_back(glTexture);
			break;
		}
	case RenderTargetBufferType::Depth:
		{
			DBG_ASSERT(m_depth_texture == NULL);
			m_depth_texture = glTexture;
			break;
		}
	case RenderTargetBufferType::Stencil:
		{
			DBG_ASSERT(m_stencil_texture == NULL);
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

void Null_RenderTarget::Validate()
{
}

