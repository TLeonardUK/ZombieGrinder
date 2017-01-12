// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorObjectChangeStateAction.h"
#include "Game/Game/EditorManager.h"
#include "Game/Runner/Game.h"

#include "Game/Network/ActorReplicator.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Scene/Actors/ScriptedActor.h"
#include "Engine/Scene/Pathing/PathManager.h"
#include "Game/Scene/GameScene.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Map/Map.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"

EditorChangeObjectStateAction::EditorChangeObjectStateAction(NetPacket_C2S_EditorChangeObjectState* action)
	: m_action(*action)
{
}

const char* EditorChangeObjectStateAction::Get_Name()
{
	return "Change Object State";
}

void EditorChangeObjectStateAction::Undo()
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	NetPacket_C2S_EditorChangeObjectState* packet = &m_action;

	DBG_LOG("Performing object properties change undo on object %i.", packet->ID);
	
	ScriptedActor* actor = EditorManager::Get()->Get_Actor_From_Editor_ID(packet->ID);
	DBG_ASSERT(actor != NULL);
	
	CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

	if (packet->Old_Serialized_Data.Size() > 0)
	{
		BinaryStream bs(packet->Old_Serialized_Data.Buffer(), packet->Old_Serialized_Data.Size());
		actor->Get_Script_Object().Get()->Deserialize(&bs, CVMObjectSerializeFlags::Full, packet->Old_Serialized_Data_Version);
		bs.Close();

		actor->Get_Event_Listener()->On_Editor_Property_Change();
	}

	PathManager::Get()->Reset();
}

void EditorChangeObjectStateAction::Do()
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	NetPacket_C2S_EditorChangeObjectState* packet = &m_action;

	DBG_LOG("Performing object properties change on object %i (state size %i).", packet->ID, packet->Serialized_Data.Size());
	
	ScriptedActor* actor = EditorManager::Get()->Get_Actor_From_Editor_ID(packet->ID);
	DBG_ASSERT(actor != NULL);
	
	CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

	if (packet->Serialized_Data.Size() > 0)
	{
		BinaryStream bs(packet->Serialized_Data.Buffer(), packet->Serialized_Data.Size());
		actor->Get_Script_Object().Get()->Deserialize(&bs, CVMObjectSerializeFlags::Full, packet->Serialized_Data_Version);
		bs.Close();

		actor->Get_Event_Listener()->On_Editor_Property_Change();
	}

	PathManager::Get()->Reset();
}
