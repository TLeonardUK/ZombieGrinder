// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_TIME_
#define _GAME_RUNTIME_TIME_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Time
{
public:
	static float Get_Frame_Time(CVirtualMachine* vm);
	static float Get_Delta(CVirtualMachine* vm);
	static float Get_Delta_Seconds(CVirtualMachine* vm);
	static float Get_Ticks(CVirtualMachine* vm);
	static void Sleep(CVirtualMachine* vm, float seconds);

	static void Bind(CVirtualMachine* machine);

};

#endif