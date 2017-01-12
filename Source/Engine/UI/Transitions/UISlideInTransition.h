// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_TRANSITIONS_UISLIDETRANSITION_
#define _ENGINE_UI_TRANSITIONS_UISLIDETRANSITION_

#include "Engine/UI/UITransition.h"

#include "Generic/Types/Vector2.h"

class UIScene;
class UIManager;
class UIElement;

class UISlideInTransition : public UITransition
{
	MEMORY_ALLOCATOR(UISlideInTransition, "UI");

protected:
	enum
	{
		SLIDE_IN_INTERVAL = 700
	};

	float m_timer;

	bool m_include_fade;
	bool m_do_not_fade_bg;

private:
	Vector2 Calculate_Element_Offset(UIElement* element, float delta);

public:
	UISlideInTransition(bool include_fade = true, bool do_not_fade_bg = false);
	~UISlideInTransition();
	
	const char* Get_Name();
	
	float Get_Delta();

	bool Is_Complete();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current);

};

#endif

