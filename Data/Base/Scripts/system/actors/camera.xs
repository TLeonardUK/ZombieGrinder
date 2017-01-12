// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.scene;

[
	Placeable(false), 
	Name("Camera"), 
	Description("Base class for all specialized cameras.") 
]
public class Camera
{
	// Called by native code each frame to get the current viewport the 
	// camera is framing.
	public abstract Vec4 Get_Viewport();
	
	// Gets the actor currently being spectated. Or null if not spectating.
	public virtual Actor Get_Spectate_Target()
	{
		return null;
	}

	// Returns true if in spectating mode.
	public virtual bool Is_Spectating()
	{
		return false;
	}
	
	// Shakes the cameras viewport.
	public virtual void Shake(float duration, float intensity, Vec3 position, Actor instigator, float chromatic_abberation_intensity = 0.0f)
	{
		// Do in derived class.
	}

	// Shakes all active cameras.
	public static void Shake_All(float duration, float intensity, Vec3 position, Actor instigator, float chromatic_abberation_intensity = 0.0f)
	{
		Game_Mode mode = Scene.Active_Game_Mode;
		int camera_count = mode.Get_Camera_Count();
		for (int i = 0; i < camera_count; i++)
		{
			mode.Get_Camera(i).Shake(duration, intensity, position, instigator, chromatic_abberation_intensity);
		}
	}
}
