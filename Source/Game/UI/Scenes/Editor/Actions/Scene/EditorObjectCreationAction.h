// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_EDITOR_OBJECT_CREATION_ACTION_
#define _GAME_UI_SCENES_EDITOR_OBJECT_CREATION_ACTION_

#include "Game/UI/Scenes/Editor/Actions/EditorAction.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Map.h"

#include <vector>

class ScriptedActor;

class EditorObjectCreationAction : public EditorAction
{
	MEMORY_ALLOCATOR(EditorObjectCreationAction, "UI");

private:
	NetPacket_C2S_EditorCreateObjects m_action;

	std::vector<int> m_created_object_ids;

protected:

public:
	EditorObjectCreationAction(NetPacket_C2S_EditorCreateObjects* action);

	const char* Get_Name();
	void Undo();
	void Do();

};

#endif

