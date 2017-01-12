// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/UIEvent.h"

const char* UIEvent::Event_Names[] =
{
#define EVENT(name) #name,
#include "Engine/UI/UIEventList.inc"
#undef EVENT
};

UIEvent::UIEvent(UIEventType::Type type, UIElement* source, UIScene* scene, OnlineUser* user, JoystickState* joystick)
	: Type(type)
	, Source(source)
	, Scene(scene)
	, Source_User(user)
	, Source_Joystick(joystick)
{
	Name = Event_Names[(int)type];
}