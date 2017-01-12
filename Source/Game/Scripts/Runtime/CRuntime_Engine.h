// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ENGINE_
#define _GAME_RUNTIME_ENGINE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Engine
{
public:
	static int Has_Crashed(CVirtualMachine* vm);
	static int Has_Uploaded_To_Workshop(CVirtualMachine* vm);
	static int In_Map_Restricted_Mode(CVirtualMachine* vm);
	static int In_Restricted_Mode(CVirtualMachine* vm);
	static int Get_Highest_Profile_Level(CVirtualMachine* vm);

	static void Bind(CVirtualMachine* machine);
};

#endif
