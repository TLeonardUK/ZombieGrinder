// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Random_Rifle_1 : Skill_Archetype
{
	private Type[] m_weapon_table = 
	{
		typeof(Weapon_Ak47),
		typeof(Weapon_Assault_Rifle),
		typeof(Weapon_Winchester_Rifle)
	};

	Skill_Random_Rifle_1()
	{
		Name 				= "#skill_random_rifle_1_name";
		Description 		= "#skill_random_rifle_1_description";
		Icon_Frame 			= "skill_random_rifle_1";
		Cost 				= 2;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 300;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Random_Weapon_1";
		Is_Team_Based		= false;
		Duration			= 0.0f;
		Player_Effect		= "random_weapon_player";
		Unlock_Criteria_Threshold = 37;
	}

	public override void Apply(NetUser user)
	{
		if (Network.Is_Server())
		{
			if (user.Controller != null)
			{
				Pawn pawn = user.Controller.Possessed;
				if (pawn != null)
				{
					pawn.Give_Weapon(m_weapon_table[Math.Rand(0, m_weapon_table.Length())]);
				}
			}	
		}
	}
}
