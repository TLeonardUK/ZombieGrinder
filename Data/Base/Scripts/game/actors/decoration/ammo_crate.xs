// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using system.audio;
using game.actors.interactable.interactable;

[
	Placeable(true), 
	Name("Ammo Crate"), 
	Description("Allows the user to reload their weapons for a price!") 
]
public class Ammo_Crate : Actor, Interactable
{
	[ 
		Name("Unlimited Uses"),
		Description("If true this ammo crate can be used an unlimited amount of times.")
	]
	public serialized(1) bool Unlimited_Uses = false;
	
	[ 
		Name("Uses Remaining"),
		Description("Number of times this ammo crate can be used.")
	]
	public serialized(1) int Uses_Remaining = 8;
	
	[ 
		Name("Maximum Uses"),
		Description("Maximum uses, overridden when game starts with Uses_Remaining. Used to calculate correct 'fullness' animation of ammo crate.")
	]
	private serialized(1) int Max_Uses = 8;

	Ammo_Crate()
	{
		Bounding_Box = Vec4(0, 0, 48, 32);	
	}

	default state Idle
	{
		event void On_Enter()
		{		
			Max_Uses = Uses_Remaining;
					
			Depth_Bias = 16;

			if (Uses_Remaining <= 1 && !Unlimited_Uses)
			{
				sprite.Frame = "actor_ammo_crate_default_11";
			}
		}
	}
	
	public override bool Can_Interact(Pawn other)
	{
		return (Uses_Remaining > 0 || Unlimited_Uses == true);
	}

	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#fill_ammo_prompt");
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 1000;
	}

	public override float Get_Duration(Pawn other)
	{
		return 250.0f;
	}

	public override void Interact(Pawn other)
	{
		// Soooo, this should be server sided, but it feels a lot nicer
		// for the local client to be client-sided. Possibility is multiple
		// users using it at the same time and using it more than "Uses_Remaining".
		// But meh, who cares its ammo.		
		if (Uses_Remaining > 0 || Unlimited_Uses == true)
		{
		//	other.Fill_Reserve_Ammo();
			other.Fill_Active_Ammo();
		}

		rpc(RPCTarget.All, RPCTarget.None) RPC_Trigger();
	}
	
	private rpc void RPC_Trigger()
	{
		float fraction = <float>Uses_Remaining / <float>Max_Uses;
		int amount = 3 - Math.Clamp(<int>(fraction / 0.25f), 0, 3);

		if (Uses_Remaining <= 1 && !Unlimited_Uses)
		{
			sprite.Frame = "actor_ammo_crate_default_11";
			sprite.Play_Oneshot_Animation("actor_ammo_crate_default_open_empty", 1.0f);
		}
		else
		{
			sprite.Play_Oneshot_Animation("actor_ammo_crate_default_open" + (amount + 1), 1.0f);
		}

		if (Unlimited_Uses == false)
		{
			Uses_Remaining--;
		}

		Audio.Play3D("sfx_objects_pickups_ammobox_open", Position);
	}

	/*
	private rpc void RPC_Request_Trigger()
	{
		if (Network.Is_Server())
		{
			if (Uses_Remaining > 0 || Unlimited_Uses == true)
			{
				if (Unlimited_Uses)
				{
					Uses_Remaining--;
				}
				rpc(RPCTarget.All, RPCTarget.None) RPC_Trigger();
			}
		}
	}
	
	private rpc void RPC_Trigger()
	{
		Log.Write("Ammo crate used, uses '%i'.");	

		Audio.Play3D(Audio_Name, Position);
	}
	*/

	// Force replication as network object.
	replicate(ReplicationPriority.Low)
	{
	}
	
	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_ammo_crate_default_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = 16;
			Object_Mask = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}