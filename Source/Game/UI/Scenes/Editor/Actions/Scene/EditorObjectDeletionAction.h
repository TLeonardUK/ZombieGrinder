// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_EDITOR_OBJECT_DELETION_ACTION_
#define _GAME_UI_SCENES_EDITOR_OBJECT_DELETION_ACTION_

#include "Game/UI/Scenes/Editor/Actions/EditorAction.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Map.h"

#include <vector>

class ScriptedActor;

class EditorObjectDeletionAction : public EditorAction
{
	MEMORY_ALLOCATOR(EditorObjectDeletionAction, "UI");

private:

	struct Deleted_Actor
	{
		MapFileObjectBlock		state;
		EditorDeleteObjectID* id;
	};

	NetPacket_C2S_EditorDeleteObjects m_action;

	std::vector<Deleted_Actor> m_deleted_actor_states;

protected:

public:
	EditorObjectDeletionAction(NetPacket_C2S_EditorDeleteObjects* action);

	const char* Get_Name();
	void Undo();
	void Do();

};

#endif

