// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UITRANSITION_
#define _ENGINE_UI_UITRANSITION_

#include "Engine/Engine/FrameTime.h"

class UIScene;
class UIManager;

struct UITransitionDrawOrder
{
	enum Type
	{
		NewFirstOnPopElseOld,
		OldFirstOnPopElseNew,
		NewFirst,
		OldFirst
	};
};

class UITransition
{
	MEMORY_ALLOCATOR(UITransition, "UI");

protected:

public:
	UITransition();
	virtual ~UITransition();
	
	virtual const char* Get_Name() = 0;

	virtual bool Is_Complete() = 0;

	virtual UITransitionDrawOrder::Type Get_Draw_Order() 
	{
		return UITransitionDrawOrder::NewFirstOnPopElseOld;
	}

	virtual float Get_Delta() = 0;

	virtual void Tick(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current) = 0;
	virtual void Draw(const FrameTime& time, UIManager* manager, UIScene* scene, int scene_index, bool is_current) = 0;

};

#endif

