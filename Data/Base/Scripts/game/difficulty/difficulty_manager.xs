// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using system.network;

[
	Placeable(false), 
	Name("Difficulty_Manager"), 
	Description("Singleton manager used for controlling global difficulty of the game.") 
]
public class Difficulty_Manager
{
	// How much to increase difficulty by active user.
	[ Inspectable(true) ]
	static float Difficulty_Multiplier_Per_User = 0.3f;
	static float Spawn_Count_Multiplier_Per_User = 0.3f;

	// Multiplier of exponential difficulty curve.
	[ Inspectable(true) ]
	static float Difficulty_Curve_Coefficient_A = 1.0f;

	// Base difficulty multiplier for exponential difficulty curve. 
	[ Inspectable(true) ]
	static float Difficulty_Curve_Coefficient_B = 0.05f;

	// Difficulty Curve multipliers for each multiplier.
	[ Inspectable(true) ]
	static float Difficulty_Speed_Curve				= 0.100f;
	[ Inspectable(true) ]
	static float Difficulty_Health_Curve			= 0.25f;
	[ Inspectable(true) ]
	static float Difficulty_Damage_Curve			= 0.55f;
	[ Inspectable(true) ]
	static float Difficulty_Spawn_Count_Curve		= 0.5f;
	[ Inspectable(true) ]
	static float Difficulty_Spawn_Interval_Curve	= 0.5f;
	
	// Base difficulty multiplier without being molested. Probably shouldn't use this if you don't know what it is!
	[ Inspectable(true) ]
	public static float Base_Difficulty = 1.0f;
	
	// Base difficulty after being adjusted for player counts / difficulty curve / etc.
	[ Inspectable(true) ]
	public static float Difficulty = 1.0f;

	// Multiplier for enemy speeds.
	[ Inspectable(true) ]
	public static float Enemy_Speed_Multiplier = 1.0f;
	
	// Multiplier for enemy health.
	[ Inspectable(true) ]
	public static float Enemy_Health_Multiplier = 1.0f;
	
	// Multiplier for enemy damage counts.
	[ Inspectable(true) ]
	public static float Enemy_Damage_Multiplier = 1.0f;
	
	// Multiplier for enemy spawn counts.
	[ Inspectable(true) ]
	public static float Enemy_Spawn_Count_Multiplier = 1.0f;

	// Multiplier for enemy intervals between spawns.
	[ Inspectable(true) ]
	public static float Enemy_Spawn_Interval_Multiplier = 1.0f;

	// Multiplier for refactory periods in different game modes.
	[ Inspectable(true) ]
	public static float Refactory_Time_Multiplier = 1.0f;

    // Fear based spawning manipulation!
    public const float Fear_Target = 100.0f;
    public const float Fear_Difficulty_Lerp = 0.08f;
    public const float Fear_Max_Difficulty_Increment = 2.0f;

    public static float Fear_Difficulty_Increment = 0.0f;

    private static float Last_Set_Difficulty = 0.0f;

    public static bool Use_Fear_Difficulty = false;

	// -------------------------------------------------------------------------------------------------
	// Takes a scalar value from 0-inf and uses that as a multiplier
	// for difficulty.
	// -------------------------------------------------------------------------------------------------
	public static void Set_Difficulty(float scalar)
	{
        Last_Set_Difficulty = scalar;

        Update_Difficulty(scalar);

		Log.Write("[Difficulty] Base_Difficulty = " + Base_Difficulty);
		Log.Write("[Difficulty] Difficulty = " + Difficulty);
		Log.Write("[Difficulty] Enemy_Speed_Multiplier = " + Enemy_Speed_Multiplier);
		Log.Write("[Difficulty] Enemy_Health_Multiplier = " + Enemy_Health_Multiplier);
		Log.Write("[Difficulty] Enemy_Damage_Multiplier = " + Enemy_Damage_Multiplier);
		Log.Write("[Difficulty] Enemy_Spawn_Count_Multiplier = " + Enemy_Spawn_Count_Multiplier);
		Log.Write("[Difficulty] Enemy_Spawn_Interval_Multiplier = " + Enemy_Spawn_Interval_Multiplier);
		Log.Write("[Difficulty] Refactory_Time_Multiplier = " + Refactory_Time_Multiplier);
	}

    public static void Reset()
    {
        Use_Fear_Difficulty = false;
        Fear_Difficulty_Increment = 0.0f;
        Set_Difficulty(1.0f);
    }

    private static void Update_Difficulty(float scalar)
    {
		Base_Difficulty = scalar;
	
		// Calculate exponential difficulty curve.
		Difficulty = Difficulty_Curve(scalar);

		// Bias difficulty by player count.		
		int user_count = Network.Get_User_Count();
		float spawn_count_multiplier_addition = 0.0f;
        if (user_count > 1)
		{
			Difficulty *= 1.0f + (Difficulty_Multiplier_Per_User * (user_count - 1));
			spawn_count_multiplier_addition = (Spawn_Count_Multiplier_Per_User * (user_count - 1));
		}

        // Boost spawning, speed and health based on average fear.
        Actor[] players = Scene.Find_Actors(typeof(Human_Player));
        float avg_fear = 0.0f;
        foreach (Human_Player player in players)
        {
            avg_fear += player.Fear;
        }
        avg_fear /= Math.Max(1, players.Length());

        float delta_t = Time.Get_Delta_Seconds();

        float fear_distance_to_target = Math.Max(0.0f, Fear_Target - avg_fear);
        float target_fear_increment = (fear_distance_to_target / Fear_Target) * Fear_Max_Difficulty_Increment;
        
        if (Use_Fear_Difficulty)
        {
            Fear_Difficulty_Increment = Math.Lerp(Fear_Difficulty_Increment, target_fear_increment, Fear_Difficulty_Lerp * delta_t);
        }
        
        Log.WriteOnScreen("FearDifficulty", Vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, "Target="+target_fear_increment+" Inc=" + Fear_Difficulty_Increment);

		// Calculate multipliers.
		float inverse_difficulty = 1.0f / (Difficulty + Fear_Difficulty_Increment);
		float difficulty_increment = (Difficulty + Fear_Difficulty_Increment) - 1.0f;

		Enemy_Speed_Multiplier				= 1.0f + (difficulty_increment * Difficulty_Speed_Curve);
		Enemy_Health_Multiplier				= 1.0f + (difficulty_increment * Difficulty_Health_Curve);
		Enemy_Damage_Multiplier				= 1.0f + (difficulty_increment * Difficulty_Damage_Curve);
		Enemy_Spawn_Count_Multiplier		= 1.0f + (difficulty_increment * Difficulty_Spawn_Count_Curve) + spawn_count_multiplier_addition;
		Enemy_Spawn_Interval_Multiplier		= 1.0f / (1.0f + (difficulty_increment * Difficulty_Spawn_Interval_Curve) + spawn_count_multiplier_addition);
		Refactory_Time_Multiplier			= inverse_difficulty;
    }

    public static float Tick()
    {
        Update_Difficulty(Last_Set_Difficulty);
    }

	// -------------------------------------------------------------------------------------------------
	// Determines difficulty curve for a given round.
	// -------------------------------------------------------------------------------------------------
	private static float Difficulty_Curve(float round)
	{
		return Math.Max(1.0f, Difficulty_Curve_Coefficient_A * Math.Exp(Difficulty_Curve_Coefficient_B * round));	
	}
}
