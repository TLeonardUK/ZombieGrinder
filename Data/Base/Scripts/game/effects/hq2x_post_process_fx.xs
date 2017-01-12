// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.post_process_fx;
using system.time;
using system.audio;
using system.network;
using game.actors.player.human_player;
using system.options;
 
// TODO:
// Blah this shader currently sucks a significant amount of ass. 
// Needs some seriously work.

public class HQ2X_Post_Process_FX : Post_Process_FX
{	
	HQ2X_Post_Process_FX()
	{	
		Shader = "hq2x"; 
	}

	public override void On_Shader_Setup()
	{
	}	

	event void On_Tick()
	{
		Active = Options.Get_Bool("hq2x_enabled");
	}
}

/*
public class HQ2X_Post_Process_Resize_FX : Post_Process_FX
{	
	HQ2X_Post_Process_Resize_FX()
	{	
		Shader = "hq2x_resize"; 
	}

	public override void On_Shader_Setup()
	{
	}	

	event void On_Tick()
	{
		Active = Options.Get_Bool("hq2x_enabled");
	}
}
*/