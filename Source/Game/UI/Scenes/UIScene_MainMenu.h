// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_MAINMENU_
#define _GAME_UI_SCENES_UISCENE_MAINMENU_

#include "Engine/UI/UIScene.h"

class UIScene_MainMenu : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_MainMenu, "UI");

private:
	int m_last_purchase_state;

	bool m_restricted_warning_pending;

protected:
	void Check_Layout();

public:
	UIScene_MainMenu();
	~UIScene_MainMenu();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	UIScene* Get_Background(UIManager* manager);
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

