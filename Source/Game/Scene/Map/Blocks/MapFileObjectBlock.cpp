// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Scripts/GameVirtualMachine.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Helper/StringHelper.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Network/GameNetManager.h"

#include "Engine/IO/Stream.h"
#include "Engine/IO/BinaryStream.h" 

#include "Engine/Scene/Scene.h"

MapFileObjectBlock::MapFileObjectBlock()
{
}

MapFileObjectBlock::~MapFileObjectBlock()
{
}

bool MapFileObjectBlock::Decode(MapVersion::Type version, Stream* stream)
{
	Class_Name = stream->ReadNullTerminatedString();

	Serialized_Data_Version = stream->Read<int>();

	int size = stream->Read<int>();
	Serialized_Data.Reserve(size, false);
	stream->ReadBuffer(Serialized_Data.Buffer(), 0, size);

	return true;
}

bool MapFileObjectBlock::Encode(MapVersion::Type version, Stream* stream)
{
	stream->WriteNullTerminatedString(Class_Name.c_str());

	stream->Write<int>(Serialized_Data_Version);
	stream->Write<int>(Serialized_Data.Size());
	stream->WriteBuffer(Serialized_Data.Buffer(), 0, Serialized_Data.Size());

	return true;
}

u32 MapFileObjectBlock::Get_ID()
{
	return COMBINE_MAP_FILE_BLOCK_ID('S', 'O', 'B', 'J');
}

ScriptedActor* MapFileObjectBlock::Create_Instance()
{	
	// Grab VM classes we will be using.
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* class_symbol = vm->Find_Class(Class_Name.c_str());
	DBG_ASSERT(class_symbol != NULL);

	// If replicated and we are not the server, then don't create it.
	if (class_symbol->symbol->class_data->is_replicated != 0)
	{
		if (GameNetManager::Get()->Server() == NULL)
		{
			return NULL;
		}
	}

	// Create the object!
	ScriptedActor* actor = new ScriptedActor(vm, class_symbol);
	actor->Setup_Script();
	actor->Set_Position(Vector3(0, 0, 0));

	// Deserialize objects properties.
	CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

	if (Serialized_Data.Size() > 0)
	{
		BinaryStream bs(Serialized_Data.Buffer(), Serialized_Data.Size());
		actor->Get_Script_Object().Get()->Deserialize(&bs, CVMObjectSerializeFlags::Full, Serialized_Data_Version);
		bs.Close();
	}

	//DBG_LOG("NEW OBJECT: %s", class_symbol->symbol->name);

	// Change to default state.
	vm->Set_Default_State(actor->Get_Script_Object());
	actor->Setup_Event_Hooks();
	
	// Add to scene.	
	//Scene* scene = GameEngine::Get()->Get_Scene();
	//scene->Add_Actor(actor);
	//scene->Add_Tickable(actor);
					
	return actor;
}
