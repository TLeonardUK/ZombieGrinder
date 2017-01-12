// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_SKILL_
#define _GAME_RUNTIME_SKILL_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Skill
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Archetype(CVirtualMachine* vm, CVMValue self);
	static void Set_Archetype(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static int Get_Archetype_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Archetype_ID(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Unique_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Unique_ID(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Recieve_Time(CVirtualMachine* vm, CVMValue self);
	static void Set_Recieve_Time(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Equip_Time(CVirtualMachine* vm, CVMValue self);
	static void Set_Equip_Time(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Equip_Slot(CVirtualMachine* vm, CVMValue self);
	static void Set_Equip_Slot(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Was_Rolled_Back(CVirtualMachine* vm, CVMValue self);
	static void Set_Was_Rolled_Back(CVirtualMachine* vm, CVMValue self, int value);

	static void Bind(CVirtualMachine* machine);
};

#endif
