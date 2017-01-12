// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_TRANSITIONS_UIFADETOPLEVELTRANSITION_
#define _ENGINE_UI_TRANSITIONS_UIFADETOPLEVELTRANSITION_

#include "Engine/UI/UITransition.h"

class UIScene;
class UIManager;

class UIFadeTopLevelTransition : public UITransition
{
	MEMORY_ALLOCATOR(UIFadeTopLevelTransition, "UI");

protected:
	enum
	{
		FADE_INTERVAL = 500
	};

	float m_timer;

public:
	UIFadeTopLevelTransition();
	~UIFadeTopLevelTransition();
	
	const char* Get_Name();
	
	float Get_Delta();

	bool Is_Complete();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);

};

#endif

