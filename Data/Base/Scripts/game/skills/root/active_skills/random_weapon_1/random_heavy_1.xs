// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Random_Heavy_1 : Skill_Archetype
{
	private Type[] m_weapon_table = 
	{
		typeof(Weapon_Grenade_Launcher),
		typeof(Weapon_RPC),
		typeof(Weapon_Laser_Cannon),
		typeof(Weapon_Rocket_Launcher),
		typeof(Weapon_Rocket_Launcher_2),
		typeof(Weapon_Rocket_Launcher_3),
		typeof(Weapon_Flamethrower),
		typeof(Weapon_Freeze_Ray),
		typeof(Weapon_Acid_Gun),
		typeof(Weapon_Turret),
		typeof(Weapon_Tripmine)
	};

	Skill_Random_Heavy_1()
	{
		Name 				= "#skill_random_heavy_1_name";
		Description 		= "#skill_random_heavy_1_description";
		Icon_Frame 			= "skill_random_heavy_1";
		Cost 				= 2;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Random_Weapon_1";
		Is_Team_Based		= false;
		Duration			= 0.0f;
		Player_Effect		= "random_weapon_player";
		Unlock_Criteria_Threshold = 41;
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
