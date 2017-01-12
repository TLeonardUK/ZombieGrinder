// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_EFFECTCOMPONENT_
#define _GAME_RUNTIME_EFFECTCOMPONENT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_EffectComponent
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);
	static void Set_Visible(CVirtualMachine* vm, CVMValue self, int val);
	static int Get_Visible(CVirtualMachine* vm, CVMValue self);
	static void Set_Paused(CVirtualMachine* vm, CVMValue self, int val);
	static int Get_Paused(CVirtualMachine* vm, CVMValue self);
	static void Set_One_Shot(CVirtualMachine* vm, CVMValue self, int val);
	static int Get_One_Shot(CVirtualMachine* vm, CVMValue self);

	static void Set_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val);
	static CVMObjectHandle Get_Offset(CVirtualMachine* vm, CVMValue self);

	static void Set_Angle_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val);
	static CVMObjectHandle Get_Angle_Offset(CVirtualMachine* vm, CVMValue self);

	static int Get_Finished(CVirtualMachine* vm, CVMValue self);
	static void Set_Effect_Name(CVirtualMachine* vm, CVMValue self, CVMString val);
	static CVMString Get_Effect_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Instigator(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val);
	static CVMObjectHandle Get_Instigator(CVirtualMachine* vm, CVMValue self);

	static void Set_Ignore_Spawn_Collision(CVirtualMachine* vm, CVMValue self, int val);
	static int Get_Ignore_Spawn_Collision(CVirtualMachine* vm, CVMValue self);

	static void Set_Modifier(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Modifier(CVirtualMachine* vm, CVMValue self);

	static void Set_Meta_Number(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Meta_Number(CVirtualMachine* vm, CVMValue self);

	static void Set_Sub_Type(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Sub_Type(CVirtualMachine* vm, CVMValue self);

	static void Set_Weapon_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Weapon_Type(CVirtualMachine* vm, CVMValue self);

	static void Set_Upgrade_Modifiers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Upgrade_Modifiers(CVirtualMachine* vm, CVMValue self);

	static void Restart(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);
};

#endif
