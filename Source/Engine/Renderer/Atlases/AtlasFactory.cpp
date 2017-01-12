// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Atlases/AtlasFactory.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Helper/StringHelper.h"

ThreadSafeHashTable<AtlasHandle*, int>	AtlasFactory::m_loaded_atlases;

AtlasHandle* AtlasFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Atlas already loaded?
	if (m_loaded_atlases.Contains(url_hash))
	{
		DBG_LOG("Loaded atlas from cache: %s", url);
		return m_loaded_atlases.Get(url_hash);
	}

	// Try and load atlas!
	Atlas* atlas = Load_Without_Handle(url);
	if (atlas != NULL)
	{		
		AtlasHandle* handle = new AtlasHandle(url, atlas);

		m_loaded_atlases.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Atlas* AtlasFactory::Load_Without_Handle(const char* url)
{
	Atlas* result = Load_Atlas(url);
	if (result != NULL)
	{
		DBG_LOG("Loaded atlas: %s", url);
		return result;
	}
	else
	{
		DBG_LOG("Failed to load atlas: %s", url);
		return NULL;
	}
}

Atlas* AtlasFactory::Load_Atlas(const char* url)
{
	Stream* chunk_stream = ResourceFactory::Get()->Open(url, StreamMode::Read);
	if (chunk_stream == NULL)
	{
		return NULL;
	}

	PatchedBinaryStream* stream = new PatchedBinaryStream(chunk_stream, StreamMode::Read);
	SAFE_DELETE(chunk_stream);

	CompiledAtlasHeader* header = (CompiledAtlasHeader*)stream->Take_Data();

	Atlas* atlas = new Atlas();
	if (!atlas->Load_Compiled_Config(header))
	{
		SAFE_DELETE(stream);
		SAFE_DELETE(atlas);
		return NULL;
	}

	stream->Close();
	SAFE_DELETE(stream);

	return atlas;
}

AtlasFactory::AtlasFactory()
{
}

