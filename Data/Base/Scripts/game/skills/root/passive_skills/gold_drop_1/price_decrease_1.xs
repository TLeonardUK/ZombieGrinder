// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Price_Decrease_1 : Skill_Archetype
{
	Skill_Price_Decrease_1()
	{
		Name 				= "#skill_price_decrease_1_name";
		Description 		= "#skill_price_decrease_1_description";
		Icon_Frame 			= "skill_price_decrease_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Gold_Drop_1";
		Unlock_Criteria_Threshold = 8;
	}
	
	override void On_Loaded(Profile profile, Skill skill)
	{
        float target = 0.9f;
        if (profile.Stat_Multipliers[Profile_Stat.Buy_Price] > target)
        {
            profile.Stat_Multipliers[Profile_Stat.Buy_Price] = target;
            Log.Write("Adjusted old buy price skill to " + target);
        }
    } 

	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Buy_Price] = 0.9;
	}
}
