// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_SERVER_BROWSER_
#define _GAME_UI_SCENES_UISCENE_SERVER_BROWSER_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineMatching.h"

class UIScene_ServerBrowser : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ServerBrowser, "UI");

private:
	OnlineMatching_ServerSource::Type m_last_server_source;
	std::vector<OnlineMatching_Server> m_servers;

protected:
	OnlineMatching_ServerSource::Type Get_Server_Source();
	
public:
	UIScene_ServerBrowser();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

