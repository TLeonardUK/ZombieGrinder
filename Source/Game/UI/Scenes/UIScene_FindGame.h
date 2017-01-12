// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_FIND_GAME_
#define _GAME_UI_SCENES_UISCENE_FIND_GAME_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineMatching.h"

class MapFileHandle;

class UIScene_FindGame : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_FindGame, "UI");

private:
	MapFileHandle* m_map;
	bool m_join_lobbies;
	bool m_join_games;

	bool m_accept_invite;

	bool m_checking_servers;
	float m_server_search_timer;
	int m_server_refresh_state;

	bool m_connecting;

	enum 
	{
		server_search_timeout = 10000,
		max_server_ping = 300
	};

protected:
	
public:
	UIScene_FindGame(bool accept_invite);
	UIScene_FindGame(MapFileHandle* map, bool join_lobbies, bool join_games);

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

