// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_CLIENT_
#define _ENGINE_ONLINE_CLIENT_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

class OnlineUser;

class OnlineClient
{
	MEMORY_ALLOCATOR(OnlineClient, "Network");

private:

public:
	virtual void Tick(const FrameTime& time) = 0;

	// Platform specific dialogs.
	virtual void Show_Invite_Dialog() = 0;
	virtual void Show_Profile_Dialog(OnlineUser* user) = 0;

};

#endif

