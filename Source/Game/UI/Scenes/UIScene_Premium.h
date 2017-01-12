// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_PREMIUM_
#define _GAME_UI_SCENES_UISCENE_PREMIUM_

#include "Engine/UI/UIScene.h"

class UIScene_Premium : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Premium, "UI");

private:
	bool m_dialog_open;
	float m_dialog_close_delay;

	enum
	{
		DIALOG_CLOSE_DELAY = 1000,
	};

protected:

public:
	UIScene_Premium();
	~UIScene_Premium();

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

