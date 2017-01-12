// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_VOTESELECTMAP_
#define _GAME_UI_SCENES_UISCENE_VOTESELECTMAP_

#include "Engine/UI/UIScene.h"

class Vote;

class UIScene_VoteSelectMap : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_VoteSelectMap, "UI");

private:
	Vote* m_vote;

protected:
	void Refresh_Preview();

public:
	UIScene_VoteSelectMap(Vote* vote);

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

