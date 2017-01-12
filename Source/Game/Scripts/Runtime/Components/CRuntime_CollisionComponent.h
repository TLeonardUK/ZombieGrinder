// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_COLLISIONCOMPONENT_
#define _GAME_RUNTIME_COLLISIONCOMPONENT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_CollisionComponent
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);

	static void Set_Enabled(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Enabled(CVirtualMachine* vm, CVMValue self);

	static void Set_Blocks_Path(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Blocks_Path(CVirtualMachine* vm, CVMValue self);

	static void Set_Blocks_Path_Spawns(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Blocks_Path_Spawns(CVirtualMachine* vm, CVMValue self);

	static void Set_Shape(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Shape(CVirtualMachine* vm, CVMValue self);

	static void Set_Type(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Type(CVirtualMachine* vm, CVMValue self);

	static void Set_Group(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Group(CVirtualMachine* vm, CVMValue self);

	static void Set_Collides_With(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Collides_With(CVirtualMachine* vm, CVMValue self);

	static void Set_Area(CVirtualMachine* vm, CVMValue self, CVMObjectHandle visible);
	static CVMObjectHandle Get_Area(CVirtualMachine* vm, CVMValue self);

	static void Set_Velocity(CVirtualMachine* vm, CVMValue self, CVMObjectHandle visible);
	static CVMObjectHandle Get_Velocity(CVirtualMachine* vm, CVMValue self);

	static void Set_Smoothed(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Smoothed(CVirtualMachine* vm, CVMValue self);

	static int Is_Colliding(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);

};

#endif