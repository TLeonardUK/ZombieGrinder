// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PixelmapFACTORY_
#define _ENGINE_RENDERER_TEXTURES_PixelmapFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/HashTable.h"
#include "Engine/IO/Stream.h"

#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Textures/Pixelmap.h"

class PixelmapFactory
{
	MEMORY_ALLOCATOR(PixelmapFactory, "Rendering");

private:
	static LinkedList<PixelmapFactory*> m_factories;

public:
		
	// Static methods.
	static bool	   Save	(const char* url, Pixelmap* texture);
	static bool	   Save	(Stream* stream, Pixelmap* texture);
	static Pixelmap* Load	(const char* url);
	static Pixelmap* Load	(Stream* stream);
	
	// Constructors
	PixelmapFactory();	
	virtual ~PixelmapFactory();	

	// Derived factory methods.
	virtual Pixelmap*  Try_Load(const char* url) = 0;
	virtual Pixelmap*  Try_Load(Stream* url) = 0;
	virtual bool     Try_Save(const char* url, Pixelmap* texture) = 0;
	virtual bool     Try_Save(Stream* url, Pixelmap* texture) = 0;

};

#endif

