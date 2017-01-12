// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_PATHPLANNERCOMPONENT_
#define _GAME_RUNTIME_PATHPLANNERCOMPONENT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_PathPlannerComponent
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);
	static int Get_Has_Path(CVirtualMachine* vm, CVMValue self);
	static void Set_Target_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Target_Position(CVirtualMachine* vm, CVMValue self);
	static void Set_Source_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Source_Position(CVirtualMachine* vm, CVMValue self);
	static void Set_Regenerate_Delta(CVirtualMachine* vm, CVMValue self, float value);
	static float Get_Regenerate_Delta(CVirtualMachine* vm, CVMValue self);
	static void Set_Collision_Group(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Collision_Group(CVirtualMachine* vm, CVMValue self);
	static void Set_Client_Side(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Client_Side(CVirtualMachine* vm, CVMValue self);
	static int Get_Target_In_LOS(CVirtualMachine* vm, CVMValue self);
	static int Get_At_Target(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Movement_Vector(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);
};

#endif
