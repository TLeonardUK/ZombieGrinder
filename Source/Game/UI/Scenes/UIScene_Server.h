// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_SERVER_
#define _GAME_UI_SCENES_UISCENE_SERVER_

#include "Engine/UI/UIScene.h"

class UIScene_Server : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Server, "UI");

private:

protected:
	
public:
	UIScene_Server();
	~UIScene_Server();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

