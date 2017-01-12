// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Text/FreeType/FreeType_Font.h"
#include "Engine/Renderer/Text/FreeType/FreeType_FontFactory.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Resources/Compiled/Fonts/CompiledFont.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include "Engine/IO/StreamFactory.h"

FT_Library FreeType_FontFactory::m_library;

FreeType_FontFactory::FreeType_FontFactory()
{
}

FreeType_FontFactory::~FreeType_FontFactory()
{
}

FT_Library FreeType_FontFactory::Get_FreeType_Library()
{
	if (m_library == NULL)
	{
		Init_FreeType();
	}
	return m_library;
}

void FreeType_FontFactory::Init_FreeType()
{
	DBG_LOG("Initializing FreeType font library ..");

	int result = FT_Init_FreeType(&m_library);
	DBG_ASSERT_STR(result == 0, "FreeType failed to initialize with error 0x%08x", result);
}

Font* FreeType_FontFactory::Try_Load(const char* url, FontFlags::Type flags)
{	
	Platform* platform = Platform::Get();

	Stream* chunk_stream = ResourceFactory::Get()->Open(url, StreamMode::Read);
	if (chunk_stream == NULL)
	{
		return NULL;
	}

	PatchedBinaryStream* stream = new PatchedBinaryStream(chunk_stream, StreamMode::Read);
	SAFE_DELETE(chunk_stream);

	CompiledFontHeader* header = (CompiledFontHeader*)stream->Take_Data();

	// Load compiled config.
	FreeType_Font* program = new FreeType_Font();
	if (program->Load_Compiled_Config(header) == false)
	{
		DBG_LOG("Failed to load font from '%s'.", url);
		SAFE_DELETE(stream);
		SAFE_DELETE(program);
		return NULL;
	}

	stream->Close();
	SAFE_DELETE(stream);

	DBG_LOG("Finished loading font from '%s'.", url);
	return program;
}