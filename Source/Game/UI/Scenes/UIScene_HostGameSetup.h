// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_HOST_GAME_SETUP_
#define _GAME_UI_SCENES_UISCENE_HOST_GAME_SETUP_

#include "Engine/UI/UIScene.h"

class UIScene_HostGameSetup : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_HostGameSetup, "UI");

private:
	bool m_changing_lobby_settings;

protected:
	void Refresh_Map_List();
	
public:
	UIScene_HostGameSetup(bool change_lobby_settings = false);

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	void Refresh_Items();

	void Refresh(UIManager* manager);

	void Refresh_Preview();
	void Refresh_Player_Count();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

