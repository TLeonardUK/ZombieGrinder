// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_NULL_RENDERTARGET_
#define _ENGINE_RENDERER_NULL_RENDERTARGET_

#include "Engine/Renderer/Textures/RenderTarget.h"

#include "Engine/Renderer/Null/Null_Renderer.h"

#include <vector>

class Null_Texture;

class Null_RenderTarget : public RenderTarget
{
	MEMORY_ALLOCATOR(Null_RenderTarget, "Rendering");

private:
	friend class Null_Renderer;

	std::vector<const Null_Texture*>	m_color_textures;
	const Null_Texture*					m_depth_texture;
	const Null_Texture*					m_stencil_texture;

private:
	Null_RenderTarget();

public:
		
	// Binding support.
	void Bind_Texture(RenderTargetBufferType::Type type, const Texture* texture);
	void Validate();

	// Destructor!
	~Null_RenderTarget();

};

#endif

