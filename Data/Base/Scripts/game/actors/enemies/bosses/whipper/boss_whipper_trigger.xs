// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.components.collision;
using system.actors.actor;
using system.scene;
using system.time;

[
    Placeable(true),
    Name("Whipper Boss Trigger"),
    Description("Starts the whipper boss battle.")
]
public class Whipper_Boss_Trigger : Trigger
{
    serialized(2000) bool bBossActive = false;

    bool bAudioPushed = false;

    Whipper_Boss_Trigger()
    {
        Maximum_Triggers = 1;
        Require_All_Players = true;
    }
     
    override void Base_Update()
    {
        if (bBossActive)
        {
            if (!bAudioPushed)
            {
                Audio.Push_BGM("music_bossu");
                bAudioPushed = true;
            }
        }
        else
        {
            if (bAudioPushed)
            {
                Audio.Pop_BGM();
                bAudioPushed = false;
            }
        }
    }

    override void Trigger_Logic()
    {
        if (Network.Is_Server())
        {
            Log.Write("Whipper boss has been triggered!");

            // Find the initial emergence marker.
            Actor[] markers = Scene.Find_Actors(typeof(Boss_Whipper_Emerge_Marker));
            Boss_Whipper_Emerge_Marker initial_emerge_marker = null;
            foreach (Boss_Whipper_Emerge_Marker marker in markers)
            {
                if (marker.Is_Initial)
                {
                    initial_emerge_marker = marker;
                    break;
                }
            }

            if (initial_emerge_marker == null)
            {
                Log.Write("[ERROR] Whipper boss was triggered, but no initial spawn point!?");
                return;
            }

            // Change audio.
            bBossActive = true;

            // Lock doors.
            Actor[] doors = Scene.Find_Actors(typeof(Boss_Whipper_Door));            
            foreach (Boss_Whipper_Door door in doors)
            {
                door.Is_Open = false;
            }

            // Despawn other enemies and disable enemy spawning.            
            Enemy_Manager.Spawning_Enabled = false;
            
			Actor[] actors = Scene.Find_Actors(typeof(Enemy));
			foreach (Enemy p in actors)
			{
				p.Magic_Despawn();
            }
            
            // Spawn the boss!
            Enemy_Manager.Spawn_Enemy(
                Enemy_Type.Boss_Whipper, 
                initial_emerge_marker.World_Bounding_Box, 
                initial_emerge_marker.Layer, 
                Enemy_Manager.Default_Team_Index
            );

            // Wait for boss to despawn.
            while (true)
            {
                Actor[] bosses = Scene.Find_Actors(typeof(Boss_Whipper));            
                if (bosses.Length() <= 0)
                {
                    break;
                }
                Time.Sleep(0.0f);
            }

            // Re-enable enemy spawning.
            Enemy_Manager.Spawning_Enabled = true;

            // Unlock doors.        
            foreach (Boss_Whipper_Door door in doors)
            {
                door.Is_Open = !door.Is_Entrance;
            }

            // Disable boss.
            bBossActive = false;
        }
    }
}