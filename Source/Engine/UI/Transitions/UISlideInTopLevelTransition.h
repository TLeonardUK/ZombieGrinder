// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_TRANSITIONS_UISLIDETOPLEVELTRANSITION_
#define _ENGINE_UI_TRANSITIONS_UISLIDETOPLEVELTRANSITION_

#include "Engine/UI/UITransition.h"

#include "Generic/Types/Vector2.h"

class UIScene;
class UIManager;
class UIElement;

class UISlideInTopLevelTransition : public UITransition
{
	MEMORY_ALLOCATOR(UISlideInTopLevelTransition, "UI");

protected:
	enum
	{
		SLIDE_IN_INTERVAL = 700
	};

	float m_timer;

private:
	Vector2 Calculate_Element_Offset(UIElement* element, float delta);

public:
	UISlideInTopLevelTransition();
	~UISlideInTopLevelTransition();

	UITransitionDrawOrder::Type Get_Draw_Order();
	
	const char* Get_Name();
	
	float Get_Delta();

	bool Is_Complete();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);

};

#endif

