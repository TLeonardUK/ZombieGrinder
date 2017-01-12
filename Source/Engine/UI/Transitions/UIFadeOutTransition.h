// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_TRANSITIONS_UIFADEOUTTRANSITION_
#define _ENGINE_UI_TRANSITIONS_UIFADEOUTTRANSITION_

#include "Engine/UI/UITransition.h"

class UIScene;
class UIManager;

class UIFadeOutTransition : public UITransition
{
	MEMORY_ALLOCATOR(UIFadeOutTransition, "UI");

protected:
	enum
	{
		FADE_INTERVAL = 500
	};

	float m_timer;

public:
	UIFadeOutTransition();
	~UIFadeOutTransition();
	
	const char* Get_Name();
	
	float Get_Delta();

	bool Is_Complete();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);

};

#endif

