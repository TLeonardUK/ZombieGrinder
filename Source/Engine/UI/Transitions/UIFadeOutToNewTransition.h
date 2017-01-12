// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_TRANSITIONS_UIFADEOUTTONEWTRANSITION_
#define _ENGINE_UI_TRANSITIONS_UIFADEOUTTONEWTRANSITION_

#include "Engine/UI/UITransition.h"

class UIScene;
class UIManager;

class UIFadeOutToNewTransition : public UITransition
{
	MEMORY_ALLOCATOR(UIFadeOutToNewTransition, "UI");

protected:
	enum
	{
		FADE_INTERVAL = 1000
	};

	float m_timer;

public:
	UIFadeOutToNewTransition();
	~UIFadeOutToNewTransition();
	
	const char* Get_Name();
	
	UITransitionDrawOrder::Type Get_Draw_Order();

	float Get_Delta();

	bool Is_Complete();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);

};

#endif

