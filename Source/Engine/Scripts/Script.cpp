// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scripts/Script.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/IO/PatchedBinaryStream.h"
#include "Engine/Platform/Platform.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "XScript/VirtualMachine/CVMBinary.h"

Script* Script::Load(const char* url)
{
	Platform* platform = Platform::Get();

	Stream* chunk_stream = ResourceFactory::Get()->Open(url, StreamMode::Read);
	if (chunk_stream == NULL)
	{
		return NULL;
	}

	PatchedBinaryStream* stream = new PatchedBinaryStream(chunk_stream, StreamMode::Read);
	
//	static int total = 0;
//	total += stream->Length();
//	DBG_LOG("TOTAL SCRIPT DATA: %.2f MB", total / 1024.0f / 1024.0f);

	CVMBinary* binary = new CVMBinary();
	binary->Deserialize(stream);
	
	stream->Close();
	chunk_stream->Close();

	SAFE_DELETE(stream);
	SAFE_DELETE(chunk_stream);

	Script* script = new Script();
	script->m_binary = binary;
	script->m_name = url;	

	DBG_LOG("Loaded Script: %s", script->Get_Name().c_str());

	/*{
		GameEngine::Get()->Get_VM()->Link_Script(binary);
		DBG_LOG("Linked Script: %s", script->Get_Name().c_str());

		script->m_binary = NULL;
		SAFE_DELETE(binary);
	}*/

	return script;
}

Script::Script()
{
}

Script::~Script()
{
	SAFE_DELETE(m_binary);
}

std::string Script::Get_Name()
{
	return m_name;
}

CVMBinary* Script::Get_Binary()
{
	return m_binary;
}