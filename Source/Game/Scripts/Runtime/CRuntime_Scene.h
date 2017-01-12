// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_SCENE_
#define _GAME_RUNTIME_SCENE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Scene
{
public:
	static CVMObjectHandle Get_Active_Game_Mode(CVirtualMachine* vm);
	static void Set_Active_Game_Mode(CVirtualMachine* vm, CVMObjectHandle actor);

	static CVMObjectHandle Get_Map_Tint(CVirtualMachine* vm);
	static void Set_Map_Tint(CVirtualMachine* vm, CVMObjectHandle actor);

	static CVMObjectHandle Get_Ambient_Lighting(CVirtualMachine* vm);
	static void Set_Ambient_Lighting(CVirtualMachine* vm, CVMObjectHandle actor);

	static CVMObjectHandle Get_Clear_Color(CVirtualMachine* vm);
	static void Set_Clear_Color(CVirtualMachine* vm, CVMObjectHandle actor);

	static int Get_Map_Dungeon_Level(CVirtualMachine* vm);
	static int Is_Workshop_Map(CVirtualMachine* vm);

	static CVMObjectHandle Find_Actors(CVirtualMachine* vm, CVMObjectHandle type);
	static CVMObjectHandle Find_Actor_By_Field(CVirtualMachine* vm, CVMObjectHandle type, int field_offset, int value);	

	static CVMObjectHandle Find_Enabled_Actors(CVirtualMachine* vm, CVMObjectHandle type);
	static CVMObjectHandle Find_Actors_In_Radius(CVirtualMachine* vm, CVMObjectHandle type, float radius, CVMObjectHandle center);
	static CVMObjectHandle Find_Actors_In_Area(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle bbox);
	static CVMObjectHandle Find_Actors_With_Collision_In_Area(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle bbox);
	static CVMObjectHandle Find_Actors_Distance_From_Cameras(CVirtualMachine* vm, CVMObjectHandle type, float min_distance, float max_distance);
	static CVMObjectHandle Find_Actors_In_FOV(CVirtualMachine* vm, CVMObjectHandle type, float radius, CVMObjectHandle center, float fov, CVMObjectHandle heading);
	static CVMObjectHandle Find_Closest_Actor(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle center);
	static CVMObjectHandle Find_Closest_Actor_Filtered(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle center, CVMObjectHandle except);

	static CVMObjectHandle Find_Actors_By_Tag(CVirtualMachine* vm, CVMString type);

	static CVMObjectHandle Spawn(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle owner);
	static void Despawn(CVirtualMachine* vm, CVMObjectHandle obj);
	static int Is_Spawned(CVirtualMachine* vm, CVMObjectHandle obj);

	static int Are_Actors_Colliding(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle bbox);
	static int Are_Actors_Visible(CVirtualMachine* vm, CVMObjectHandle type);

	static float Distance_To_Closest_Actor_Of_Type(CVirtualMachine* vm, CVMObjectHandle center, CVMObjectHandle type);
	static float Distance_To_Furthest_Actor_Of_Type(CVirtualMachine* vm, CVMObjectHandle center, CVMObjectHandle type);
	static float Distance_To_Enemy_Actors(CVirtualMachine* vm, CVMObjectHandle center, int team_index);

	static int Count_Actors_By_Type(CVirtualMachine* vm, CVMObjectHandle type);

	static CVMObjectHandle Get_Net_ID_Actor(CVirtualMachine* vm, int id);
	static CVMObjectHandle Get_Unique_ID_Actor(CVirtualMachine* vm, int id);

	static CVMObjectHandle Get_Camera_Screen_Viewport_Index(CVirtualMachine* vm, int index);
	static int Get_Camera_Index(CVirtualMachine* vm, CVMObjectHandle cam);
	static CVMObjectHandle Get_Camera_Screen_Viewport(CVirtualMachine* vm, CVMObjectHandle cam);
	static CVMObjectHandle Get_Camera_Screen_Resolution(CVirtualMachine* vm, CVMObjectHandle cam);

	static int Get_Camera_Count(CVirtualMachine* vm);

	static void Vibrate(CVirtualMachine* vm, CVMObjectHandle center, float duration, float intensity);

	//static CVMObjectHandle Get_Possessed_Actors(CVirtualMachine* vm);
	//static CVMObjectHandle Get_Local_Possessed_Actors(CVirtualMachine* vm);
	
	static CVMObjectHandle Get_Bounds(CVirtualMachine* vm);

	static float Get_Map_Time(CVirtualMachine* vm);

	//static CVMObjectHandle Get_Active_Camera(CVirtualMachine* vm);

	static void Bind(CVirtualMachine* machine);

};

#endif