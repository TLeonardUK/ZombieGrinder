// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_SKILL_ARCHETYPE_
#define _GAME_RUNTIME_SKILL_ARCHETYPE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Skill_Archetype
{
public:
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Description(CVirtualMachine* vm, CVMValue self);
	static void Set_Description(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Icon_Frame(CVirtualMachine* vm, CVMValue self);
	static void Set_Icon_Frame(CVirtualMachine* vm, CVMValue self, CVMString value);
	static int Get_Cost(CVirtualMachine* vm, CVMValue self);
	static void Set_Cost(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Is_Passive(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Passive(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Cannot_Rollback(CVirtualMachine* vm, CVMValue self);
	static void Set_Cannot_Rollback(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Are_Children_Mutex(CVirtualMachine* vm, CVMValue self);
	static void Set_Are_Children_Mutex(CVirtualMachine* vm, CVMValue self, int value);
	static CVMString Get_Parent_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Parent_Name(CVirtualMachine* vm, CVMValue self, CVMString value);
	static int Get_Is_Team_Based(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Team_Based(CVirtualMachine* vm, CVMValue self, int value);
	static float Get_Duration(CVirtualMachine* vm, CVMValue self);
	static void Set_Duration(CVirtualMachine* vm, CVMValue self, float value);
	static CVMString Get_Player_Effect(CVirtualMachine* vm, CVMValue self);
	static void Set_Player_Effect(CVirtualMachine* vm, CVMValue self, CVMString value);

	static float Get_Energy_Required(CVirtualMachine* vm, CVMValue self);
	static void Set_Energy_Required(CVirtualMachine* vm, CVMValue self, float value);
	static int Get_Energy_Type(CVirtualMachine* vm, CVMValue self);
	static void Set_Energy_Type(CVirtualMachine* vm, CVMValue self, int value);

	static int Get_Unlock_Criteria(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlock_Criteria(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Unlock_Criteria_Threshold(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlock_Criteria_Threshold(CVirtualMachine* vm, CVMValue self, int value);
	static CVMObjectHandle Get_Unlock_Criteria_Item(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlock_Criteria_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);

	static CVMObjectHandle Find_By_Name(CVirtualMachine* vm, CVMString name);
	static CVMObjectHandle Find_By_ID(CVirtualMachine* vm, int name);

	static void Bind(CVirtualMachine* machine);
};

#endif
