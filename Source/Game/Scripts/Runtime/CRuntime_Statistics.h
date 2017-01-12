// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_STATISTICS_
#define _GAME_RUNTIME_STATISTICS_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Statistics
{
public:
	static void Set_ID(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Description(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Description(CVirtualMachine* vm, CVMValue self);
	static void Set_Category(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Category(CVirtualMachine* vm, CVMValue self);
	static void Set_Aggregated(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Aggregated(CVirtualMachine* vm, CVMValue self);
	static void Set_Aggregation_Period(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Aggregation_Period(CVirtualMachine* vm, CVMValue self);
	static void Set_Display(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Display(CVirtualMachine* vm, CVMValue self);
	static void Set_Value(CVirtualMachine* vm, CVMValue self, float value);
	static float Get_Value(CVirtualMachine* vm, CVMValue self);
	static float Get_Aggregated_Value(CVirtualMachine* vm, CVMValue self);
	static void Set_Aggregated_Value(CVirtualMachine* vm, CVMValue self, float value);
	static void Set_Data_Type(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Data_Type(CVirtualMachine* vm, CVMValue self);
	static void Set_Mirror_Statistic(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Mirror_Statistic(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);

};

#endif