// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_SHADERHANDLE_
#define _ENGINE_RENDERER_SHADERHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Localise/Language.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

// The language handle wraps a audio bank class instance, and automatically
// reloads the audio bank if the source file is changed.

class ShaderFactory;

class ShaderProgramHandle : public Reloadable
{
	MEMORY_ALLOCATOR(ShaderProgramHandle, "Localise"); 

private:

	ShaderProgram*		m_bank;
	std::string			m_url;

protected:

	friend class ShaderFactory;

	// Only texture factory should be able to modify these!
	ShaderProgramHandle(const char* url, ShaderProgram* sound);
	~ShaderProgramHandle();

	void Reload();

public:

	// Get/Set
	ShaderProgram* Get();
	std::string Get_URL();

};

#endif

