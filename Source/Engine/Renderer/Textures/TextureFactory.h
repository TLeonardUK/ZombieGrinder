// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTUREFACTORY_
#define _ENGINE_RENDERER_TEXTURES_TEXTUREFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Engine/IO/Stream.h"

#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/TextureHandle.h"

class TextureFactory
{
	MEMORY_ALLOCATOR(TextureFactory, "Rendering");

private:
	static ThreadSafeHashTable<TextureHandle*, int> m_loaded_textures;
	
	// Static class!
	virtual ~TextureFactory() {} 

public:
		
	// Dispose.
	static void Dispose();

	// Static methods.
	static bool			  Save				 (const char* url, Texture* texture, TextureFlags::Type flags);
	static TextureHandle* Load				 (const char* url, TextureFlags::Type flags);
	static Texture*		  Load_Without_Handle(const char* url, TextureFlags::Type flags);

};

#endif

