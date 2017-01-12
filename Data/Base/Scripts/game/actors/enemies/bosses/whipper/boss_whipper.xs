// storyboard:

//		all players enter area, spikes raise behind user.
//		cutscene? Spike monster rises out of ground - lots of dust vfx.
//		tentacles raise and slam on floor. Players nearby are hurt. Shake camera etc.
//		damage to tentacles is passed along the full tentacle 

//		periodically moves location

//		starts with one tentacle
//		after 1/4 health taken
//		dive under ground
//		reappear in another place with 2 tentacles
//		after 1/4 health taken
//		dive under ground
//		reappear in another place with 3 tentacles, moves faster
//		after 1/4 health taken
//		dive under ground
//		reappear in another place with 4 tentacles, moves even faster
//		
//		enemy dies and falls to ground.
//		tentacle falls on objects guarding exiting. Player can leave.

//		each tentacle sphere blows up in sequence.

//		damage is done to base not tentacles.	

//		lots of score
//		potentially vfx for "boss killed!"

// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.enemies.enemy;
using system.network;
using system.components.sprite;
using system.components.collision;
using system.components.boid;
using system.actors.actor;
using system.time;
using system.fx;
using system.actors.pawn;
using system.events;
using game.profile.profile;
using game.weapons.enemies.weapon_bouncer_zombie;
using game.difficulty.difficulty_manager;
 
public enum Whipper_State
{
    Initial_Emerge = 0,
    Retreating = 1,
    Emerging = 2,
    Idle = 3,
    Death = 4,
    Crunch = 5
}

[
	Placeable(false), 
	Name("Whipper Boss"), 
	Description("Blobby boss with super tentacle whip!") 
]
public class Boss_Whipper : Boss
{  
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
    const float DEFENCELESS_HEALTH_THRESHOLD_REGENERATED = 250.0f;
    const float DEFENCELESS_HEALTH_THRESHOLD = 550.0f;

	const float START_HEALTH				= 1200.0f;
    const float PER_USER_HEALTH_BOOST       = 600.0f;
	const float HIT_OFFSET_FX_AMOUNT	    = 6.0f;

	const string BODY_IDLE_ANIMATION		= "boss_whipper_body_idle"; 
	const string BODY_HURT_ANIMATION		= "boss_whipper_body_hurt";
	const string BODY_BLINK_ANIMATION		= "boss_whipper_body_blink";
	const string BODY_SHOCK_ANIMATION		= "boss_whipper_body_shock";

    private bool bHasGivenAilmentScore 		= false;
	const int AILMENT_HIT_SCORE_INCREASE 	= 1000;
	const int HIT_SCORE_INCREASE			= 1; 

	const float HURT_EVENT_RADIUS = 32.0f;

    Boss_Whipper_Tentacle[] m_tentacles = new Boss_Whipper_Tentacle[0];

    const int BASE_TENTACLE_COUNT = 4;
    const int REGENERATE_TENTACLE_COUNT = 10;

    public serialized(1) int m_active_tentacle_count = 5; // Start with one.
    public serialized(1) int[] DeadSegments = new int[0]; 
	         
    public float Angle = 0.0f;

    public float HitTimer = 0.0f;
     
    public bool bRaised = false;

    const float IDLE_SPIN_SPEED = 3.5f;
    const float RAISED_SPIN_SPEED = 1.5f;

    const float TIME_TO_SLAM_MIN = 3.0f;
    const float TIME_TO_SLAM_MAX = 3.0f;
    const float TIME_TO_RAISE_MIN = 3.0f;
    const float TIME_TO_RAISE_MAX = 6.0f;
    public float Time_To_Hit = 0.0f;

    const float MOVEMENT_PROBABILITY = 0.2f;

    float Tentacle_Retract_Factor = 1.0f;
    const float TENTACLE_RETRACT_SPEED = 1.3f;

    const float RETREAT_SPIN_SPEED = 6.0f;
    const float SPEED_ACCELERATION = 0.8f;
    float Current_Speed = 0.0f;
    bool bSpeedIsInverted = false;
    public float TimeToSpeedInvert = 0.0f;
    
    const float TIME_TO_SPEED_INVERT_MIN = 16.0f;
    const float TIME_TO_SPEED_INVERT_MAX = 32.0f;

    const float MAX_SLAM_DELAY = 5.0f;

    const float ATTACK_DISTANCE = 120.0f;

    const float PANIC_SPEED = 19.0f;
    const float CHASE_SPEED = 18.0f;
    const float IDLE_SPEED = 6.0f;
    
    public bool bRetreatPending = false;
    
    const float REGENERATE_DEFENCELESS_INTERVAL = 5.0f;
    const int DEFENCELESS_THRESHOLD = 11;
    public float Defenceless_Time = 0.0f;

    const float SEGMENT_DAMAGE_MULTIPLIER = 2.0f;

    public int bRegeneratedCount = 0;
    public bool bRegeneratePending = false;

    public bool bPanic = false;
    public float m_panic_timer = 0.0f;
    public Vec3 m_panic_vector;
    
    const float PANIC_SWITCH_VECTOR_MIN = 0.5f;    
    const float PANIC_SWITCH_VECTOR_MAX = 1.5f;
    
    const float MAX_SEGMENT_DAMAGE_PER_INSTANCE = 15.0f;
    const float MAX_SEGMENT_DAMAGE_PER_FRAME = 60.0f;
    public float m_segment_damage_this_frame = 0.0f;

    public bool Is_Dieing = false;
    public int Tentacle_Segment_Auto_Kill_Index = 7;

    private Direction m_direction = Direction.S;

    const float SHOCK_SPEED_MULTIPLIER = 0.4f;
    const float FREEZE_SPEED_MULTIPLIER = 0.4f;

    const int Kill_Score_Bonus = 250000;

    const float MAX_CRUNCH_INTERVAL = 5.0f;
    const float MIN_CRUNCH_INTERVAL = 15.0f;
    const float MIN_CRUNCH_SEGMENT_FRACTION = 0.5f;
    float Crunch_Attack_Timer = 0.0f;

    const float CRUNCH_RETREAT_MIN = 0.3f;
    const float TENTACLE_CRUNCH_RETRACT_SPEED = 0.7f;
    const float TENTACLE_CRUNCH_SLAM_SPEED = 1.0f;
    const float CRUNCH_TURN_SPEED = 2.0f;

    const float FAR_AWAY_DISTANCE = 170.0f;

    public serialized(1) Whipper_State State = Whipper_State.Initial_Emerge;

    public bool bUseHighDamage = false;

	// -------------------------------------------------------------------------
	//  Coonstructor.
	// -------------------------------------------------------------------------
	Boss_Whipper()
	{
		Bounding_Box			= Vec4(0, 0, 32, 32);
		Depth_Y_Offset			= collision.Area.Y + collision.Area.Height;

		if (Network.Is_Server())
		{
			// Setup base health.
	    	int user_count = Network.Get_User_Count();
			Setup_Health((START_HEALTH + (PER_USER_HEALTH_BOOST * (user_count - 1)))); // * Difficulty_Manager.Enemy_Health_Multiplier

			// Give fodder zombie a standard weapon.
			Give_Weapon(typeof(Weapon_Bouncer_Zombie));
			Set_Active_Weapon_Slot(Weapon_Slot.Melee);
		}
	}

	// -------------------------------------------------------------------------
	// Death work.
	// -------------------------------------------------------------------------
	override void On_Death(Actor instigator, Item_Archetype weapon_type)
	{
	}

	public override float Get_Damage_Multiplier()
	{	
		return Difficulty_Manager.Difficulty_Damage_Curve;
	}
	
	public override bool Can_Target()
	{
		return true;
	}
    
    public override string Get_Boss_Intro_Text()
    {
        return Locale.Get("#hud_boss_whipper_intro_text");
    }

	// -------------------------------------------------------------------------
	// Wait for spawn.
	// -------------------------------------------------------------------------
    void Update_Tentacles()
    {
        Log.WriteOnScreen("WhipperBossState", Vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f, "State="+this.State);

        // Spawn any tentacles we are missing.
        while (m_active_tentacle_count > m_tentacles.Length())
        {
            Boss_Whipper_Tentacle tentacle = <Boss_Whipper_Tentacle>Scene.Spawn(typeof(Boss_Whipper_Tentacle), null);
            tentacle.Parent = this;
            tentacle.Angle = m_tentacles.Length() * Math.PI2;
            tentacle.Parent_Boss = this;
            tentacle.Position = this.Center;
            tentacle.Layer = this.Layer;
            tentacle.Index = m_tentacles.Length();
            
            m_tentacles.AddLast(tentacle);
            Log.Write("[BOSS] Added new tentacle for a total of " + m_tentacles.Length());
        }

        int index = 0;
        foreach (Boss_Whipper_Tentacle tentacle in m_tentacles)
        {
            tentacle.Angle = ((Math.PI2 /  m_tentacles.Length()) * index);        
            index++;
        }
        
        body_hitbox.Enabled = Is_Visible();
    }

    void Slam_All_Tentacles()
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Slam_All_Tentacles();
    }
    
    rpc void RPC_Slam_All_Tentacles()
    {
        Audio.Play3D("sfx_objects_enemies_bosses_whipper_slam", this.Center);
        bRaised = false;
    }

    void Raise_All_Tentacles()
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Raise_All_Tentacles();
    }
    
    rpc void RPC_Raise_All_Tentacles()
    {
        Audio.Play3D("sfx_objects_enemies_bosses_whipper_raise", this.Center);
        bRaised = true;
    }

    public bool Is_Visible()
    {
        return body_sprite.Visible;
    }

    void Spin_At_Speed(float speed)
    {
        // Shocked slows us down.
		if (m_status_ailment == StatusAilmentType.Shocked)
        {
            speed *= SHOCK_SPEED_MULTIPLIER;
        }
		if (m_status_ailment == StatusAilmentType.Frozen)
        {
            speed *= FREEZE_SPEED_MULTIPLIER;
        }

        if (bSpeedIsInverted)
        {
            speed = -speed;
        }

        float delta_t = Time.Get_Delta_Seconds();
        Current_Speed = Math.Lerp(Current_Speed, speed, SPEED_ACCELERATION * delta_t);
        Angle += Current_Speed * delta_t;   

        TimeToSpeedInvert -= delta_t;
        if (TimeToSpeedInvert <= 0.0f)
        {
            bSpeedIsInverted = !bSpeedIsInverted;
            TimeToSpeedInvert = Math.Rand(TIME_TO_SPEED_INVERT_MIN, TIME_TO_SPEED_INVERT_MAX);
        }
    }

    void Kill_Segment(int index)
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Kill_Segment(index);
    }

    rpc void RPC_Kill_Segment(int index)
    {
        if (!DeadSegments.Contains(index))
        {
            Log.Write("[BOSS] Segment "+index+" was killed.");
            DeadSegments.AddLast(index);
        }
    }

    void Regenerate()
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Regenerate();
    }

    rpc void RPC_Regenerate()
    {
        DeadSegments.Clear();
        Log.Write("[BOSS] Regenerated dead segments.");
    }

    void Retreat()
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Retreat();
    }

    rpc void RPC_Retreat()
    {    
        bRetreatPending = false;
        change_state(Retreating);
        Log.Write("[BOSS] Retreating.");
    }
    
    void Begin_Dieing()
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Begin_Dieing();
    }

    rpc void RPC_Begin_Dieing()
    {    
        change_state(Death);
        Log.Write("[BOSS] Dieing.");
    }

	// -------------------------------------------------------------------------
	// Wait for spawn.
	// -------------------------------------------------------------------------
	default state Spawn
	{
		event void On_Tick()
		{
			// Setup pawn.
			Setup();

			// Walking time.
            if (Network.Is_Server())
            {
                change_state(Initial_Emerge);
            }
            else
            {       
                switch (State)
                {
                    case Whipper_State.Initial_Emerge:      { change_state(Initial_Emerge);   break; }
                    case Whipper_State.Retreating:          { change_state(Retreating);       break; }
                    case Whipper_State.Emerging:            { change_state(Emerging);         break; }
                    case Whipper_State.Idle:                { change_state(Idle);             break; }
                    case Whipper_State.Death:               { change_state(Death);            break; }
                    case Whipper_State.Crunch:              { change_state(Crunch_Attack);    break; }
                }
            }
		}	
	}
 
	// -------------------------------------------------------------------------
	// Initial spawn from ground.
	// -------------------------------------------------------------------------
	state Initial_Emerge
	{
		event void On_Enter()
		{
            State = Whipper_State.Initial_Emerge;
            FX.Spawn_Untracked("whipper_boss_dirt_puff_large", this.Center, 0.0f, Layer);
		}		
		event void On_Tick()
		{
            Tentacle_Retract_Factor = 0.0f;
            
            // Start introduction.
		    Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
            mode.Begin_Boss_Intro(this);
            Time.Sleep(1.0f);

            // Spawn and raise tentacles.
            Update_Tentacles();
            Raise_All_Tentacles();
            
            // Wait a second then make the sprites visible. Gives VFX time to spawn some cover.
            Time.Sleep(0.5f);
            shadow_sprite.Visible = true;
            body_sprite.Visible = true;

		    if (m_status_ailment_effect != null && Scene.Is_Spawned(m_status_ailment_effect))
		    {
                Tick_Ailment();
			    m_status_ailment_effect.Get_Component().Visible = true;
		    }
            
            Audio.Play3D("sfx_objects_enemies_bosses_whipper_emerge", this.Center);

            // Wait for a while for tentacles to extend.
            while (Tentacle_Retract_Factor < 1.0f)
            {
                float delta_t = Time.Get_Delta_Seconds();

                if (Network.Is_Server())
                {
                    Tentacle_Retract_Factor = Math.Min(1.0f, Tentacle_Retract_Factor + (TENTACLE_RETRACT_SPEED * delta_t));

                    // Keep spinning.        
                    Spin_At_Speed(RETREAT_SPIN_SPEED);
                }

                Update_Tentacles();
                Time.Sleep(0.0f);
            }

            // Finish introduction
            Time.Sleep(2.0f);
            mode.End_Boss_Intro();

            // Slam after a little bit of time.
            Time_To_Hit = Math.Rand(TIME_TO_SLAM_MIN, TIME_TO_SLAM_MAX);

            // Now go to the normal idle behaviour!
			change_state(Idle);
		}
		event void On_Exit()
		{
		}				
	}
	
	// -------------------------------------------------------------------------
	// Retreating into the ground.
	// -------------------------------------------------------------------------
	state Retreating
	{
		event void On_Enter()
		{
            State = Whipper_State.Retreating;
            FX.Spawn_Untracked("whipper_boss_dirt_puff_large", this.Center, 0.0f, Layer);
		}		
		event void On_Tick()
		{
            // Wait for a while for tentacles to retract.
            Audio.Play3D("sfx_objects_enemies_bosses_whipper_retreat", this.Center);

            while (Tentacle_Retract_Factor > 0.0f)
            {
                float delta_t = Time.Get_Delta_Seconds();
                if (Network.Is_Server())
                {
                    Tentacle_Retract_Factor = Math.Max(0.0f, Tentacle_Retract_Factor - (TENTACLE_RETRACT_SPEED * delta_t));

                    // Keep spinning.                
                    Spin_At_Speed(RETREAT_SPIN_SPEED);
                }

                Update_Tentacles();
                Time.Sleep(0.0f);
            }

            // Invisible time!
            shadow_sprite.Visible = false;
            body_sprite.Visible = false;

		    if (m_status_ailment_effect != null && Scene.Is_Spawned(m_status_ailment_effect))
		    {
			    m_status_ailment_effect.Get_Component().Visible = false;
		    }
            
            Time.Sleep(1.0f);

            // Find random point to emerge at.
            Actor[] emerge_points = Scene.Find_Actors(typeof(Boss_Whipper_Emerge_Marker));
            Boss_Whipper_Emerge_Marker marker = <Boss_Whipper_Emerge_Marker>emerge_points[Math.Rand(0, emerge_points.Length() - 1)];
            this.Position = marker.Position - (this.Collision_Center - this.Position);

            // Regenerate?
            if (bRegeneratePending)
            {
                bRegeneratedCount++;

                if (Network.Is_Server())
                {
                    Regenerate();             
                    m_active_tentacle_count = Math.Min(m_active_tentacle_count + 2, REGENERATE_TENTACLE_COUNT);
                }

                // Sleep before emerging.
                float timer = 5.0f;
                while (timer >= 0.0f)
                {
                    timer -= Time.Get_Delta_Seconds();
                    Update_Tentacles();
                    Time.Sleep(0.0f);
                }
            }
            else
            {
                // Sleep before emerging.
                Time.Sleep(2.0f);
            }

            change_state(Emerging);
		}
		event void On_Exit()
		{
		}		
	}
	
  	// -------------------------------------------------------------------------
	// Emerging from the ground.
	// -------------------------------------------------------------------------
	state Emerging
	{
		event void On_Enter()
		{
            State = Whipper_State.Emerging;
            FX.Spawn_Untracked("whipper_boss_dirt_puff_large", this.Center, 0.0f, Layer);
		}		
		event void On_Tick()
		{
            // Visible Time!
            shadow_sprite.Visible = true;
            body_sprite.Visible = true;

		    if (m_status_ailment_effect != null && Scene.Is_Spawned(m_status_ailment_effect))
		    {          
                Tick_Ailment();
			    m_status_ailment_effect.Get_Component().Visible = true;
		    }
            
            Audio.Play3D("sfx_objects_enemies_bosses_whipper_emerge", this.Center);

            // Wait for a while for tentacles to extend
            while (Tentacle_Retract_Factor < 1.0f)
            {
                float delta_t = Time.Get_Delta_Seconds();
                if (Network.Is_Server())
                {
                    Tentacle_Retract_Factor = Math.Min(1.0f, Tentacle_Retract_Factor + (TENTACLE_RETRACT_SPEED * delta_t));

                    // Keep spinning.               
                    Spin_At_Speed(RETREAT_SPIN_SPEED);
                }

                Update_Tentacles();
                Time.Sleep(0.0f);
            }

            // Sleep before idling.
            Time.Sleep(1.0f);
            
            change_state(Idle);
		}
		event void On_Exit()
		{
            bRegeneratePending = false;
		}	
	}

    // -------------------------------------------------------------------------
    // Crunch attack
    // -------------------------------------------------------------------------
    state Crunch_Attack
    {
		event void On_Enter()
		{
            State = Whipper_State.Crunch;

			Base_Update();
			Update_Sprites();
		}	
		event void On_Tick()
		{                    
            if (Network.Is_Server())
            {
                // Slow down to a stop.
                while (Math.Abs(Current_Speed - CRUNCH_TURN_SPEED) < 0.5f)
                {
                    Spin_At_Speed(CRUNCH_TURN_SPEED);
                    Update_Tentacles();
                    Time.Sleep(0.0f);
                }

                // Retreat tentacles.
                while (Tentacle_Retract_Factor > CRUNCH_RETREAT_MIN)
                {
                    float delta_t = Time.Get_Delta_Seconds();
                    Tentacle_Retract_Factor = Math.Max(CRUNCH_RETREAT_MIN, Tentacle_Retract_Factor - (TENTACLE_CRUNCH_RETRACT_SPEED * delta_t));

                    // Slow down to no speed.               
                    Spin_At_Speed(CRUNCH_TURN_SPEED);

                    Update_Tentacles();
                    Time.Sleep(0.0f);
                }

                // Flash a few times.
                Blink();
                Time.Sleep(0.5f);
                Blink();
                Time.Sleep(0.2f);
                Blink();
                Time.Sleep(0.1f);
            
                bUseHighDamage = true;

                // Slam all tentacles straight out and then elastic back into place.
                float slam_delta = 0.0f; 
                while (slam_delta < 1.0f)
                {
                    float delta_t = Time.Get_Delta_Seconds();
                    slam_delta = Math.Min(1.0f, slam_delta + (TENTACLE_CRUNCH_SLAM_SPEED * delta_t));
                    
                    Tentacle_Retract_Factor = Math.BezierCurveLerp(CRUNCH_RETREAT_MIN, 4.0f, 1.0f, 0.20f, slam_delta);
                    Spin_At_Speed(CRUNCH_TURN_SPEED);
                    
                    Update_Tentacles();
                    Time.Sleep(0.0f);
                }
            
                bUseHighDamage = false;
            }
            else 
            {
                while (State == Whipper_State.Crunch)
                {
                    Time.Sleep(0.0f);
                }
            }

            // Back to idle.
            change_state(Idle);
        }	
		event void On_Exit()
		{
		}	
    }
    	
    void Blink()
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Blink();
    }

    rpc void RPC_Blink()
    {
	    body_sprite.Play_Oneshot_Animation(BODY_BLINK_ANIMATION + "_" + Direction_Helper.Mnemonics[m_direction], 1.0f);
        for (int i = 0; i < m_active_tentacle_count; i++)
        {
            m_tentacles[i].Blink();
        }
    }

	// -------------------------------------------------------------------------
	// Idle spinning.
	// -------------------------------------------------------------------------
	state Idle
	{
		event void On_Enter()
		{
            State = Whipper_State.Idle;
            Crunch_Attack_Timer = Math.Rand(MIN_CRUNCH_INTERVAL, MAX_CRUNCH_INTERVAL);

			Base_Update();
			Update_Sprites();
		}		
		event void On_Tick()
		{
			Base_Update();
            Tick_Weapon();
            Update_Tentacles();
			Update_Sprites();  
              
            if (Network.Is_Server())
            {
                float delta_t = Time.Get_Delta_Seconds();

                m_segment_damage_this_frame = 0.0f;
             
                int TotalSegments = (m_active_tentacle_count * Boss_Whipper_Tentacle.MAX_SEGMENT_COUNT);
                int AliveSegments = TotalSegments - DeadSegments.Length();

                // If raised, move towards player.
                Actor[] humans = Scene.Find_Actors(typeof(Human_Player));

                Human_Player closest_player = null;
                float closest_dist = 0.0f;

                foreach (Human_Player human in humans)
                {
                    if (!human.Is_Incapacitated())
                    {
                        float dist = (this.Collision_Center - human.Collision_Center).Length();
                        if (closest_player == null || dist < closest_dist)
                        {
                            closest_player = human;
                            closest_dist = dist;
                        }
                    }
                }

                float distance_to_player = closest_dist;
                bool bNearPlayer = distance_to_player < ATTACK_DISTANCE;
                bool bAllPlayersFarAway = distance_to_player > FAR_AWAY_DISTANCE;

                Log.WriteOnScreen("WhipperBossPlayerState", Vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f, "Distance="+distance_to_player+" bNearPlayer="+bNearPlayer+" bAllPlayersFarAway="+bAllPlayersFarAway);
            
                if (bPanic)
                {
                    Run_From_Player();
                }
                else 
                {
                    Move_Towards_Player(closest_player);
                }

                // Player far enough away, start crunch attack.
                Crunch_Attack_Timer -= delta_t;
                if (Crunch_Attack_Timer < 0.0f && 
                    AliveSegments > TotalSegments * MIN_CRUNCH_SEGMENT_FRACTION && 
                    !bRaised && 
                    m_status_ailment != StatusAilmentType.Shocked && 
                    m_status_ailment != StatusAilmentType.Frozen)
                {
                    Crunch_Attack_Timer = Math.Rand(MIN_CRUNCH_INTERVAL, MAX_CRUNCH_INTERVAL);
                    change_state(Crunch_Attack);
                }

                // If defenceless for long enough, regenerate.
                float Warning_Health = bRegeneratedCount > 0 ? DEFENCELESS_HEALTH_THRESHOLD_REGENERATED : DEFENCELESS_HEALTH_THRESHOLD;
                if (AliveSegments < DEFENCELESS_THRESHOLD)// || Health < Warning_Health)
                {
                    if (!bRegeneratePending)
                    {
                        Defenceless_Time += Time.Get_Delta_Seconds();
                        if (Defenceless_Time >= REGENERATE_DEFENCELESS_INTERVAL && !bRaised) 
                        {
                            bRegeneratePending = true;
                            bRetreatPending = true;
                            Time_To_Hit = 0.0f;
                            return;
                        }
                    }
                    bPanic = true;
                }
                else 
                {            
                    bPanic = false;
                }

                // Spin tentacles.
                Spin_At_Speed(bRaised ? RAISED_SPIN_SPEED : IDLE_SPIN_SPEED);
                Time_To_Hit -= Time.Get_Delta_Seconds();

                if (Time_To_Hit <= 0.0f)
                {
                    if (bRaised)
                    {
                        if (bNearPlayer || Time_To_Hit < -MAX_SLAM_DELAY || bRetreatPending || bRegeneratePending)
                        {
                            // Periodically move to a different place.
                            if (bRetreatPending || bRegeneratePending || bAllPlayersFarAway)
                            {
                                Retreat();
                            }
                            else
                            {
                                Slam_All_Tentacles();
                            }
                        
                            Time_To_Hit = Math.Rand(TIME_TO_RAISE_MIN, TIME_TO_RAISE_MAX);
                        }
                    }
                    else
                    {
                        Raise_All_Tentacles();
                    
                        Time_To_Hit = Math.Rand(TIME_TO_SLAM_MIN, TIME_TO_SLAM_MAX);
                        bRetreatPending = (Math.Rand(0.0, 1.0f) <= MOVEMENT_PROBABILITY);
                    }
                }
            }
        }
	}   
	
  	// -------------------------------------------------------------------------
	// Enemy dieing!
	// -------------------------------------------------------------------------
	state Death
	{
		event void On_Enter()
		{
            Is_Dieing = true;
            State = Whipper_State.Death;
		}		
		event void On_Tick()
		{
            // Blow up individual tentacle segments.           
            while (Tentacle_Segment_Auto_Kill_Index > 0)
            {
                Time.Sleep(0.3f);      
                
                if (Network.Is_Server())        
                {
                    Tentacle_Segment_Auto_Kill_Index--;
                }
            }

            // Big boom and hide.
            Audio.Play3D("sfx_objects_enemies_bosses_whipper_destroy", this.Center);
            FX.Spawn_Untracked("whipper_boss_die", this.Center, 0.0f, Layer);
            Time.Sleep(1.0f);
            shadow_sprite.Visible = false;
            body_sprite.Visible = false;

            // Coins everywhere!
            FX.Spawn_Untracked("coin_splash_boss", this.Center, 0.0f, Layer);
            Time.Sleep(2.0f);
            
		    // Add score.
		    Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		    mode.Add_Score(Kill_Score_Bonus);
		    FX.Spawn_Untracked("score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, Kill_Score_Bonus);
            
            // Despawn!
            if (Network.Is_Server())
            {
                Despawn();
            }
		}
		event void On_Exit()
		{
		}	
	}

    void Run_From_Player()
    {    
        float delta_t = Time.Get_Delta_Seconds();

        m_panic_timer -= delta_t;
        if (m_panic_timer <= 0)
        {
            m_panic_vector = Vec3(Math.Rand(-1.0f, 1.0f), Math.Rand(-1.0f, 1.0f), 0.0f);
            m_panic_timer = Math.Rand(PANIC_SWITCH_VECTOR_MIN, PANIC_SWITCH_VECTOR_MAX);
        }

		// Move towards the chase vector.            
		Move_Towards_At_Speed(Position + m_panic_vector, PANIC_SPEED);
    }
    
    void Move_Towards_Player(Human_Player player) 
    {
        if (player != null)
        {
			path_planner.Source_Position = this.Collision_Center;
			path_planner.Target_Position = player.Collision_Center;

            Vec3 chase_vector = path_planner.Movement_Vector;
			
            Log.WriteOnScreen("WhipperBossMoveState", Vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f, "Moving at speed "+(bRaised ? CHASE_SPEED : IDLE_SPEED));

			// Move towards the chase vector.            
			Move_Towards_At_Speed(Position + chase_vector, (bRaised ? CHASE_SPEED : IDLE_SPEED));
        }
    }
	 
	bool Move_Towards_At_Speed(Vec3 target, float speed)
	{ 
		Vec3 start_position = this.Position;
		Vec3 final_position = start_position;
		float delta = Time.Get_Delta();
		bool at_target = false;

        // Shocked slows us down.
		if (m_status_ailment == StatusAilmentType.Shocked)
        {
            speed *= SHOCK_SPEED_MULTIPLIER;
        }        
		if (m_status_ailment == StatusAilmentType.Frozen)
        {
            speed *= FREEZE_SPEED_MULTIPLIER;
        }

		// Apply movement.
		Vec3 vector_to_target = (target - final_position).Unit();
		Vec3 movement = ((vector_to_target * speed) * delta);
        
		final_position = final_position + movement;
	
        m_direction = (movement.Y > 0.0f ? Direction.S : Direction.N);

		// Onwards!
		this.Move_Towards(final_position);

		return at_target;
	}

	// -------------------------------------------------------------------------
	// Damage handling!
	// -------------------------------------------------------------------------        
    void Segment_Damaged(int index, float damage)
    {
        damage = Math.Min(damage, MAX_SEGMENT_DAMAGE_PER_INSTANCE);

        float max_damage = Math.Max(0.0f, MAX_SEGMENT_DAMAGE_PER_FRAME - m_segment_damage_this_frame);
        float dmg = Math.Min(max_damage, damage * SEGMENT_DAMAGE_MULTIPLIER);
        
        m_segment_damage_this_frame += dmg;

        //Log.Write("index="+index+" damage="+damage+" max_damage="+max_damage+" dmg="+dmg);
        
        Health -= dmg;
        if (Health <= 0.0f)
        {
            Health = 0.0f;
            if (Network.Is_Server() && !Is_Dieing)
            {
                Begin_Dieing();
            }
        }
    }

    public float Adjust_Damage(float damage, CollisionDamageType damage_type)
    {
        if (damage_type == CollisionDamageType.Laser)
        {
            return damage * 0.07f;
        }
        else if (damage_type == CollisionDamageType.Fire)
        {
            return damage * 0.075f;
        }
        else if (damage_type == CollisionDamageType.Explosive)
        {
            return damage * 0.25f;
        }
        else if (damage_type == CollisionDamageType.Acid)
        {
            return damage * 1.0f;//0.35f;
        }
        else if (damage_type == CollisionDamageType.Poison)
        {
            // Poison dosen't hurt us at all.
            return 0.0f;
        }

        return damage;
    }

	event bool On_Damage(
		CollisionComponent component, 
		Actor instigator, 
		Vec3 position, 
		float dir, 
		float damage, 
		CollisionDamageType type,
		int ricochet_count,
		int penetration_count,
		Weapon_SubType weapon_subtype,
        Item_Archetype weapon_item)
	{
		// Still alive?
		if (Is_Dieing || Is_Dead)
		{
			return false;
		}
        
		// Ignore own projectiels.
		if (instigator == this)
		{
			return false;
		}

        // Ignore tentacle damage.
        Boss_Whipper_Tentacle_Segment segment = <Boss_Whipper_Tentacle_Segment>instigator;
        if (segment != null)
        {
            return false;
        }

		// Ignore buffs.
		if (type == CollisionDamageType.Buff_Ammo	||
			type == CollisionDamageType.Buff_Health	||
			type == CollisionDamageType.Buff_Damage	||
			type == CollisionDamageType.Buff_Price	||
			type == CollisionDamageType.Buff_Reload ||
			type == CollisionDamageType.Buff_ROF	||
			type == CollisionDamageType.Buff_Speed	||
			type == CollisionDamageType.Buff_XP)
		{
			return true;
		}

		// Ignore healing based damage.
		if (damage < 0)
		{
			return false;
		}
			
        // Adjust damage by resistences.
        damage = Adjust_Damage(damage, type);

		// Scale by instigators damage multiplier.
		damage = Calculate_Damage_Multiplier(instigator, damage, weapon_subtype);

		bool accepted = Apply_Damage(component, instigator, position, dir, damage, type, ricochet_count, penetration_count, true, weapon_item);
		if (accepted == true)
		{
			// Throttle the FX if we are taking constant damage (acid etc)
			if (Throttle_Damage_FX(damage))
			{
				// Spawn a hit effect.
				FX.Spawn_Untracked("whipper_boss_hit", position.Offset(dir, HIT_OFFSET_FX_AMOUNT), dir, Layer);
			
				// Show HP reduction.
				FX.Spawn_Untracked("score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>Get_Throttled_Damage());
			}

			// Flash in damage.	
			if (m_status_ailment != StatusAilmentType.Shocked)
			{
				body_sprite.Play_Oneshot_Animation(BODY_HURT_ANIMATION + "_" + Direction_Helper.Mnemonics[m_direction], 1.0f);
			}
	
			// Notify nearby that we have been hurt.
			Event_Hear_Sound hear_evt = new Event_Hear_Sound();
			hear_evt.Instigator = instigator;
			hear_evt.Fire_In_Radius(Center, HURT_EVENT_RADIUS);			
            	
            // Notify everyone of what happened.
            // Note: Shock/Freeze based weapons do not give score increases on hit, otherwise they can just spam it and the enemy will not be able to respond :(
            bool bIsAilmentDamage = (type == CollisionDamageType.Ice || type == CollisionDamageType.Shock);
            if (!bIsAilmentDamage || !bHasGivenAilmentScore)
            {
                Event_Pawn_Hurt evt = new Event_Pawn_Hurt();
                evt.Hurt = this;
                evt.Harmer = instigator;
                evt.Damage = damage;
                evt.Score_Increase = bIsAilmentDamage ? AILMENT_HIT_SCORE_INCREASE : HIT_SCORE_INCREASE;
                evt.Damage_Type = type;
                evt.Weapon_Sub_Type = weapon_subtype;
                evt.Fire_Global();

                if (bIsAilmentDamage)
                { 
                    bIsAilmentDamage = true;
                    bHasGivenAilmentScore = true;
                }
            }
				
			// Dead?		
            if (Network.Is_Server() && Health <= 0.0f)
            {
                Health = 0.0f;
                if (!Is_Dieing && !Is_Dead)
                {
                    Begin_Dieing();
                }
            }
		}

		return accepted;
	}
	
	// -------------------------------------------------------------------------
	//  Sprite / animation updating. 
	// -------------------------------------------------------------------------
	void Update_Sprites()
	{
		// Use shocked sprite if neccessary.
		if (m_status_ailment == StatusAilmentType.Shocked)
		{
			body_sprite.Animation = BODY_SHOCK_ANIMATION + "_" + Direction_Helper.Mnemonics[m_direction];
		}
		else
		{
			body_sprite.Animation = BODY_IDLE_ANIMATION + "_" + Direction_Helper.Mnemonics[m_direction];
		}		

        body_sprite.Animation_Speed = (bPanic ? 2.0f : 1.0f);		
	}
	 
	// -------------------------------------------------------------------------
	//  Replication
	// -------------------------------------------------------------------------
	replicate(ReplicationPriority.High)
	{
        m_active_tentacle_count
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
        State
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
        Angle
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Interpolate_Smoothed;
		}
        Tentacle_Retract_Factor
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Interpolate_Smoothed;
		}
        bRegeneratePending
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
        Tentacle_Segment_Auto_Kill_Index
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
        Health
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Interpolate_Smoothed;
		}
        bUseHighDamage
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}

	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{	
		SpriteComponent shadow_sprite
		{
			Color 			= Vec4(0.0, 0.0, 0.0, 200.0);
			Area			= Vec4(1.0, 21.0, 30.0, 18.0);
			Scale			= true;
			Depth_Bias 		= -128.0;
			Sort_By_Transparency = true;
            Visible     = false;
		}		
		SpriteComponent body_sprite
		{
			Color		= Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias	= 0.0f;
            Visible     = false;
		}
		
		// Movement collision.
		CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(7, 16, 16, 16);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Environment;
		}

		// Hit boxies.
		CollisionComponent body_hitbox
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(6, 14, 19, 18);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Damage;
		}

		// Boids for seperation. Should probably use cohesion
		// and avoidance as well, but blah.
		BoidComponent boid
		{
			Center				= Vec2(14, 22);				
			Neighbour_Distance	= 72.0f;
			Seperation_Distance	= 30.0f;
			Maximum_Speed		= 200.0f;
			Maximum_Force		= 200.0f;
			Cohesion_Weight		= 0.0f;
			Avoidance_Weight	= 0.0f;
			Seperation_Weight	= 0.0f;
		}

		PathPlannerComponent path_planner
		{
			Regenerate_Delta = 64.0f;
			Collision_Group  = CollisionGroup.Enemy;
		}
	}
}
