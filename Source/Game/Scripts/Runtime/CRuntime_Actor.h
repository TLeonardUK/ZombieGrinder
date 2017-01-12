// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ACTOR_
#define _GAME_RUNTIME_ACTOR_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Actor
{
public:
	static void Hibernate(CVirtualMachine* vm, CVMValue self);
	static void Wake_Up(CVirtualMachine* vm, CVMValue self);
	static int Is_Hibernating(CVirtualMachine* vm, CVMValue self);
	static int Is_Spawned(CVirtualMachine* vm, CVMValue self);

	static int Get_Net_ID(CVirtualMachine* vm, CVMValue self);
	static int Get_Unique_ID(CVirtualMachine* vm, CVMValue self);

	static void Set_Tag(CVirtualMachine* vm, CVMValue self, CVMString pos);
	static CVMString Get_Tag(CVirtualMachine* vm, CVMValue self);
	static void Set_Link(CVirtualMachine* vm, CVMValue self, CVMString pos);
	static CVMString Get_Link(CVirtualMachine* vm, CVMValue self);
	
	static void Set_Activated_By_Player(CVirtualMachine* vm, CVMValue self, int pos);
	static int Get_Activated_By_Player(CVirtualMachine* vm, CVMValue self);

	static void Set_Enabled(CVirtualMachine* vm, CVMValue self, int pos);
	static int Get_Enabled(CVirtualMachine* vm, CVMValue self);
	static void Set_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos);
	static CVMObjectHandle Get_Position(CVirtualMachine* vm, CVMValue self);
	static void Set_Rotation(CVirtualMachine* vm, CVMValue self, float pos);
	static float Get_Rotation(CVirtualMachine* vm, CVMValue self);
	static void Set_Scale(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos);
	static CVMObjectHandle Get_Scale(CVirtualMachine* vm, CVMValue self);
	static void Set_Layer(CVirtualMachine* vm, CVMValue self, int pos);
	static int Get_Layer(CVirtualMachine* vm, CVMValue self);
	static void Set_Depth_Bias(CVirtualMachine* vm, CVMValue self, float pos);
	static float Get_Depth_Bias(CVirtualMachine* vm, CVMValue self);
	static void Set_Depth_Y_Offset(CVirtualMachine* vm, CVMValue self, float pos);
	static float Get_Depth_Y_Offset(CVirtualMachine* vm, CVMValue self);
	static void Set_Bounding_Box(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos);
	static CVMObjectHandle Get_Bounding_Box(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_World_Bounding_Box(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Screen_Bounding_Box(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Center(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Collision_Center(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Collision_Box(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_World_Collision_Box(CVirtualMachine* vm, CVMValue self);
	static void Set_Owner(CVirtualMachine* vm, CVMValue self, CVMObjectHandle owner);
	static CVMObjectHandle Get_Owner(CVirtualMachine* vm, CVMValue self);

	static void Set_Parent(CVirtualMachine* vm, CVMValue self, CVMObjectHandle owner);
	static CVMObjectHandle Get_Parent(CVirtualMachine* vm, CVMValue self);

	static void Set_Tick_Offscreen(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Tick_Offscreen(CVirtualMachine* vm, CVMValue self);
	static void Set_Tick_Priority(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Tick_Priority(CVirtualMachine* vm, CVMValue self);
	static void Set_Tick_Area(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Tick_Area(CVirtualMachine* vm, CVMValue self);

	static void MoveTo(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos);
	static void Teleport(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos);
	static void LimitMovement(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos);

	static CVMObjectHandle Get_Last_Frame_Movement(CVirtualMachine* vm, CVMValue self);

	static void Deactivate_Components(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);

};

#endif