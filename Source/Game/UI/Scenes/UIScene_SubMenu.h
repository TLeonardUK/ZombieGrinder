// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_SUBMENU_
#define _GAME_UI_SCENES_UISCENE_SUBMENU_

#include "Engine/UI/UIScene.h"

#include "Game/UI/Scenes/Backgrounds/MenuTitleRenderer.h"

class UIScene_SubMenu : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_SubMenu, "UI");

private:
	MenuTitleRenderer m_title_renderer;
	bool m_should_sync_profile;
	bool m_leaving;

protected:

public:
	UIScene_SubMenu();
	~UIScene_SubMenu();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();	
	bool Should_Render_Lower_Scenes_Background();
	bool Should_Tick_When_Not_Top();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	UIScene* Get_Background(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Close(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

