// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.effect;

[
	Placeable(false), 
	Name("FX"), 
	Description("Global class, responsible for spawning and managing effects - particles etc.") 
]
public native("FX") class FX
{
    // STAT_TODO

	// Should be used in preference to Spawn, directly spawns an effect, without the need
	// to go through scene spawning a new actor, faster and less memory usage.
	public static native("Spawn_Untracked") void Spawn_Untracked(string effect_name, Vec3 pos, float dir, int layer, Actor instigator = null, int meta_number = 0, string modifier = "", int subtype = 0, Item_Archetype weapon_type = null, float[] upgrade_modifiers = null);

	public static Effect Spawn(string effect_name, Vec3 pos, float dir, int layer, Actor instigator, int meta_number = 0, string modifier = "", int subtype = 0, Item_Archetype weapon_type = null, float[] upgrade_modifiers = null)
	{
		Effect f = <Effect>Scene.Spawn(typeof(Effect), null);
		f.Get_Component().Effect_Name = effect_name;
		f.Get_Component().Instigator = instigator;
		f.Get_Component().Meta_Number = meta_number;
		f.Get_Component().Modifier = modifier;
		f.Get_Component().SubType = subtype;
		f.Get_Component().Weapon_Type = weapon_type;
		f.Get_Component().Upgrade_Modifiers = upgrade_modifiers;
		f.Position = pos;
		f.Rotation = dir;
		f.Layer = layer;
		f.Parent = instigator;
		return f;
	}	

	// For all particles from the given instigator a GameTrigger event will be executed.
	public static native("Invoke_Game_Trigger") void Invoke_Game_Trigger(Actor instigator);
}
