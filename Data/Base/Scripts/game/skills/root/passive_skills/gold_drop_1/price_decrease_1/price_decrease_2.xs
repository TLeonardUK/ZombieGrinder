// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Price_Decrease_2 : Skill_Archetype
{
	Skill_Price_Decrease_2()
	{
		Name 				= "#skill_price_decrease_2_name";
		Description 		= "#skill_price_decrease_2_description";
		Icon_Frame 			= "skill_price_decrease_2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Price_Decrease_1";
		Unlock_Criteria_Threshold = 16;
	}
	
	override void On_Loaded(Profile profile, Skill skill)
	{
        float target = 0.8f;
        if (profile.Stat_Multipliers[Profile_Stat.Buy_Price] > target)
        {
            profile.Stat_Multipliers[Profile_Stat.Buy_Price] = target;
            Log.Write("Adjusted old buy price skill to " + target);
        }
    } 

	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Buy_Price] = 0.8;
	}
}
