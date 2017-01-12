// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_COLOR_SELECTOR_
#define _GAME_UI_SCENES_UISCENE_COLOR_SELECTOR_

#include "Engine/UI/UIScene.h"

#include "Generic/Types/Color.h"

class UIScene_ColorSelector : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ColorSelector, "UI");

private:
	Color* m_color;

protected:
	
public:
	UIScene_ColorSelector(Color* color);

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

