// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorObjectDeletionAction.h"
#include "Game/Game/EditorManager.h"
#include "Game/Runner/Game.h"

#include "Game/Network/ActorReplicator.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/GameScene.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Pathing/PathManager.h"
#include "Engine/Scene/Map/Map.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"

EditorObjectDeletionAction::EditorObjectDeletionAction(NetPacket_C2S_EditorDeleteObjects* action)
	: m_action(*action)
{
}

const char* EditorObjectDeletionAction::Get_Name()
{
	return "Object Deletion";
}

void EditorObjectDeletionAction::Undo()
{
	GameScene* scene = Game::Get()->Get_Game_Scene();
	Map* map = Game::Get()->Get_Map();

	NetPacket_C2S_EditorDeleteObjects* packet = &m_action;

	DBG_LOG("Performing undo operation for object deletion.");

	for (std::vector<Deleted_Actor>::iterator iter = m_deleted_actor_states.begin(); iter != m_deleted_actor_states.end(); iter++)
	{
		Deleted_Actor& block = *iter;

		ScriptedActor* scripted = ScriptedActor::Deserialize(&block.state);
		if (scripted != NULL)
		{
			block.id->ID = EditorManager::Get()->Get_Editor_ID(scripted);
			DBG_LOG("Recreating actor %i.", block.id->ID);
		}
	}	

	PathManager::Get()->Reset();
}

void EditorObjectDeletionAction::Do()
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	NetPacket_C2S_EditorDeleteObjects* packet = &m_action;

	DBG_LOG("Performing object deletion on %i objects.", packet->IDs.size());

	m_deleted_actor_states.clear();

	for (unsigned int i = 0; i < packet->IDs.size(); i++)
	{
		EditorDeleteObjectID& state = packet->IDs.at(i);

		ScriptedActor* actor = EditorManager::Get()->Get_Actor_From_Editor_ID(state.ID);
		if (actor != NULL)
		{
			// Serialize so we can undelete if needed.
			MapFileObjectBlock object_block;
			actor->Serialize(&object_block);

			DBG_LOG("Deleting actor %i", state.ID);

			Deleted_Actor da;
			da.state = object_block;
			da.id = &state;
			m_deleted_actor_states.push_back(da);

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
