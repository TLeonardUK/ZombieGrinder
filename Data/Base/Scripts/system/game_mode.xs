// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.team;
using system.components.sprite;
using system.actors.actor;
using system.actors.camera;
using system.hud;
using system.network;

[
	Placeable(false), 
	Name("Game_Mode"), 
	Description("Base class for all game-modes. There should always be one game-mode per map to determine how the map should function.") 
]
public native("Game_Mode") class Game_Mode : Actor
{
	// General behaviour properties.
	public abstract bool Is_PVP();
	public abstract Team[] Get_Teams();
	
	// Scoreboard properties.
	public abstract string[] Get_Scoreboard_Columns();	
	public abstract string Get_Scoreboard_Title();
	public abstract string Get_Scoreboard_Value(NetUser user, int column);
    public abstract bool Get_Scoreboard_Sort_Value(NetUser user);
	public abstract int Get_Scoreboard_Is_Dead(NetUser user);
	public abstract int Get_Scoreboard_Team_Score(int team_index);
    public native("Get_Scoreboard_Rank") int Get_Scoreboard_Rank(NetUser user);

	// Camera manipulation.
	public abstract Camera Create_Camera(int user_index);
	public native("Get_Camera") Camera Get_Camera(int index);
	public native("Get_Camera_Count") int Get_Camera_Count();
	
	// Hud manipulation.
	public abstract HUD Create_HUD();	
	public native("Get_HUD") HUD Get_HUD();

	// Game over state.
	public native("Reset_Game_Over") void Reset_Game_Over();
	public native("Add_Game_Over_Bonus") void Add_Game_Over_Bonus(string name, int value);
	public native("Show_Game_Over") void Show_Game_Over(string title = "", string destination = "");
	public native("Get_Game_Over_Score") int Get_Game_Over_Score();

	// Persistent state.
	public virtual object Persist_State()
	{
		Log.Write("Base persist state called. Nothing will be persisted.");
	}
	public virtual void Restore_State(object s)
	{
	}

	Game_Mode()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
	}
	 
	// Enforce network replication of game modes.
	replicate(ReplicationPriority.Normal)
	{
	}
	
	// Sprite to show in the editor.
	components
	{
		SpriteComponent sprite
		{
			Frame 			= "actor_game_mode_default_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}
