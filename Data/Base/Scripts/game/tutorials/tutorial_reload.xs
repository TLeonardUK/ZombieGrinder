// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Reload"), 
	Description("Shows an explanation of manual reloading.") 
]
public class Tutorial_Reload : BaseTutorial
{	
	public serialized(1) int ManualReloads;
	public const int LearnReloadThreshold = 3;

	Tutorial_Reload()
	{
		Description					= "#tutorial_reload";
		Title						= "#tutorial_title";
		MinimumInterval				= (60.0 * 1000.0) * ManualReloads;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 0;
	}
	
	public event void On_Pawn_Manual_Reload(Event_Pawn_Manual_Reload evt)
	{
		if (evt.Reloader != null && evt.Reloader.Owner != null && evt.Reloader.Owner.Is_Local)
		{
			ManualReloads++;
			if (ManualReloads >= LearnReloadThreshold)
			{
				IsComplete = true;
			}
		}
	}

	public override bool Should_Show()
	{
		MinimumInterval = (60.0 * 1000.0) * (ManualReloads + 1);

		Actor[] local_pawns = Scene.Get_Local_Controlled_Pawns();
		foreach (Pawn pwn in local_pawns)
		{
			Weapon active_weapon = pwn.Get_Active_Weapon();
			if (active_weapon != null)
			{
				if (active_weapon.Clip_Ammo <= active_weapon.Clip_Size * 0.33f)
				{
					return true;
				}
			}
		}

		return false;
	}
}
 

