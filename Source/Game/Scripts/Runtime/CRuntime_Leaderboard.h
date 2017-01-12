// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_LEADERBOARD_
#define _GAME_RUNTIME_LEADERBOARD_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Leaderboard
{
public:
	static void Set_ID(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Replace_Score(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Replace_Score(CVirtualMachine* vm, CVMValue self);
	static void Set_Ascending(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Ascending(CVirtualMachine* vm, CVMValue self);
	static void Submit(CVirtualMachine* vm, CVMValue self, int value);

	static void Bind(CVirtualMachine* machine);

};

#endif