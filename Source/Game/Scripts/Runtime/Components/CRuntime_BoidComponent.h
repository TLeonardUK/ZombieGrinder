// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_BOIDCOMPONENT_
#define _GAME_RUNTIME_BOIDCOMPONENT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_BoidComponent
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);
	static void Set_Center(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val);
	static CVMObjectHandle Get_Center(CVirtualMachine* vm, CVMValue self);
	static void Set_Neighbour_Distance(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Neighbour_Distance(CVirtualMachine* vm, CVMValue self);
	static void Set_Seperation_Distance(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Seperation_Distance(CVirtualMachine* vm, CVMValue self);
	static void Set_Maximum_Speed(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Maximum_Speed(CVirtualMachine* vm, CVMValue self);
	static void Set_Maximum_Force(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Maximum_Force(CVirtualMachine* vm, CVMValue self);
	static void Set_Cohesion_Weight(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Cohesion_Weight(CVirtualMachine* vm, CVMValue self);
	static void Set_Avoidance_Weight(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Avoidance_Weight(CVirtualMachine* vm, CVMValue self);
	static void Set_Seperation_Weight(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Seperation_Weight(CVirtualMachine* vm, CVMValue self);
	static void Set_Enabled(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Enabled(CVirtualMachine* vm, CVMValue self);


	static void Bind(CVirtualMachine* machine);
};

#endif
