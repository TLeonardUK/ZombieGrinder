// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_CHALLENGE_ANNOUNCEMENT_
#define _GAME_UI_SCENES_UISCENE_CHALLENGE_ANNOUNCEMENT_

#include "Engine/UI/UIScene.h"

#include "Game/Online/ChallengeManager.h"

class UIScene_ChallengeAnnouncement : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_ChallengeAnnouncement, "UI");

private:
	ChallengeAnnouncement m_announcement;

protected:

public:
	UIScene_ChallengeAnnouncement(ChallengeAnnouncement announcement);

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

#pragma once
