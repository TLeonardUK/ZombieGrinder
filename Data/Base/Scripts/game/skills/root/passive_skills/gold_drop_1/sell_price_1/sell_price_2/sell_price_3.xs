// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Sell_Price_3 : Skill_Archetype
{
	Skill_Sell_Price_3()
	{
		Name 				= "#skill_sell_price_3_name";
		Description 		= "#skill_sell_price_3_description";
		Icon_Frame 			= "skill_sell_price_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Sell_Price_2";
		Unlock_Criteria_Threshold = 28;
	}
	
	override void On_Loaded(Profile profile, Skill skill)
	{
        float target = profile.Start_Sell_Price_Multiplier + 0.5;
        if (profile.Stat_Multipliers[Profile_Stat.Sell_Price] < target)
        {
            profile.Stat_Multipliers[Profile_Stat.Sell_Price] = target;
            Log.Write("Adjusted old sell price skill to "+ target);
        }
    } 

	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Sell_Price] = profile.Start_Sell_Price_Multiplier + 0.5;
	}
}
