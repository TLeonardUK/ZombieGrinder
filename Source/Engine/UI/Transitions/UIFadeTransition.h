// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_TRANSITIONS_UIFADETRANSITION_
#define _ENGINE_UI_TRANSITIONS_UIFADETRANSITION_

#include "Engine/UI/UITransition.h"

class UIScene;
class UIManager;

class UIFadeTransition : public UITransition
{
	MEMORY_ALLOCATOR(UIFadeTransition, "UI");

protected:
	enum
	{
		FADE_INTERVAL = 2000
	};

	float m_timer;

public:
	UIFadeTransition();
	~UIFadeTransition();
	
	const char* Get_Name();
	
	float Get_Delta();

	bool Is_Complete();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);

};

#endif

