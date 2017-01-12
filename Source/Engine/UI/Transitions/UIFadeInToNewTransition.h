// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_TRANSITIONS_UIFADEINTONEWTRANSITION_
#define _ENGINE_UI_TRANSITIONS_UIFADEINTONEWTRANSITION_

#include "Engine/UI/UITransition.h"

class UIScene;
class UIManager;

class UIFadeInToNewTransition : public UITransition
{
	MEMORY_ALLOCATOR(UIFadeInToNewTransition, "UI");

protected:
	enum
	{
		FADE_INTERVAL = 500
	};

	float m_timer;

public:
	UIFadeInToNewTransition();
	~UIFadeInToNewTransition();
	
	const char* Get_Name();
	
	UITransitionDrawOrder::Type Get_Draw_Order();

	float Get_Delta();

	bool Is_Complete();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);

};

#endif

