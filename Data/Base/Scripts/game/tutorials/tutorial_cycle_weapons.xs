// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Cycle Weapons"), 
	Description("Shows an explanation of weapon cycling.") 
]
public class Tutorial_Cycle_Weapons : BaseTutorial
{	
	public serialized(2500) int CycleCount;
	public const int LearnCycleThreshold = 3;

	Tutorial_Cycle_Weapons()
	{
		Description					= "#tutorial_cycle_weapons";
		Title						= "#tutorial_title";
		MinimumInterval				= (60.0 * 1000.0);
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 0;
	}
	
	public override bool Should_Show()
	{
		Actor[] local_pawns = Scene.Get_Local_Controlled_Pawns();
		foreach (Pawn pwn in local_pawns)
		{
			Weapon active_weapon = pwn.Get_Active_Weapon();
			if (active_weapon != null)
			{
				if (active_weapon.Clip_Ammo == 0 && active_weapon.Reserve_Ammo == 0 && !active_weapon.Has_Unlimited_Ammo)
				{
					return true;
				}
			}
		}

		return false;
	}

	public event void On_Pawn_Cycle_Weapon(Event_Pawn_Cycle_Weapon evt)
	{
		if (evt.Instigator != null && evt.Instigator.Owner != null && evt.Instigator.Owner.Is_Local)
		{
			CycleCount++;
			if (CycleCount >= LearnCycleThreshold)
			{
				IsComplete = true;
			}
		}
	}
}
 

