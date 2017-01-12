// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_STARTING_LOCAL_SERVER_
#define _GAME_UI_SCENES_UISCENE_STARTING_LOCAL_SERVER_

#include "Engine/UI/UIScene.h"

#include "Engine/Online/OnlineMatching.h"

class UIScene_StartingLocalServer : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_StartingLocalServer, "UI");

private:
	bool m_start_editor;

protected:

	void Succeeded();
	void Failed();
	
public:
	UIScene_StartingLocalServer(bool start_editor = false);

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

