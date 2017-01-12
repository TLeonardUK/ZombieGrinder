// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item;
using game.items.item_archetype;
using game.skills.skill;
using game.skills.skill_archetype;
using game.weapons.weapon;
using system.locale;

// Keep in sync with enum in Profile.h. Specifies all the stat-multipliers
// that can be used to adjust player abilities. Mainly changed by skills.
public enum Profile_Stat
{
	Wallet_Interest,

	Max_Health,
	Health_Regen,
	Speed,
	Ammo_Capacity,
	Ammo_Regen,
	Gold_Drop,
	Gold_Magnet,
	Buy_Price,
	Sell_Price,
	Gem_Drop,
	Gem_Magnet,
	XP,

	Melee_Damage,
	Melee_Stamina,

	Rifle_Damage,
	Rifle_Ammo,

	Heavy_Weapon_Damage,
	Heavy_Weapon_Ammo,
	Explosion_Radius,
	Enemy_Status_Duration,

	Turret_Damage,
	Turret_Ammo,

	Small_Arms_Damage,
	Small_Arms_Ammo,

	Buff_Grenade_Radius,
	Buff_Grenade_Ammo,
	 
	Ghost_Life_Time,
	Ghost_Revive_Time,
	Healing_Speed,
	
	// Generic multipliers
	Damage,
	Damage_Taken,
	Rate_Of_Fire,
	Reload_Speed,
	Ammo_Usage,
	Targetable,

	COUNT
}

// Keep in sync with enum in Profile.h. Specifies all the types of per-profile statistics
// stored about individual items.
public enum Profile_Item_Stat_Type
{
	Ailments_Inflicted,
	Damage_Inflicted,
	Enemies_Killed, // STAT_TODO
	Shots_Fired
}

public struct Profile_Item_Stat
{
	// Order is assumed in CPP code, because of the dumbs.
	// Check Profile.h/Profile.cpp files whenever you change this.
	
	public serialized(1202) forceoffset(0) int						Item_ID;
	public serialized(1202) forceoffset(1) Profile_Item_Stat_Type 	Stat_Type;
	public serialized(1202) forceoffset(2) float 					Value;
}

public struct Profile_Skill_Energy
{
	// Order is assumed in CPP code, because of the dumbs.
	// Check Profile.h/Profile.cpp files whenever you change this.
	
	public serialized(3500) forceoffset(0) int						Skill_ID;
	public serialized(3500) forceoffset(1) Skill_Energy_Type 	    Energy_Type;
	public serialized(3500) forceoffset(2) float 					Value;
}

[
	Name("Profile"), 
	Description("A profile is an individual set of equips and stats that make up a single character in the game. The player can have many of these and can switch between them at will.") 
]
public native("Profile") class Profile
{
	property serialized(1) string Name
	{
		public native("Get_Name") string Get();
		public native("Set_Name") void Set(string value);		
	}
	
	property serialized(1) int Level
	{
		public native("Get_Level") int Get();
		public native("Set_Level") void Set(int value);	
	}
	
	property serialized(1) int Level_XP
	{
		public native("Get_Level_XP") int Get();
		public native("Set_Level_XP") void Set(int value);	
	}
		
	property int Next_Level_XP
	{
		public native("Get_Next_Level_XP") int Get();
	}
	
	property serialized(1) int Coins
	{
		public native("Get_Coins") int Get();
		public native("Set_Coins") void Set(int value);	
	}
	
	property serialized(50) float Coins_Interest
	{
		public native("Get_Coins_Interest") float Get();
		public native("Set_Coins_Interest") void Set(float value);	
	}
	
	property serialized(1) int Wallet_Size
	{
		public native("Get_Wallet_Size") int Get();
		public native("Set_Wallet_Size") void Set(int value);	
	}
	
	property serialized(1) float[] Stat_Multipliers
	{
		public native("Get_Stat_Multipliers") float[] Get();
		public native("Set_Stat_Multipliers") void Set(float[] value);		
	}
	
	property serialized(1) int Inventory_Size
	{
		public native("Get_Inventory_Size") int Get();
		public native("Set_Inventory_Size") void Set(int value);	
	}
	
	property serialized(1) int Skill_Points
	{
		public native("Get_Skill_Points") int Get();
		public native("Set_Skill_Points") void Set(int value);	
	}
	
	property serialized(1) Skill[] Skills
	{
		public native("Get_Skills") Skill[] Get();
		public native("Set_Skills") void Set(Skill[] value);	
	} 
	
	property serialized(1) Item[] Items
	{
		public native("Get_Items") Item[] Get();
		public native("Set_Items") void Set(Item[] value);	
	}
	
	property serialized(1) int Unique_ID_Counter
	{
		public native("Get_Unique_ID_Counter") int Get();
		public native("Set_Unique_ID_Counter") void Set(int value);	
	}
	
	property serialized(200) int[] Applied_DLC_IDs
	{
		public native("Get_Applied_DLC_IDs") int[] Get();
		public native("Set_Applied_DLC_IDs") void Set(int[] value);		
	}	

	property serialized(1200) int[] Unlocked_Item_IDs
	{
		public native("Get_Unlocked_Item_IDs") int[] Get();
		public native("Set_Unlocked_Item_IDs") void Set(int[] value);		
	}	
	
	property serialized(1201) int[] Unlocked_Skill_IDs
	{
		public native("Get_Unlocked_Skill_IDs") int[] Get();
		public native("Set_Unlocked_Skill_IDs") void Set(int[] value);		
	}	

	property serialized(2500) bool Is_Male
	{
		public native("Get_Is_Male") bool Get();
		public native("Set_Is_Male") void Set(bool value);		
	}	
	
	property serialized(1202) Profile_Item_Stat[] Profile_Item_Stats
	{
		public native("Get_Profile_Item_Stats") Profile_Item_Stat[] Get();
		public native("Set_Profile_Item_Stats") void Set(Profile_Item_Stat[] value);		
	}	

	property serialized(3500) Profile_Skill_Energy[] Profile_Skill_Energies
	{
		public native("Get_Profile_Skill_Energies") Profile_Skill_Energy[] Get();
		public native("Set_Profile_Skill_Energies") void Set(Profile_Skill_Energy[] value);		
	}	
		
	// Skill operations.
	public native("Unlock_Skill") 				Skill Unlock_Skill(Skill_Archetype archetype);
	public native("Has_Skill") 					bool  Has_Skill(Skill_Archetype archetype); 
	public native("Get_Skill") 					Skill Get_Skill(Skill_Archetype archetype); 	
	public native("Get_Skill_Slot") 			Skill Get_Skill_Slot(int index); 

	public native("Add_Unpacked_Fractional_XP") void Add_Unpacked_Fractional_XP(float amount);
	public native("Add_Unpacked_Item") void Add_Unpacked_Item(Item_Archetype type);
	public native("Add_Unpacked_Coins") void Add_Unpacked_Coins(int coins);
	
	// Item operations.
	public native("Get_Inventory_Space")	int		Get_Inventory_Space();
	public native("Has_Space_For")			bool	Has_Space_For(Item_Archetype archetype);
	public native("Add_Item") 				Item  	Add_Item(Item_Archetype archetype);
	public native("Has_Item") 			bool  	Has_Item(Item_Archetype archetype); 

	public native("Get_Preferred_Consume_Item") 			Item  	Get_Preferred_Consume_Item(Item_Archetype archetype); 
	public native("Get_Item") 			Item  	Get_Item(Item_Archetype archetype); 
	public native("Remove_Item") 		void 	Remove_Item(Item item, bool bConsume = false); 
	public native("Get_Item_Slot") 		Item  	Get_Item_Slot(Item_Slot slot); 
	public native("Equip_Item") 		Item  	Equip_Item(Item item); 
	public native("Combine_Items") 		void  	Combine_Items(Item item_a, Item item_b); 
	public native("Get_Attached_Items") Item[]  Get_Attached_Items(Item item); 

	// Combine stats.
	public native("Combine_Stat_Multipliers") void Combine_Stat_Multipliers(float[] output, float[] skill_multipliers, bool bAllowHealthRegen, bool bAllowAmmoRegen); 
	 
	// Level up operations.
	public native("Level_Up")				void  Level_Up();
	public native("Give_XP")				bool  Give_XP(int count); // Returns true if XP caused a level up.
	public native("Give_Fractional_XP")		bool  Give_Fractional_XP(float x); // Gives a fraction of the XP to the next level.

	// Item stats.
	public native("Get_Item_Stat")			float Get_Item_Stat(Item_Archetype archetype, Profile_Item_Stat_Type stat);
	public native("Set_Item_Stat")			void  Set_Item_Stat(Item_Archetype archetype, Profile_Item_Stat_Type stat, float value);
	public native("Increment_Item_Stat")	void  Increment_Item_Stat(Item_Archetype archetype, Profile_Item_Stat_Type stat, float value);
	
    // Skill values.
    public native("Set_Skill_Energy")           void Set_Skill_Energy(Skill_Archetype skill, float value);
    public native("Get_Skill_Energy")           float Get_Skill_Energy(Skill_Archetype skill);
    public native("Increment_Skill_Energies")   float Increment_Skill_Energies(Skill_Energy_Type type, float value);

	// Dun dun dun, the maximum levels we support!
	const int Level_Cap = 99;
	
	// How much it costs per-skill-point to reroll skills.
	const int Skill_Reroll_Cost = 100;
	
	// Skill points gained per level.
	const int Skill_Points_Per_Level = 1;
	
	// Starting values.
	const int Start_Wallet_Size 			= 250000;
    const int Inventory_Slots_Per_Page      = 20;
	const int Start_Inventory_Size 			= 20; // 1 page as default.
    const int Max_Inventory_Size            = 16 * 20; // base 1 page + premium 16 + 3 upgrade pages.
	const float Start_Sell_Price_Multiplier	= 0.5;
	
	// Invoked from host ProfileManager to calculate xp-requirements for each level.
	// This is only called once for each level at the start of the game, the result
	// is cached in a lookup table. It's relatively safe to make changes to this 
	// post-release as we track xp on a per-level basis not globally, meaning changes
	// should not effective a users progression significantly.
	static int XP_Curve(int level)
	{
		// This butt load of fuck basically produces a smooth exponential curve
		// between the values first_level_xp and final_level_xp. Values are rounded
		// to power of 10 figures. (So you show 12300 rather than 12386 etc).
	
		float avg_xp_per_enemy = 500;
		float final_level_xp = avg_xp_per_enemy * 10000;
		float first_level_xp = avg_xp_per_enemy * 25;
		
		float coeff_b = Math.Log(final_level_xp / first_level_xp) / <float>Level_Cap;
		float coeff_a = first_level_xp / (Math.Exp(coeff_b) - 1.0);
		float rounding = 2;
		
		float x = coeff_a * Math.Exp(coeff_b * level);
		float y = Math.Pow(10, Math.Floor(Math.Log10(Math.Abs(x))) - rounding);	
	
		return (<int>(x / y)) * y;	
	}

	// Called when stats are rerolled, should reset any skill-affected values.
	void On_Reroll_Skills()
	{
		Wallet_Size 			= Start_Wallet_Size;
		Inventory_Size 			= Start_Inventory_Size;
	
		// Reset all stats to 1 by default as they are multipliers.
		for (int i = 0; i < Profile_Stat.COUNT; i++)
		{
			Stat_Multipliers[i] = 1.0;
		}
		
		// Couple of them require specific starting values.
		Stat_Multipliers[Profile_Stat.Health_Regen] 	= 0.0;
		Stat_Multipliers[Profile_Stat.Ammo_Regen] 		= 0.0;
		Stat_Multipliers[Profile_Stat.Wallet_Interest] 	= 0.0;
		Stat_Multipliers[Profile_Stat.Sell_Price] 		= Start_Sell_Price_Multiplier;
	}

    // Called when profile is loaded, used to sanitize any invalid properties.
    void Sanitize()
    {
        if (Unlocked_Item_IDs == null)
        {
		    Unlocked_Item_IDs = new int[0];
        }
        if (Unlocked_Skill_IDs == null)
        {
		    Unlocked_Skill_IDs	= new int[0];
        }
        if (Profile_Item_Stats == null)
        {
            Profile_Item_Stats = new Profile_Item_Stat[0];
        }
        if (Profile_Skill_Energies == null)
        {
            Profile_Skill_Energies = new Profile_Skill_Energy[0];
        }

        // We reset inventory size to 0, the skills will adjust it correctly in their On_Loaded method.
        Inventory_Size = Start_Inventory_Size;
    }
	 
	// Default constructor.
	Profile()
	{
		Name  				    = Locale.Get("#menu_select_character_untitled_name");
		Level 				    = 1;
		Level_XP			    = 0;
		Skill_Points		    = 5;
		Coins				    = 30000;
		Inventory_Size		    = Start_Inventory_Size;
		Wallet_Size			    = Start_Wallet_Size;
		Items				    = new Item[0];
		Skills				    = new Skill[0];
		Applied_DLC_IDs		    = new int[0];
		Unlocked_Item_IDs	    = new int[0];
		Unlocked_Skill_IDs	    = new int[0];
		Stat_Multipliers	    = new float[Profile_Stat.COUNT];
		Profile_Item_Stats	    = new Profile_Item_Stat[0];
		Profile_Skill_Energies  = new Profile_Skill_Energy[0];

		// Reset skills to base.
		On_Reroll_Skills();
		
		Item head_1 = Add_Item(Item_Archetype.Find_By_Name("Item_Player1hair"));
		Item head_2 = Add_Item(Item_Archetype.Find_By_Name("Item_Player2hair"));
		Item head_3 = Add_Item(Item_Archetype.Find_By_Name("Item_Player3hair"));
		Item head_4 = Add_Item(Item_Archetype.Find_By_Name("Item_Player4hair"));
		Item head_5 = Add_Item(Item_Archetype.Find_By_Name("Item_Little_Sis"));
		Item head_6 = Add_Item(Item_Archetype.Find_By_Name("Item_Poniteru"));

		Add_Item(Item_Archetype.Find_By_Name("Item_Uzi"));
		Add_Item(Item_Archetype.Find_By_Name("Item_Shotgun"));

		Item paint_1 = Add_Item(Item_Archetype.Find_By_Name("Item_Dosh"));
		Item paint_2 = Add_Item(Item_Archetype.Find_By_Name("Item_Burnt_Orange"));		
		Item paint_3 = Add_Item(Item_Archetype.Find_By_Name("Item_Ultra_Violet"));
		Item paint_4 = Add_Item(Item_Archetype.Find_By_Name("Item_Truish_Blue_Paint"));
		Item paint_5 = Add_Item(Item_Archetype.Find_By_Name("Item_Smooth_Criminal"));
		Item paint_6 = Add_Item(Item_Archetype.Find_By_Name("Item_Lukewarm_Pink"));
		
		// Randomize the default head a bit.
		int rand_head = Math.Rand(0, 7);
		int rand_paint = Math.Rand(0, 7);
		Item default_paint;
		Item default_head;

		if (rand_head == 0) 	 default_head = head_1;
		else if (rand_head == 1) default_head = head_2;
		else if (rand_head == 2) default_head = head_3;
		else if (rand_head == 3) default_head = head_4;
		else if (rand_head == 4) default_head = head_5;
		else                     default_head = head_6;

		if (rand_paint == 0) 	  default_paint = paint_1;
		else if (rand_paint == 1) default_paint = paint_2;
		else if (rand_paint == 2) default_paint = paint_3;
		else if (rand_paint == 3) default_paint = paint_4;
		else if (rand_paint == 4) default_paint = paint_5;
		else                      default_paint = paint_6;
						
		Equip_Item(default_head);
		Combine_Items(default_head, default_paint);		
	}
}
