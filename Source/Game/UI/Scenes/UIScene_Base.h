// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_BASE_
#define _GAME_UI_SCENES_UISCENE_BASE_

#include "Engine/UI/UIScene.h"

class UIScene_Base : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Base, "UI");

private:

protected:

public:
	UIScene_Base();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	
	bool Can_Accept_Invite();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

