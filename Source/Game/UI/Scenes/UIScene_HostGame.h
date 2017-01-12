// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_HOST_GAME_
#define _GAME_UI_SCENES_UISCENE_HOST_GAME_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineMatching.h"

class MapFileHandle;

class UIScene_HostGame : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_HostGame, "UI");

private:
	MapFileHandle* m_map;
	int m_players;
	bool m_is_private;
	bool m_is_secure;
	bool m_is_local;
	bool m_auto_continue;
	bool m_start_editor;

protected:
	
public:
	UIScene_HostGame(MapFileHandle* map, int players, bool is_private, bool is_secure, bool is_local, bool auto_continue = false, bool start_editor = false);

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	bool Can_Accept_Invite();

	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

