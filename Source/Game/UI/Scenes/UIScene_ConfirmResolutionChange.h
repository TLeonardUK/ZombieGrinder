// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_CONFIRM_RESOLUTION_CHANGE_
#define _GAME_UI_SCENES_UISCENE_CONFIRM_RESOLUTION_CHANGE_

#include "Engine/UI/UIScene.h"

#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

class UIScene_ConfirmResolutionChange : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ConfirmResolutionChange, "UI");

private:
	float m_revert_timer;

	int m_old_width;
	int m_old_height;
	int m_old_mode;
	int m_old_hertz;

	enum
	{
		REVERT_TIME_INTERVAL = 15000
	};

protected:
	
public:
	UIScene_ConfirmResolutionChange(int old_width, int old_height, int old_mode, int old_hertz);

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

