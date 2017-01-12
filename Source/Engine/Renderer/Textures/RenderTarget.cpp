// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/RenderTarget.h"

RenderTarget::RenderTarget()
{
}

RenderTarget::~RenderTarget()
{
}

RenderTarget* RenderTarget::Create()
{
	return Renderer::Get()->Create_Render_Target();
}