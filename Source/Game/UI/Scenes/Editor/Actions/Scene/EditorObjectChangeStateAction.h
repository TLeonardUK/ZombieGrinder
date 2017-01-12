// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_EDITOR_CHANGE_OBJECT_STATE_ACTION_
#define _GAME_UI_SCENES_EDITOR_CHANGE_OBJECT_STATE_ACTION_

#include "Game/UI/Scenes/Editor/Actions/EditorAction.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Map.h"

#include <vector>

class ScriptedActor;

class EditorChangeObjectStateAction : public EditorAction
{
	MEMORY_ALLOCATOR(EditorChangeObjectStateAction, "UI");

private:
	NetPacket_C2S_EditorChangeObjectState m_action;

protected:

public:
	EditorChangeObjectStateAction(NetPacket_C2S_EditorChangeObjectState* action);

	const char* Get_Name();
	void Undo();
	void Do();

};

#endif

