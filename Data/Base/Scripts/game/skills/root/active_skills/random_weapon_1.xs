// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Random_Weapon_1 : Skill_Archetype
{
	private Type[] m_weapon_table = 
	{
		typeof(Weapon_Grenade_Launcher),
		typeof(Weapon_RPC),
		typeof(Weapon_Laser_Cannon),
		typeof(Weapon_Rocket_Launcher),
		typeof(Weapon_Rocket_Launcher_2),
		typeof(Weapon_Rocket_Launcher_3),
		typeof(Weapon_Frag),
		typeof(Weapon_Molotov),
		typeof(Weapon_Sword),
		typeof(Weapon_Flamethrower),
		typeof(Weapon_Freeze_Ray),
		typeof(Weapon_Acid_Gun),
		typeof(Weapon_Turret),
		typeof(Weapon_Shock_Rifle),
		typeof(Weapon_Bolt_Gun),
		typeof(Weapon_Sticky_Launcher),
		typeof(Weapon_Buff_Grenade_Ammo),
		typeof(Weapon_Buff_Grenade_Damage),
		typeof(Weapon_Buff_Grenade_Health),
		typeof(Weapon_Buff_Grenade_Price),
		typeof(Weapon_Buff_Grenade_Reload),
		typeof(Weapon_Buff_Grenade_ROF),
		typeof(Weapon_Buff_Grenade_Speed),
		typeof(Weapon_Buff_Grenade_XP),
		typeof(Weapon_Love_Cannon),
		typeof(Weapon_Love_Thrower),
		typeof(Weapon_DB_Shotgun),
		typeof(Weapon_Scatter_Gun),
		typeof(Weapon_Shotgun),
		typeof(Weapon_Spaz),
		typeof(Weapon_Uzi),
		typeof(Weapon_Magnum),
		typeof(Weapon_Flare_Gun),
		typeof(Weapon_Baseball_Bat),
		typeof(Weapon_Fight_Saber),
		typeof(Weapon_Ak47),
		typeof(Weapon_Assault_Rifle),
		typeof(Weapon_Winchester_Rifle)
	};

	//typeof(Weapon_Tripmine),

	Skill_Random_Weapon_1()
	{
		Name 				= "#skill_random_weapon_1_name";
		Description 		= "#skill_random_weapon_1_description";
		Icon_Frame 			= "skill_random_weapon_1";
		Cost 				= 1;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 200;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_Active_Skills";
		Is_Team_Based		= false;
		Duration			= 0.0f;
		Player_Effect		= "random_weapon_player";
        Unlock_Criteria_Threshold = 5;
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
