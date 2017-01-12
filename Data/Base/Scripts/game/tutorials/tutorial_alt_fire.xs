// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Alt Fire"), 
	Description("Shows an explanation of alt fire.") 
]
public class Tutorial_Alt_Fire : BaseTutorial
{	
	public serialized(1) int AltFires;
	public const int LearnAltFireThreshold = 3;

	Tutorial_Alt_Fire()
	{
		Description					= "#tutorial_alt_fire";
		Title						= "#tutorial_title";
		MinimumInterval				= (60.0 * 1000.0) * (AltFires + 1);
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 0;
	}
	
	public event void On_Pawn_Alt_Fire(Event_Pawn_Alt_Fire evt)
	{
		if (evt.Instigator != null && evt.Instigator.Owner != null && evt.Instigator.Owner.Is_Local)
		{
			AltFires++;
			if (AltFires >= LearnAltFireThreshold)
			{
				IsComplete = true;
			}
		}
	}

	public override bool Should_Show()
	{
		MinimumInterval = (60.0 * 1000.0) * (AltFires + 1);

		Actor[] local_pawns = Scene.Get_Local_Controlled_Pawns();
		foreach (Pawn pwn in local_pawns)
		{
			Weapon active_weapon = pwn.Get_Active_Weapon();
			if (active_weapon != null && active_weapon.Can_Alt_Fire)
			{
				return true;
			}
		}

		return false;
	}
}
 

