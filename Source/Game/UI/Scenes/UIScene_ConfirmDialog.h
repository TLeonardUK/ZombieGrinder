// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_CONFIRM_DIALOG_
#define _GAME_UI_SCENES_UISCENE_CONFIRM_DIALOG_

#include "Engine/UI/UIScene.h"

#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

class UIScene_ConfirmDialog : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ConfirmDialog, "UI");

private:
	int m_selected_option;
	bool m_game_style;

protected:
	
public:
	UIScene_ConfirmDialog(std::string text, std::string opt_1, std::string opt_2, bool game_style = true);

	int Get_Selected_Index();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();
	bool Should_Fade_Cursor();

	bool Can_Accept_Invite();

	void Refresh_Items();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

