// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Sell_Price_2 : Skill_Archetype
{
	Skill_Sell_Price_2()
	{
		Name 				= "#skill_sell_price_2_name";
		Description 		= "#skill_sell_price_2_description";
		Icon_Frame 			= "skill_sell_price_2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Sell_Price_1";
		Unlock_Criteria_Threshold = 15;
	}
	
	override void On_Loaded(Profile profile, Skill skill)
	{
        float target = profile.Start_Sell_Price_Multiplier + 0.2;
        if (profile.Stat_Multipliers[Profile_Stat.Sell_Price] < target)
        {
            profile.Stat_Multipliers[Profile_Stat.Sell_Price] = target;
            Log.Write("Adjusted old sell price skill to "+ target);
        }
    } 

	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Sell_Price] = profile.Start_Sell_Price_Multiplier + 0.2;
	}
}
