// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_REPORTPLAYER_
#define _GAME_UI_SCENES_UISCENE_REPORTPLAYER_

#include "Engine/UI/UIScene.h"

#include "Engine/Demo/DemoManager.h"

class UIScene_ReportPlayer : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ReportPlayer, "UI");

private:

protected:
	bool m_closing;

	std::vector<DemoPlayer> m_recent_players;

public:
	UIScene_ReportPlayer();

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

