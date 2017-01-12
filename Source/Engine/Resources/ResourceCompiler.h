// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_RESOURCECOMPILER_
#define _ENGINE_RESOURCES_RESOURCECOMPILER_

#include "Generic/Types/LinkedList.h"
#include "Engine/IO/Stream.h"
#include "Engine/IO/FileWatcher.h"

#include "Engine/Renderer/Shaders/Shader.h"

#include "Engine/Platform/Platform.h"

#include <vector>

class GameEngine;

class ResourceCompiler
{
	MEMORY_ALLOCATOR(ResourceCompiler, "Engine");

private:

protected:

public:

	// Constructors
	ResourceCompiler();	
	virtual ~ResourceCompiler();	

	// General methods.
	std::string Directory_To_Output_Directory(std::string directory);
	bool Check_File_Timestamp(std::string path);
	void Update_File_Timestamp(std::string path);

	// Derived methods.
	virtual bool		Should_Compile		() = 0;
	virtual std::string Get_Compiled_Path	() = 0;
	virtual bool		Compile				() = 0;

};

#endif

