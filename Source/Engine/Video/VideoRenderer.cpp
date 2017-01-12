// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Video/VideoRenderer.h"

#include "Engine/Video/Theora/Theora_VideoRenderer.h"

VideoRenderer* VideoRenderer::Create()
{
	return new Theora_VideoRenderer();
}
