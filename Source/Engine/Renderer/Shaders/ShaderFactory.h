// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_SHADERS_SHADERFACTORY_
#define _ENGINE_RENDERER_SHADERS_SHADERFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Engine/IO/Stream.h"

#include "Engine/Renderer/Shaders/ShaderProgram.h"
#include "Engine/Renderer/Shaders/ShaderProgramHandle.h"

class ShaderFactory
{
	MEMORY_ALLOCATOR(ShaderFactory, "Rendering");

private:
	static LinkedList<ShaderFactory*> m_factories;
	static ThreadSafeHashTable<ShaderProgramHandle*, int> m_loaded_banks;

protected:
	static ShaderProgram*	Try_Load(const char* url);

public:

	// Dispose.
	static void Dispose();

	// Static methods.
	static ShaderProgramHandle* Load(const char* url);
	static ShaderProgram*		 Load_Without_Handle(const char* url);

};

#endif

