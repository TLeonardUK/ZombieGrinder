// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_TEAMCHANGE_
#define _GAME_UI_SCENES_UISCENE_TEAMCHANGE_

#include "Engine/UI/UIScene.h"

class UIScene_TeamChange : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_TeamChange, "UI");

private:

protected:

public:
	UIScene_TeamChange();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

