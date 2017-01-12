// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorObjectCreationAction.h"
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

EditorObjectCreationAction::EditorObjectCreationAction(NetPacket_C2S_EditorCreateObjects* action)
	: m_action(*action)
{
}

const char* EditorObjectCreationAction::Get_Name()
{
	return "Object Creation";
}

void EditorObjectCreationAction::Undo()
{
	GameScene* scene = Game::Get()->Get_Game_Scene();
	Map* map = Game::Get()->Get_Map();

	for (std::vector<int>::iterator iter = m_created_object_ids.begin(); iter != m_created_object_ids.end(); iter++)
	{
		int id = *iter;
		
		ScriptedActor* actor = EditorManager::Get()->Get_Actor_From_Editor_ID(id);
		if (actor != NULL)
		{
			if (actor->Get_Script_Object().Get()->Get_Symbol()->symbol->class_data->is_replicated != 0)
			{
				ActorReplicator::Get()->Purge_Actor(actor, true);
			}
			else
			{
				SAFE_DELETE(actor);
			}
		}
	}		
	
	UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();
	if (editor != NULL)
	{
		editor->Objects_Deleted();
	}

	PathManager::Get()->Reset();
}

void EditorObjectCreationAction::Do()
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	NetPacket_C2S_EditorCreateObjects* packet = &m_action;

	DBG_LOG("Performing object creation on %i objects.", packet->States.size());

	m_created_object_ids.clear();

	std::vector<ScriptedActor*> objs;

	for (unsigned int i = 0; i < packet->States.size(); i++)
	{
		EditorCreateObjectState& state = packet->States.at(i);

		MapFileObjectBlock block;
		block.Class_Name = state.Class_Name;
		block.Serialized_Data = state.Serialized_Data;
		block.Serialized_Data_Version = state.Serialized_Data_Version;
		
		ScriptedActor* scripted = ScriptedActor::Deserialize(&block);
		if (scripted != NULL)
		{
			scripted->Set_Position(scripted->Get_Position() + Vector3(packet->Origin_X, packet->Origin_Y, 0.0f));
			scripted->Set_Layer(packet->Origin_Layer);
			m_created_object_ids.push_back(EditorManager::Get()->Get_Editor_ID(scripted));

			objs.push_back(scripted);
		}
	}

	// If we were creator, select them in editor.
	if (packet->Creator_ID == NetManager::Get()->Get_Primary_Local_Net_User()->Get_Net_ID())
	{
		UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();
		if (editor != NULL)
		{
			editor->Select_Specific_Objects(objs);
		}
	}

	PathManager::Get()->Reset();
}
