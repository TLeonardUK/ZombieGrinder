// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_RESTRICTED_MODE_WARNING_
#define _GAME_UI_SCENES_UISCENE_RESTRICTED_MODE_WARNING_

#include "Engine/UI/UIScene.h"

class UIScene_RestrictedModeWarning : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_RestrictedModeWarning, "UI");

private:
	bool m_in_mode;

protected:

public:
	UIScene_RestrictedModeWarning(bool bInMode = true);

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

