// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_MODERATION_
#define _GAME_UI_SCENES_UISCENE_MODERATION_

#include "Engine/UI/UIScene.h"

#include "Engine/UI/Elements/UIListView.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

class UIScene_Moderation : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Moderation, "UI");

private:

public:
	UIScene_Moderation();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();

	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Update_Labels();

	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

