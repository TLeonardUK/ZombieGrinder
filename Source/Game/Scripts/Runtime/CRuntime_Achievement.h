// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ACHIEVEMENT_
#define _GAME_RUNTIME_ACHIEVEMENT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Achievement
{
public:
	static void Set_ID(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Description(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Description(CVirtualMachine* vm, CVMValue self);
	static void Set_Progress(CVirtualMachine* vm, CVMValue self, float value);
	static float Get_Progress(CVirtualMachine* vm, CVMValue self);
	static void Set_Max_Progress(CVirtualMachine* vm, CVMValue self, float value);
	static float Get_Max_Progress(CVirtualMachine* vm, CVMValue self);
	static void Set_Locked_Frame(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Locked_Frame(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlocked_Frame(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Unlocked_Frame(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlocked(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Unlocked(CVirtualMachine* vm, CVMValue self);
	static void Set_Track_Statistic(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Track_Statistic(CVirtualMachine* vm, CVMValue self);
	static void Set_Use_Statistic_Progress(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Use_Statistic_Progress(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);

};

#endif