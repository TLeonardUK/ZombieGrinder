// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_EDITOR_OBJECT_RESPOSITION_ACTION_
#define _GAME_UI_SCENES_EDITOR_OBJECT_RESPOSITION_ACTION_

#include "Game/UI/Scenes/Editor/Actions/EditorAction.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Scene/Map/Map.h"

#include <vector>

class EditorObjectRepositionAction : public EditorAction
{
	MEMORY_ALLOCATOR(EditorObjectRepositionAction, "UI");

private:
	NetPacket_C2S_EditorRepositionObjects m_action;

	std::vector<EditorRepositionObjectsState> m_old_states;

protected:

public:
	EditorObjectRepositionAction(NetPacket_C2S_EditorRepositionObjects* action);

	const char* Get_Name();
	void Undo();
	void Do();

};

#endif

