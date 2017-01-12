// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.game_mode;
using system.network;
using system.actors.controller;
using system.actors.camera;

[
	Placeable(false), 
	Name("Scene"), 
	Description("Global class, responsible for interacting with the current scene. Spawn objects, getting information, etc.") 
]
public native("Scene") class Scene
{
	property Game_Mode Active_Game_Mode
	{
		public static native("Get_Active_Game_Mode") Game_Mode Get();
		public static native("Set_Active_Game_Mode") void Set(Game_Mode value);
	}

	public static native("Set_Map_Tint") void Set_Map_Tint(Vec4 tint);
	public static native("Get_Map_Tint") Vec4 Get_Map_Tint();

	public static native("Set_Ambient_Lighting") void Set_Ambient_Lighting(Vec4 tint);
	public static native("Get_Ambient_Lighting") Vec4 Get_Ambient_Lighting();
	
	public static native("Set_Clear_Color") void Set_Clear_Color(Vec4 tint);
	public static native("Get_Clear_Color") Vec4 Get_Clear_Color();

	public static native("Get_Map_Dungeon_Level") int Get_Map_Dungeon_Level();

	public static native("Is_Workshop_Map") bool Is_Workshop_Map();
	
	public static native("Get_Bounds") Vec4 Get_Bounds();
	
	//public static native("Get_Active_Camera") Camera Get_Active_Camera();
	public static native("Get_Camera_Count") int Get_Camera_Count();
	
	public static native("Is_Spawned") bool Is_Spawned(object actor);

	public static native("Find_Actor_By_Field") Actor Find_Actor_By_Field(Type class_type, int field_offset, int match);
	public static native("Find_Actors") Actor[] Find_Actors(Type class_type);
	public static native("Find_Enabled_Actors") Actor[] Find_Enabled_Actors(Type class_type);
	public static native("Find_Actors_By_Tag") Actor[] Find_Actors_By_Tag(string tag);
	public static native("Find_Actors_Distance_From_Cameras") Actor[] Find_Actors_Distance_From_Cameras(Type class_type, float min_distance, float max_distance);
	public static native("Find_Actors_In_Radius") Actor[] Find_Actors_In_Radius(Type class_type, float radius, Vec3 center);
	public static native("Find_Actors_In_Area") Actor[] Find_Actors_In_Area(Type class_type, Vec4 bbox);
	public static native("Find_Actors_With_Collision_In_Area") Actor[] Find_Actors_With_Collision_In_Area(Type class_type, Vec4 bbox);
	public static native("Find_Actors_In_FOV") Actor[] Find_Actors_In_FOV(Type class_type, float radius, Vec3 center, float fov, Vec3 heading);
	public static native("Find_Closest_Actor") Actor Find_Closest_Actor(Type class_type, Vec3 center);
	public static native("Find_Closest_Actor_Filtered") Actor Find_Closest_Actor(Type class_type, Vec3 center, object except);

	public static native("Distance_To_Closest_Actor_Of_Type") float Distance_To_Closest_Actor_Of_Type(Vec3 center, Type class_type);
	public static native("Distance_To_Furthest_Actor_Of_Type") float Distance_To_Furthest_Actor_Of_Type(Vec3 center, Type class_type);
	public static native("Distance_To_Enemy_Actors") float Distance_To_Enemy_Actors(Vec3 center, int team_index);
	public static native("Count_Actors_By_Type") int Count_Actors_By_Type(Type class_type);
	
	public static native("Are_Actors_Visible") bool Are_Actors_Visible(Type class_type);
	public static native("Are_Actors_Colliding") bool Are_Actors_Colliding(Type class_type, Vec4 bbox);

	public static native("Spawn") Actor Spawn(Type class_type, NetUser owner);
	public static native("Despawn") void Despawn(Actor actor);	

	public static native("Get_Net_ID_Actor") Actor Get_Net_ID_Actor(int id);
	public static native("Get_Unique_ID_Actor") Actor Get_Unique_ID_Actor(int id);
	
	public static native("Get_Camera_Screen_Viewport_Index") Vec4 Get_Camera_Screen_Viewport_Index(int index);
	public static native("Get_Camera_Index") int Get_Camera_Index(Camera camera);
	public static native("Get_Camera_Screen_Viewport") Vec4 Get_Camera_Screen_Viewport(Camera camera);
	public static native("Get_Camera_Screen_Resolution") Vec2 Get_Camera_Screen_Resolution(Camera camera);

	public static native("Vibrate") void Vibrate(Vec3 center, float duration, float intensity);
		
	// TODO: Move all below to cpp 

	public static Actor[] Get_Controlled_Pawns()
	{
		Actor[] controllers = Find_Actors(typeof(Controller));
		Actor[] result = new Actor[0];
		foreach (Controller ctrl in controllers)
		{
			if (ctrl.Owner != null && ctrl.Possessed != null)
			{	
				result.AddLast(ctrl.Possessed);
			}
		}
		return result;
	}
	
	public static Actor[] Get_Local_Controlled_Pawns()
	{
		Actor[] controllers = Find_Actors(typeof(Controller));
		Actor[] result = new Actor[0];
		foreach (Controller ctrl in controllers)
		{
			if (ctrl.Owner != null && ctrl.Owner.Is_Local == true && ctrl.Possessed != null)
			{	
				result.AddLast(ctrl.Possessed);
			}
		}
		return result;
	}
	
	public static Actor[] Get_Local_User_Controlled_Pawns(int user_index)
	{
		NetUser user = Network.Get_Local_User_By_Index(user_index);

		Actor[] result = new Actor[0];

		if (user.Controller != null && 
			user.Controller.Possessed != null)
		{
			result.AddLast(user.Controller.Possessed);
		}

		return result;
	}

	public static void Trigger_Actors(string tag)
	{
		Actor[] tags = Find_Actors_By_Tag(tag);
		foreach (Actor a in tags)
		{
			a.On_Trigger();
		}
	}
	
}
