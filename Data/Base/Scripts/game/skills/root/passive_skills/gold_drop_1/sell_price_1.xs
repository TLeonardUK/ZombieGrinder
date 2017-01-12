// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Sell_Price_1 : Skill_Archetype
{
	Skill_Sell_Price_1()
	{
		Name 				= "#skill_sell_price_1_name";
		Description 		= "#skill_sell_price_1_description";
		Icon_Frame 			= "skill_sell_price_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Gold_Drop_1";
		Unlock_Criteria_Threshold = 4;
	}
	
	override void On_Loaded(Profile profile, Skill skill)
	{
        float target = profile.Start_Sell_Price_Multiplier + 0.1;
        if (profile.Stat_Multipliers[Profile_Stat.Sell_Price] < target)
        {
            profile.Stat_Multipliers[Profile_Stat.Sell_Price] = target;
            Log.Write("Adjusted old sell price skill to "+ target);
        }
    } 

	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Sell_Price] = profile.Start_Sell_Price_Multiplier + 0.1;
	}
}
