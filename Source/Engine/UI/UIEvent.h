// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIEVENT_
#define _ENGINE_UI_UIEVENT_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/Rect2D.h"

#include <vector>

class UIManager;
class UIScene;
class UILayout;
class UIElement;
class OnlineUser;
class JoystickState;

struct UIEventType
{
	enum Type
	{
#define EVENT(name) name,
#include "Engine/UI/UIEventList.inc"
#undef EVENT
	};
};

struct UIEvent
{	
	MEMORY_ALLOCATOR(UIEvent, "UI");

public:

	static const char* Event_Names[];
	
	UIEventType::Type	Type;
	const char*			Name;
	UIElement*			Source;
	UIScene*			Scene;
	OnlineUser*			Source_User;
	JoystickState*		Source_Joystick;
	int					Param;
	void*				Sub_Source;

	UIEvent() {}
	UIEvent(UIEventType::Type type, UIElement* source, UIScene* scene = NULL, OnlineUser* user = NULL, JoystickState* joystick = NULL);
};

#endif

