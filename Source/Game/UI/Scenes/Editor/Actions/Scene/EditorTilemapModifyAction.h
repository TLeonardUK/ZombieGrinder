// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_EDITOR_TILEMAP_MODIFY_ACTION_
#define _GAME_UI_SCENES_EDITOR_TILEMAP_MODIFY_ACTION_

#include "Game/UI/Scenes/Editor/Actions/EditorAction.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Scene/Map/Map.h"

#include <vector>

class EditorTilemapModifyAction : public EditorAction
{
	MEMORY_ALLOCATOR(EditorTilemapModifyAction, "UI");

private:
	NetPacket_C2S_ModifyTilemap m_action;

	Rect2D m_original_tiles_bounds;
	std::vector<MapTile> m_original_tiles;

protected:

public:
	EditorTilemapModifyAction(NetPacket_C2S_ModifyTilemap* action);

	const char* Get_Name();
	void Undo();
	void Do();

};

#endif

