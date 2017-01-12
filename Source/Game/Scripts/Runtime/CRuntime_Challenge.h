// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_CHALLENGE_
#define _GAME_RUNTIME_CHALLENGE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Challenge
{
public:
	static void Set_ID(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Description(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Description(CVirtualMachine* vm, CVMValue self);
	static void Set_Timeframe(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Timeframe(CVirtualMachine* vm, CVMValue self);
	static void Set_Requires_Activation(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Requires_Activation(CVirtualMachine* vm, CVMValue self);
	static void Set_Max_Progress(CVirtualMachine* vm, CVMValue self, float value);
	static float Get_Max_Progress(CVirtualMachine* vm, CVMValue self);
	static void Set_Progress(CVirtualMachine* vm, CVMValue self, float value);
	static float Get_Progress(CVirtualMachine* vm, CVMValue self);

	static void Set_Specific_Date(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Specific_Date(CVirtualMachine* vm, CVMValue self);
	static void Set_Start_Day(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Start_Day(CVirtualMachine* vm, CVMValue self);
	static void Set_Start_Month(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Start_Month(CVirtualMachine* vm, CVMValue self);

	static void Set_Announce(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Announce(CVirtualMachine* vm, CVMValue self);
	static void Set_Announce_Name(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Announce_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Announce_Description(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Announce_Description(CVirtualMachine* vm, CVMValue self);
	static void Set_Announce_Image(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Announce_Image(CVirtualMachine* vm, CVMValue self);

	static void Start_Custom_Map(CVirtualMachine* vm, CVMValue self, CVMString map, int seed);
	static int Get_In_Custom_Map(CVirtualMachine* vm, CVMValue self);
	
	static int Was_Completed(CVirtualMachine* vm, int value);

	static void Bind(CVirtualMachine* machine);

};

#endif