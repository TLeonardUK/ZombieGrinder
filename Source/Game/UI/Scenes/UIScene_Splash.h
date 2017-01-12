// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_SPLASH_
#define _GAME_UI_SCENES_UISCENE_SPLASH_

#include "Engine/UI/UIScene.h"

class UIScene_Splash : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Splash, "UI");

private:
	int		m_splash_index;
	int		m_timer_step;
	float	m_timer;
	float	m_splash_screen_interval;
	bool	m_showing_autosave;

protected:
	bool Next_Splash();

public:
	UIScene_Splash();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	
	bool Can_Accept_Invite();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

