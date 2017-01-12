// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_ADVANCED_GRAPHICS_OPTIONS_
#define _GAME_UI_SCENES_UISCENE_ADVANCED_GRAPHICS_OPTIONS_

#include "Engine/UI/UIScene.h"

class UIScene_AdvancedGraphicsOptions : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_AdvancedGraphicsOptions, "UI");

private:

protected:

public:
	UIScene_AdvancedGraphicsOptions();

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

