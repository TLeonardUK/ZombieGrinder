// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_VOTEOVERLAY_
#define _GAME_UI_SCENES_UISCENE_VOTEOVERLAY_

#include "Engine/UI/UIScene.h"

#include "Generic/Types/Color.h"

class UIScene_Game;
class UITextPanel;

class UIScene_VoteOverlay : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_VoteOverlay, "UI");

private:
	UIScene_Game* m_game_scene;
	Color m_original_bg_color;

protected:

public:
	UIScene_VoteOverlay(UIScene_Game* game);
	~UIScene_VoteOverlay();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();	
	bool Should_Render_Lower_Scenes_Background();
	bool Should_Tick_When_Not_Top();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();
	
	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

