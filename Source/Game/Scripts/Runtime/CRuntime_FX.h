// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_FX_
#define _GAME_RUNTIME_FX_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_FX
{
public:
	static void Spawn_Untracked(CVirtualMachine* vm, CVMValue self, CVMString effect_name, CVMObjectHandle pos, float dir, int layer, CVMObjectHandle instigator, int meta_number, CVMString modifier, int subtype, CVMObjectHandle weapon_type, CVMObjectHandle upgrade_modifiers);

	static void Invoke_Game_Trigger(CVirtualMachine* vm, CVMObjectHandle instigator);

	static void Bind(CVirtualMachine* machine);
};

#endif
