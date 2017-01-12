// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.input;
using game.profile.profile;
using system.actors.controller;
using game.skills.skill_effect_manager;

// -----------------------------------------------------------------------------
//  Represents a single user currently connected to the game.
//
//	WARNING: Be very careful what you mark as serialized in this class as it
//			 will be sent via the period state-update. The only things that
//			 should be marked as serialized is things such as score/team/etc.
// -----------------------------------------------------------------------------
public native("NetUser") class NetUser
{
	property int Team_Index 
	{
		public native("Get_Team_Index") int Get();
		public native("Set_Team_Index") void Set(int value);
	}
	
	property bool In_Game
	{
		public native("Get_In_Game") bool Get();
	}
	
	property bool Is_Local
	{
		public native("Get_Is_Local") bool Get();
	}
	
	property bool Is_Premium
	{
		public native("Get_Is_Premium") bool Get();
	}
	
	property string Username
	{
		public native("Get_Username") string Get();
	}
	
	property string Display_Username
	{
		public native("Get_Display_Username") string Get();
	}	
	
	// Unique first-party identifier (sony=npid, steam=steam_id_***, etc)
	property string First_Party_ID
	{
		public native("Get_First_Party_ID") string Get();
	}
	
	property int Net_ID
	{
		public native("Get_Net_ID") int Get();
	}
		
	property int Local_Index
	{
		public native("Get_Local_Index") int Get();
	}
	
	property Controller Controller
	{
		public native("Get_Controller") Controller Get();
		public native("Set_Controller") void Set(Controller value);
	}
	
	property Profile Profile
	{
		public native("Get_Profile") Profile Get();
	}
	
	property int Profile_Change_Counter
	{
		public native("Get_Profile_Change_Counter") int Get();
	}
		
	// Gets the local non-synced profile. Use this if you want to persistently
	// increment xp/etc.
	property Profile Local_Profile
	{
		public native("Get_Local_Profile") Profile Get();
	}
	
	property int Coins 
	{
		public native("Get_Coins") int Get();
		public native("Set_Coins") void Set(int value);
	}

	// Updates the heading used for spawning.
	public native("Update_Heading") void Update_Heading(Actor other);

	// Score state.
	public serialized(1) int Score;
	public serialized(1) int Kills;
	public serialized(1) int Deaths;
	public serialized(1) int Revives;
    
    public serialized(1) bool Waiting_For_Zombie_Control;

	public float Display_Coins;
	public int Picked_Up_Coins;
	public float Pickup_Coin_Timer;

	public float Time_Since_Last_Damage;
	public float Time_Since_Last_Fire;

	const float HEALTH_REGEN_DELAY_TIME = 15.0f;
	const float AMMO_REGEN_DELAY_TIME = 15.0f;

	const float MINIMUM_PICKUP_SYNC_TIME = 0.25f;

    public float Display_XP = 0.0f;

	// Spawning control.
	public float Last_Spawn_Time;	
	public float First_Spawn_Timer;

	// Skill control.
	public float[] Skill_Stat_Multipliers = new float[Profile_Stat.COUNT];

	// Final stat multipliers, combination of skill+profile stats.
	public float[] Stat_Multipliers = new float[Profile_Stat.COUNT];
	
	// Constructor just resets them.
	NetUser()
	{
		Reset();
	}

	void Reset()
	{
		Last_Spawn_Time = 0.0;
		First_Spawn_Timer = 0.0;

		Time_Since_Last_Damage = 0.0f;
		Time_Since_Last_Fire = 0.0f;

		Coins = Cheat_Manager.StartCash;
		Kills = 0;
		Deaths = 0;
		Revives = 0;

		Picked_Up_Coins = 0;
		Display_Coins = 0;
		Pickup_Coin_Timer = 0.0f;

        Waiting_For_Zombie_Control = false;

		Skill_Stat_Multipliers = new float[Profile_Stat.COUNT];
		Stat_Multipliers = new float[Profile_Stat.COUNT];

		Reset_Skill_Multipliers();
	}

	// Invoked by host code when new map is loaded, used to reset
	// script side variables.
	default state Idle
	{
		private event void On_Map_Load()
		{
			Reset();
		}

		private event void On_Tick()
		{
			// If we don't have a profile yet, no need to do any of this.
			if (this.Profile == null)
			{
				return;
			}

			Time_Since_Last_Damage += Time.Get_Delta();
			Time_Since_Last_Fire += Time.Get_Delta();

			Display_Coins = Math.Lerp(Display_Coins, Coins, 1.0f * Time.Get_Delta());

			Pickup_Coin_Timer -= Time.Get_Delta();

			if (Picked_Up_Coins > 0 && Pickup_Coin_Timer <= 0)
			{
				rpc(RPCTarget.Server, RPCTarget.None) RPC_Sync_Pickup_Coins(Net_ID, Picked_Up_Coins);

				Pickup_Coin_Timer = MINIMUM_PICKUP_SYNC_TIME;
				Picked_Up_Coins = 0;
			}

			Combine_Stat_Multipliers();
		}
	}

	public void Reset_Skill_Multipliers()
	{
		Skill_Stat_Multipliers.ClearElements(1.0f);

		// Couple of them require specific starting values.
		Skill_Stat_Multipliers[Profile_Stat.Health_Regen] 		= 0.0;
		Skill_Stat_Multipliers[Profile_Stat.Ammo_Regen] 		= 0.0;
		Skill_Stat_Multipliers[Profile_Stat.Wallet_Interest] 	= 0.0;
	}
	
	// Combines profile+skill multipliers.
	private void Combine_Stat_Multipliers()
	{
		this.Profile.Combine_Stat_Multipliers(this.Stat_Multipliers, this.Skill_Stat_Multipliers, Time_Since_Last_Damage >= HEALTH_REGEN_DELAY_TIME, Time_Since_Last_Fire >= AMMO_REGEN_DELAY_TIME);

		/*
		float[] skill_mult_array	= this.Skill_Stat_Multipliers;
		float[] profile_mult_array  = this.Profile.Stat_Multipliers;

		for (int i = 0; i < Profile_Stat.COUNT; i++)
		{
			float skill_mult    = skill_mult_array[i];
			float profile_mult  = profile_mult_array[i];

			if (i == Profile_Stat.Health_Regen || 
				i == Profile_Stat.Ammo_Regen || 
				i == Profile_Stat.Wallet_Interest)
			{
				Stat_Multipliers[i] = skill_mult + profile_mult;
			}
			else
			{
				Stat_Multipliers[i] = skill_mult * profile_mult;
			}
		}
		*/
	}

	// Sent by clients to server to notify you have locally picked up coins.
	private static rpc void RPC_Sync_Pickup_Coins(int net_id, int count)
	{
		if (Network.Is_Server())
		{
			NetUser user = Network.Get_User(net_id);
			user.Coins += count;

		//	Log.Write(user.Username+" picked up "+count+", total is now "+user.Coins);
		}
	}
	
	// Used by local clients to keep track of coins picked up locally and sync them to server.
	public void Pickup_Coins(int amount)
	{
		Picked_Up_Coins += amount;

		Event_Coins_Collected evt = new Event_Coins_Collected();
		evt.User		= this;
		evt.Collected	= amount;
		evt.Fire_Global();
	}
} 

// -----------------------------------------------------------------------------
//	This class is used to perform several network operations.
// -----------------------------------------------------------------------------
public enum ChatType
{
	Private,
	Global,
	Action,
	Server
}

public static native("Network") class Network
{
	public static native("Is_Server") 						bool 		Is_Server();
	public static native("Is_Client") 						bool 		Is_Client();
	public static native("Is_Local_Server") 				bool 		Is_Local_Server();
	public static native("Is_Dedicated_Server") 			bool 		Is_Dedicated_Server();
	public static native("Get_User_Count") 					int		 	Get_User_Count();
	public static native("Active_Team_Count") 				int		 	Active_Team_Count();
	public static native("Get_User") 						NetUser 	Get_User(int net_id);
	public static native("Get_Users") 						NetUser[] 	Get_Users();
	public static native("Get_Local_Users") 				NetUser[] 	Get_Local_Users();
	public static native("Get_Primary_Local_User") 			NetUser		Get_Primary_Local_User();
	public static native("Get_Local_User_By_Index")			NetUser		Get_Local_User_By_Index(int index);
	public static native("Accept_Pending_Profile_Changes")	void		Accept_Pending_Profile_Changes();
	public static native("Is_Visible_To_Users")				bool		Is_Visible_To_Users(Vec4 b);

    public static native("Reset_Local_Idle_Timer")          void        Reset_Local_Idle_Timer();

	public static native("Send_Chat")						void		Send_Chat(ChatType msg_type, string message, int to_net_id = -1, int from_id = -1);
}
