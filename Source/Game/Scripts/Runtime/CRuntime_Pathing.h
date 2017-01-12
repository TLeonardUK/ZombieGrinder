// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_PATHING_
#define _GAME_RUNTIME_PATHING_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Pathing
{
public:
	static CVMObjectHandle Get_Pylon_By_Position(CVirtualMachine* vm, CVMObjectHandle position);
	static CVMObjectHandle Find_Random_Spawn_Area(CVirtualMachine* vm, CVMObjectHandle bounding_box, float min_radius, float max_radius, int can_spawn_on_screen);

	static void Bind(CVirtualMachine* machine);
};

#endif
