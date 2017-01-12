// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorObjectRespositionAction.h"
#include "Game/Game/EditorManager.h"
#include "Game/Runner/Game.h"
#include "Engine/Engine/GameEngine.h"
#include "Game/Scripts/ScriptEventListener.h"

#include "Engine/Scene/Pathing/PathManager.h"
#include "Engine/Scene/Pathing/PathManager.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "Engine/Scene/Map/Map.h"

EditorObjectRepositionAction::EditorObjectRepositionAction(NetPacket_C2S_EditorRepositionObjects* action)
	: m_action(*action)
{
}

const char* EditorObjectRepositionAction::Get_Name()
{
	return "Object Resposition";
}

void EditorObjectRepositionAction::Undo()
{
	NetPacket_C2S_EditorRepositionObjects* packet = &m_action;

	DBG_LOG("Performing undo operation for object reposition.");
	
	for (unsigned int i = 0; i < packet->Old_States.size(); i++)
	{
		EditorRepositionObjectsState& state = packet->Old_States.at(i);

		ScriptedActor* actor = EditorManager::Get()->Get_Actor_From_Editor_ID(state.Index);
		if (actor != NULL)
		{
			actor->Set_Position(Vector3(state.X, state.Y, actor->Get_Position().Z));
			actor->Set_Rotation(Vector3(0.0f, 0.0f, state.Angle));
			actor->Set_Bounding_Box(state.Bounding_Box);

			{
				CVirtualMachine* vm = GameEngine::Get()->Get_VM();
				CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

				actor->Get_Event_Listener()->On_Editor_Property_Change();
			}
		}
	}

	PathManager::Get()->Reset();
}

void EditorObjectRepositionAction::Do()
{
	NetPacket_C2S_EditorRepositionObjects* packet = &m_action;

	DBG_LOG("Performing object reposition on %i objects.", packet->States.size());

	for (unsigned int i = 0; i < packet->States.size(); i++)
	{
		EditorRepositionObjectsState& state = packet->States.at(i);

		ScriptedActor* actor = EditorManager::Get()->Get_Actor_From_Editor_ID(state.Index);
		if (actor != NULL)
		{
			actor->Set_Position(Vector3(state.X, state.Y, actor->Get_Position().Z));
			actor->Set_Rotation(Vector3(0.0f, 0.0f, state.Angle));
			actor->Set_Bounding_Box(state.Bounding_Box);

			{
				CVirtualMachine* vm = GameEngine::Get()->Get_VM();
				CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

				actor->Get_Event_Listener()->On_Editor_Property_Change();
			}
		}
	}

	PathManager::Get()->Reset();
}
