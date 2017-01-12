// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FONTFACTORY_
#define _ENGINE_RENDERER_TEXT_FONTFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Engine/IO/Stream.h"
#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontHandle.h"
#include "Engine/Renderer/Text/FontFactory.h"

class FontFactory
{
	MEMORY_ALLOCATOR(FontFactory, "Rendering");

private:
	static LinkedList<FontFactory*>		m_factories;
	static ThreadSafeHashTable<FontHandle*, int>	m_loaded_fonts;

public:
		
	// Static methods.
	static FontHandle* Load(const char* url, FontFlags::Type flags);
	static Font* Load_Without_Handle(const char* url, FontFlags:: Type flags);

	// Constructors
	FontFactory();	
	virtual ~FontFactory();	

	// Derived factory methods.
	virtual Font* Try_Load(const char* url, FontFlags::Type flags) = 0;

};

#endif

