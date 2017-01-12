// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FREETYPE_FONTFACTORY_
#define _ENGINE_RENDERER_TEXT_FREETYPE_FONTFACTORY_

#include "Engine/Renderer/Text/FontFactory.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class FreeType_FontFactory : public FontFactory
{
	MEMORY_ALLOCATOR(FreeType_FontFactory, "Rendering");

private:
	static FT_Library m_library;

public:
	static FT_Library Get_FreeType_Library();
	static void Init_FreeType();
	
	FreeType_FontFactory();
	~FreeType_FontFactory();
	
	Font* Try_Load(const char* url, FontFlags::Type flags);

};

#endif

