// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Shaders/ShaderFactory.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"
#include "Engine/Renderer/Shaders/ShaderProgramHandle.h"

#include "Engine/Renderer/Renderer.h"

#include "Engine/Resources/ResourceFactory.h"
#include "Engine/IO/PatchedBinaryStream.h"

ThreadSafeHashTable<ShaderProgramHandle*, int> ShaderFactory::m_loaded_banks;

ShaderProgramHandle* ShaderFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_banks.Contains(url_hash))
	{
		DBG_LOG("Loaded shader from cache: %s", url);
		return m_loaded_banks.Get(url_hash);
	}

	// Try and load texture!
	ShaderProgram* sound = Load_Without_Handle(url);
	if (sound != NULL)
	{		
		ShaderProgramHandle* handle = new ShaderProgramHandle(url, sound);

		m_loaded_banks.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

ShaderProgram* ShaderFactory::Load_Without_Handle(const char* url)
{
	ShaderProgram* result = Try_Load(url);
	if (result != NULL)
	{
		DBG_LOG("Loaded shader: %s", url);
		return result;
	}

	DBG_LOG("Failed to load shader: %s", url);
	return NULL;
}

ShaderProgram* ShaderFactory::Try_Load(const char* url)
{
	Platform* platform = Platform::Get();

	Stream* chunk_stream = ResourceFactory::Get()->Open(url, StreamMode::Read);
	if (chunk_stream == NULL)
	{
		return NULL;
	}

	PatchedBinaryStream* stream = new PatchedBinaryStream(chunk_stream, StreamMode::Read);
	SAFE_DELETE(chunk_stream);

	CompiledShaderHeader* header = (CompiledShaderHeader*)stream->Take_Data();

	// Load compiled config.
	ShaderProgram* program = Renderer::Get()->Create_Shader_Program(header);
	if (program == NULL)
	{
		DBG_LOG("Failed to load shader from '%s'.", url);
		SAFE_DELETE(stream);
		SAFE_DELETE(header);
		return NULL;
	}

	stream->Close();
	SAFE_DELETE(stream);

	DBG_LOG("Finished loading shader from '%s'.", url);
	return program;
}

void ShaderFactory::Dispose()
{
	for (ThreadSafeHashTable<ShaderProgramHandle*, int>::Iterator iter = m_loaded_banks.Begin(); iter != m_loaded_banks.End(); iter++)
	{
		ShaderProgramHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_banks.Clear();
}
