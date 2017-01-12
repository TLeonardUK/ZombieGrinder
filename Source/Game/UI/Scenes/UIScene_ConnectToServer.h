// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_CONNECT_TO_SERVER_
#define _GAME_UI_SCENES_UISCENE_CONNECT_TO_SERVER_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineMatching.h"

class UIScene_ConnectToServer : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ConnectToServer, "UI");

private:
	OnlineMatching_Server m_server;

	bool m_connection_stated;

protected:
	
public:
	UIScene_ConnectToServer(OnlineMatching_Server server);

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

