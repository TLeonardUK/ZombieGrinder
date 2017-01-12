// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_DIALOG_
#define _GAME_UI_SCENES_UISCENE_DIALOG_

#include "Engine/UI/UIScene.h"

class UIScene_Dialog : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Dialog, "UI");

private:
	std::string m_text;
	bool m_game_style;

protected:
	
public:
	UIScene_Dialog(std::string text, std::string button_text = "", bool game_style = true, bool large_size = false, std::string override_layout = "");

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();

	bool Can_Accept_Invite();
	
	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

