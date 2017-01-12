// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_EVENT_
#define _GAME_RUNTIME_EVENT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Event
{
public:
	static void Fire_Global(CVirtualMachine* vm, CVMValue self);
	static void Fire_Global_By_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle type_of_actor);
	static void Fire_In_Radius(CVirtualMachine* vm, CVMValue self, CVMObjectHandle position, float radius);
	static void Fire_In_Radius_By_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle type_of_actor, CVMObjectHandle position, float radius);

	static void Bind(CVirtualMachine* machine);
};

#endif
