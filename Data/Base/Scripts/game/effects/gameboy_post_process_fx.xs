// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.post_process_fx;
using system.time;
using system.audio;
using system.network;

public class Gameboy_Post_Process_FX : Post_Process_FX
{	
	Gameboy_Post_Process_FX()
	{	
		Shader = "gameboy";
	}

	public override void On_Shader_Setup()
	{
	}
	
	event void On_Tick()
	{				
		Active = false;
	
		Type self_type = typeof(Gameboy_Post_Process_FX);
	
		NetUser[] players = Network.Get_Local_Users();
		foreach (NetUser user in players)
		{
			for (int i = 0; i < Item_Slot.COUNT && Active == false; i++)
			{
				Item itm = user.Profile.Get_Item_Slot(i);
				if (itm != null && itm.Archetype.Post_Process_FX_Type == self_type)
				{
					Active = true;
				}
			}
			
			if (Active)
			{
				break;
			}
		}
	}
}
