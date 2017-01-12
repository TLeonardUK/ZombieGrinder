// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_PARTICLECOLLECTORCOMPONENT_
#define _GAME_RUNTIME_PARTICLECOLLECTORCOMPONENT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_ParticleCollectorComponent
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Paused(CVirtualMachine* vm, CVMValue self, int val);
	static int Get_Is_Paused(CVirtualMachine* vm, CVMValue self);
	static void Set_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val);
	static CVMObjectHandle Get_Offset(CVirtualMachine* vm, CVMValue self);
	static void Set_Radius(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Radius(CVirtualMachine* vm, CVMValue self);
	static void Set_Strength(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Strength(CVirtualMachine* vm, CVMValue self);
	static int Get_Collected(CVirtualMachine* vm, CVMValue self, int type);

	static void Bind(CVirtualMachine* machine);
};

#endif
