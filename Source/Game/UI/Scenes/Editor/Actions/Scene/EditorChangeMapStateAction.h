// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_EDITOR_CHANGE_MAP_STATE_ACTION_
#define _GAME_UI_SCENES_EDITOR_CHANGE_MAP_STATE_ACTION_

#include "Game/UI/Scenes/Editor/Actions/EditorAction.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Map.h"

#include <vector>

class ScriptedActor;

class EditorChangeMapStateAction : public EditorAction
{
	MEMORY_ALLOCATOR(EditorChangeMapStateAction, "UI");

private:
	NetPacket_C2S_EditorChangeMapState m_action;

	bool m_map_size_changed;
	std::vector<MapTile> m_original_tiles;

protected:

public:
	EditorChangeMapStateAction(NetPacket_C2S_EditorChangeMapState* action);

	const char* Get_Name();
	void Undo();
	void Do();

};

#endif

