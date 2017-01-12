// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.cameras.game_camera;
using system.network;
using system.display;
using system.time;
using runtime.math;
 
[
	Placeable(false), 
	Name("Zoom Fit Camera"), 
	Description("Camera that zooms int and out to fit around the target users.") 
]
public class Zoom_Fit_Camera : Game_Camera
{
	private Vec4 m_viewport;
	private Vec4 m_target_viewport;
	private int m_spectate_offset;
	private float m_freecam_acceleration;
	private bool m_was_freecam;
	private Actor m_spectate_target;

	private bool m_has_tracked;

	const float Freecam_Acceleration_Max 	 = 20.0;
	const float Freecam_Acceleration_Lerp  	 = 0.6;

	const float Viewport_Lerp_Factor		    = 0.8;	
	const float Viewport_Slow_Lerp_Factor		= 0.2;		

	private int m_target_user_index = -1;

	private bool m_spectating = false;

	private bool m_first_frame = false;

    private Vec4 m_last_non_max_viewport;

	private bool m_shake_accepted_this_frame = false;

	private static float m_last_cycle_time = 0; 

    private bool m_lock_everyone = false;

	Zoom_Fit_Camera()
	{
		m_spectate_offset = 0.0; 
		m_freecam_acceleration = 0.0;
		m_target_user_index = -1;
	}

	Zoom_Fit_Camera(int user_index)
	{
		m_spectate_offset = 0.0; 
		m_freecam_acceleration = 0.0;
		m_target_user_index = user_index;
	}

	Zoom_Fit_Camera(int user_index, bool bLockEveryone)
	{
		m_spectate_offset = 0.0; 
		m_freecam_acceleration = 0.0;
		m_target_user_index = user_index;
        m_lock_everyone = bLockEveryone;
	}

	public void Center_On_Start()
	{		
		Actor[] player_spawns = Scene.Find_Actors(typeof(Player_Spawn));
		if (player_spawns.Length() > 0)
		{
			Actor[] framing = player_spawns[:1];	
            Player_Spawn first_spawn = <Player_Spawn>framing[0];

            // If spawning by intro, using intro actor.
            if (first_spawn && first_spawn.Intro_Point_Link != "")
            {            
				Actor[] points = Scene.Find_Actors_By_Tag(first_spawn.Intro_Point_Link);
				if (points.Length() > 0)
				{
					framing = points;
				}
            }

			m_target_viewport = Calculate_Ideal_Viewport(framing);	
			m_viewport = m_target_viewport;		

			Log.Write("Freecam: Snap to first player start");
		}
		else
		{
			Log.Write("Freecam: Found no player starts to snap to.");					
		}
	}
	
	public override Vec4 Get_Viewport()
	{
		if (m_first_frame == false) 
		{
			Center_On_Start();
			m_first_frame = true;
		}

		return m_viewport;
	}
	
	public override Actor Get_Spectate_Target()
	{
		if (m_spectate_target != null && Scene.Is_Spawned(m_spectate_target) == false)
		{
			m_spectate_target = null;
		}
		return m_spectate_target;
	}
	
	public override bool Is_Spectating()
	{
		return m_spectating;
	}

	private void Set_Freecam()
	{
		Actor[] potential_users = Scene.Get_Controlled_Pawns();
		m_spectate_offset = potential_users.Length();
	}
	
	// Particle shake input.
	public event void On_Tiny_Particle_Shake(Event_Particle particle)
	{
		if (m_shake_accepted_this_frame == true)
		{
			return;
		}
		m_shake_accepted_this_frame = true;

		Log.Write("[Camera] On_Tiny_Particle_Shake was invoked.");
		Shake(0.5f, 3.0f, particle.Position, null, 0.0f);
	}
	public event void On_Small_Particle_Shake(Event_Particle particle)
	{
		if (m_shake_accepted_this_frame == true)
		{
			return;
		}
		m_shake_accepted_this_frame = true;

		Log.Write("[Camera] On_Small_Particle_Shake was invoked.");
		Shake(1.0f, 4.0f, particle.Position, null, 0.0f);
	}
	public event void On_Medium_Particle_Shake(Event_Particle particle)
	{
		if (m_shake_accepted_this_frame == true)
		{
			return;
		}
		m_shake_accepted_this_frame = true;

		Log.Write("[Camera] On_Medium_Particle_Shake was invoked.");
		Shake(1.5f, 8.0f, particle.Position, null, 0.0f);
	}
	public event void On_Large_Particle_Shake(Event_Particle particle)
	{
		Log.Write("[Camera] On_Large_Particle_Shake was invoked.");
		Shake(3.0f, 10.0f, particle.Position, null, 0.0f);
	}

	default state Tracking_Local
	{
		event void On_Enter()
		{
			m_spectate_target = null;
		}
		event void On_Tick()
		{
			// Find local users to frame around.
			Actor[] framing;
			if (m_target_user_index != -1)
			{				
				framing = Scene.Get_Local_User_Controlled_Pawns(m_target_user_index);
			}
			else
			{
                bool bLocalNonHumans = false;
                Actor[] local_pawns = Scene.Get_Controlled_Pawns(); 
                Actor[] human_pawns = new Actor[0];
                foreach (Actor actor in local_pawns)
                {
                    Human_Player player = <Human_Player>actor;
                    if (player == null)
                    {
                        if (actor.Owner && actor.Owner.Is_Local)
                        {
                            bLocalNonHumans = true;
                        }
                    }
                    else
                    {
                        human_pawns.AddLast(actor);
                    }
                }

                // We normally frame all users. However if we are controlling a zombie etc, we focus on just ourselves.
                // Only non hot-local-coop can spawn as non-humans, so that shouldn't be an issue.
                if (m_lock_everyone && !bLocalNonHumans)
                {
                    framing = human_pawns;
                }
                else
                {
                    framing = Scene.Get_Local_Controlled_Pawns();                
                }
            }
			 
            float lerp_factor = Viewport_Lerp_Factor;

			Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
            if (mode.bBossIntroActive)
            {
                framing = new Actor[1];
                framing[0] = mode.BossIntroActor;
                lerp_factor = Viewport_Slow_Lerp_Factor;
            }

			// No local users? Change to spectating.
			if (framing.Length() == 0)
			{
				Log.Write("Changing to spectating state.");
				Set_Freecam();
				change_state(Spectating);
				return;
			}
			
			// Frame around local users.
			float delta = Time.Get_Delta();
			m_target_viewport = Calculate_Ideal_Viewport(framing);

            // If viewport is maxed out, don't move it, just change size.
            Vec4 constrain_viewport = m_target_viewport;
            if (m_viewport_max == true)
            {
                constrain_viewport = m_last_non_max_viewport;
               // Log.WriteOnScreen("ViewportMaxed", Vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, "ViewportMax="+m_target_viewport.X+","+m_target_viewport.Y+","+m_target_viewport.Z+","+m_target_viewport.W);
                /*m_target_viewport = Vec4(
                    m_viewport.Center().X - (m_target_viewport.Z * 0.5f),
                    m_viewport.Center().Y - (m_target_viewport.W * 0.5f),
                    m_target_viewport.Z,
                    m_target_viewport.W
                );*/
            }
            else 
            {
                m_last_non_max_viewport = m_target_viewport; 
               // Log.WriteOnScreen("ViewportNotMaxed", Vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, "VIEWPORT NOT MAXED="+m_target_viewport.X+","+m_target_viewport.Y+","+m_target_viewport.Z+","+m_target_viewport.W);               
            }

			m_viewport = Lerp_Viewport(m_viewport, m_target_viewport, lerp_factor * delta);
			m_has_tracked = true;
			m_shake_accepted_this_frame = false;

			// Constrain locals pawns to viewport.
			if (m_viewport_max == true && framing.Length() > 1)
			{
				foreach (Pawn p in framing)
				{
                    if (p.Owner != null && p.Owner.Is_Local)
                    {
    					p.LimitMovement(constrain_viewport);
                    }
				}
			}

			// Update position.
			Audio.SetListenerPosition(m_target_user_index, Vec3(m_target_viewport.Center(), 0.0f));

			// Shake dat viewport.
			m_viewport = Apply_Shake(m_viewport);
		}
	}
	
	state Spectating
	{
		event void On_Enter()
		{
			m_was_freecam = false;
			m_spectating = true;
			m_spectate_target = null;
		}
		event void On_Exit()
		{
			m_spectating = false;
			m_spectate_target = null;
		}
		event void On_Tick()
		{
			// Any local users to frame?
			Actor[] local_potential_users;
			if (m_target_user_index != -1)
			{				
				local_potential_users = Scene.Get_Local_User_Controlled_Pawns(m_target_user_index);
			}
			else
			{
				local_potential_users = Scene.Get_Local_Controlled_Pawns();
			}

			if (local_potential_users.Length() != 0)
			{
				Log.Write("Changing to tracking state.");
				change_state(Tracking_Local);
				return;
			}
			
			// Find spectatable user to frame.
			NetUser[] users = Network.Get_Users();
			Actor[] potential_users = Scene.Get_Controlled_Pawns();
			
			// Cycle through users.
			NetUser master_user = null;
			if (m_target_user_index != -1)
			{
				master_user = Network.Get_Local_User_By_Index(m_target_user_index);
			}
			else
			{
				master_user = Network.Get_Primary_Local_User();
			}
			
			bool has_cycled = false;
			
			if (Input.Was_Pressed(master_user, OutputBindings.Fire) && m_last_cycle_time > 0.5f)
			{
				m_spectate_offset++;
				m_last_cycle_time = 0;
				has_cycled = true;
			}
                 
			m_last_cycle_time += Time.Get_Delta_Seconds();			
			float delta = Time.Get_Delta();
			
			// Just linger on current viewport if no spectators available.			
			int offset = m_spectate_offset % (potential_users.Length() + 1);
			
			if (has_cycled)
			{
				Log.Write("Cycling to next spectatable user, offset=" + offset + "/" + potential_users.Length() + ".");				
			}
			 
			// Free-Cam
			if (potential_users.Length() == 0 || offset >= potential_users.Length())
			{
				Vec2 input_delta = Vec2(0, 0);
				if (Input.Is_Down(master_user, OutputBindings.Left))		input_delta.X = -1.0;
				if (Input.Is_Down(master_user, OutputBindings.Right))		input_delta.X =  1.0;
				if (Input.Is_Down(master_user, OutputBindings.Up))			input_delta.Y = -1.0;
				if (Input.Is_Down(master_user, OutputBindings.Down))		input_delta.Y =  1.0;
				
				float target_acceleration = 0.0;
				
				if (input_delta.X != 0 || input_delta.Y != 0)
				{
					target_acceleration = Freecam_Acceleration_Max;	
				}
				
				m_freecam_acceleration = Math.Lerp(m_freecam_acceleration, target_acceleration, Freecam_Acceleration_Lerp * delta);			
				m_target_viewport.XY = m_target_viewport.XY + ((input_delta * m_freecam_acceleration) * delta);
				
				m_was_freecam = true;
				m_spectate_target = null;
			} 
			
			// Spectate-Cam
			else
			{			
				Actor[] framing = potential_users[offset:offset + 1];			
				m_target_viewport = Calculate_Ideal_Viewport(framing);	
				
				m_was_freecam = false;
				m_spectate_target = framing[0];
			}			

            // Force camera for bosses etc.
            float lerp_factor = Viewport_Lerp_Factor;

            Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
            if (mode.bBossIntroActive)
            {
                Actor[] framing = new Actor[1];
                framing[0] = mode.BossIntroActor;	
				m_target_viewport = Calculate_Ideal_Viewport(framing);	
                lerp_factor = Viewport_Slow_Lerp_Factor;
            }

			// Cap viewport to bounds.
			m_target_viewport = m_target_viewport.Constrain(Scene.Get_Bounds());
			
			// Frame around current spectating actor.
			m_viewport = Lerp_Viewport(m_viewport, m_target_viewport, lerp_factor * delta);
			
			// Update position.
			Audio.SetListenerPosition(m_target_user_index, Vec3(m_target_viewport.Center(), 0.0f));

			// Shake dat viewport.
			m_viewport = Apply_Shake(m_viewport);
			m_shake_accepted_this_frame = false;
		}
	}
}
