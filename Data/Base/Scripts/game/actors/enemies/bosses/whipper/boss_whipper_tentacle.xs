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
  
[
	Placeable(false), 
	Name("Whipper Boss Tentacle Segment"), 
	Description("The whip!") 
]
public class Boss_Whipper_Tentacle_Segment : Actor
{  
    // How long to stay squashed for.
    private float m_squash_timer = 0.0f;

	const string BODY_IDLE_ANIMATION		= "boss_whipper_tentacle_idle";
	const string BODY_HURT_ANIMATION		= "boss_whipper_tentacle_hurt";
	const string BODY_BLINK_ANIMATION		= "boss_whipper_tentacle_blink";
	const string BODY_SHOCK_ANIMATION		= "boss_whipper_tentacle_shocked";
	const string BODY_SQUASH_MODIFIER		= "_squashed";

    // Distance between each tentacle segment.
    const float BASE_DISTANCE = 6.0f;
    const float DISTANCE_BETWEEN_SEGMENTS = 13.0f;
    const float DISTANCE_BETWEEN_SEGMENTS_RAISED = 4.0f;

    // Tentacle that owns us.
    public Boss_Whipper_Tentacle Parent_Tentacle = null;

    // Index in chain of this segment.
    public int Index = 0;

    // Global index into the boss whipper segment array.
    public int Global_Index = 0;

    // Height of segment.
    public float Height = 0.0f;
    
	// Bobing effect.
	const float BOB_HEIGHT = 32.0f;
	const float BOB_SCALE_REDUCTION = 0.5f;

    // Tracks the boss angle with delay depending on how we are up the chain.
    public float Boss_Angle_Tracking = 0.0f;
    const float TENTACLE_LERP_DELAY_FACTOR = 1.2f;
    const float LERP_REDUCTION_PER_SEGMENT = 0.03f;
    
    // Raising/Lowering factors.
    public float Offset_Distance = 0.0f;

    const float UNRAISED_VELOCITY = 150.0f;
    const float RAISED_VELOCITY = 30.0f;

    const float SQUASH_DURATION = 0.3f;

    public bool m_waiting_for_squash = false;

    const float DAMAGE_COOLOFF_INTERVAL = 0.3f;
    public float Damage_Cooloff = 0.0f;

    const float MAX_DAMAGE_PER_FRAME = 7.0f;
    float Damage_Per_Frame_Counter = 0.0f;

    // Health state.
    const float BASE_HEALTH = 25.0f;
    public bool Is_Dead = false;
    public float Health = 25.0f;
     
	// -------------------------------------------------------------------------
	//  Coonstructor.
	// -------------------------------------------------------------------------
	Boss_Whipper_Tentacle_Segment()
	{
		Bounding_Box			= Vec4(0, 0, 20, 20);
		Depth_Y_Offset			= 20;
	}
                
	// -------------------------------------------------------------------------
	// We don't really need to do anything, we are driven by the boss.
	// -------------------------------------------------------------------------
	default state Active
	{
		event void On_Tick()
		{
            // Boss dead, time to remove tentacle.
            if (!Scene.Is_Spawned(Parent_Tentacle) || !Scene.Is_Spawned(Parent_Tentacle.Parent_Boss))
            {
                Scene.Despawn(this);
                return;
            }

            Update_Offset();
            Update_Damage();
            Update_Sprites();

            Damage_Per_Frame_Counter = 0.0f;
            
            Boss_Whipper boss = Parent_Tentacle.Parent_Boss;
            if (boss.DeadSegments.Contains(Global_Index))
            {    
                FX.Spawn_Untracked("whipper_boss_segment_destroy", this.Position, 0.0f, Layer, this);
                change_state(Dead);
            }
		}	
	}

    state Dead
    {
        event void On_Enter()
        {
            body_sprite.Visible = false;
            shadow_sprite.Visible = false;
            body_hitbox.Enabled = false;
            boid.Enabled = false;
            Is_Dead = true;
        }
        event void On_Exit()
        {
            body_sprite.Visible = Parent_Tentacle.Parent_Boss.Is_Visible();
            shadow_sprite.Visible = Parent_Tentacle.Parent_Boss.Is_Visible();
            body_hitbox.Enabled = true;
            boid.Enabled = true;
            Is_Dead = false;
            Health = BASE_HEALTH;
        }
        event void On_Tick()
        {
            // Boss dead, time to remove tentacle.
            if (!Scene.Is_Spawned(Parent_Tentacle) || !Scene.Is_Spawned(Parent_Tentacle.Parent_Boss))
            {
                Scene.Despawn(this);
                return;
            }

            Boss_Whipper boss = Parent_Tentacle.Parent_Boss;
            if (!boss.DeadSegments.Contains(Global_Index))
            {    
                change_state(Active);
            }
        }
    }
		     
	// -------------------------------------------------------------------------
	//  Spawn damage vfx periodically.
	// -------------------------------------------------------------------------
    void Update_Damage()
    {
        Boss_Whipper boss = Parent_Tentacle.Parent_Boss;
        if (Height < 4.0f && boss.Tentacle_Retract_Factor >= 1.0f)
        {
            Damage_Cooloff -= Time.Get_Delta_Seconds();
            if (Damage_Cooloff <= 0.0f)
            {
                Damage_Cooloff = DAMAGE_COOLOFF_INTERVAL;

                if (boss.bUseHighDamage)
                {
                    FX.Spawn_Untracked("whipper_boss_high_damage", this.Position, 0.0f, Layer, this); 
                }
                else
                {         
                    FX.Spawn_Untracked("whipper_boss_damage", this.Position, 0.0f, Layer, this);      
                }
            }
            body_hitbox.Enabled = boss.Is_Visible();
        }
        else
        {
            body_hitbox.Enabled = boss.Is_Visible() && (Height < 4.0f && boss.Tentacle_Retract_Factor >= 0.5f);
        }
    }

	// -------------------------------------------------------------------------
	//  Update offsets from parent.
	// -------------------------------------------------------------------------
	void Update_Offset()
	{
        float delta_t = Time.Get_Delta_Seconds();

        float lerp_factor = Index * LERP_REDUCTION_PER_SEGMENT;
        lerp_factor = 1.0f - Math.Clamp(lerp_factor * TENTACLE_LERP_DELAY_FACTOR, 0.0f, 0.9f);

        float distance = BASE_DISTANCE + ((Index + 1) * DISTANCE_BETWEEN_SEGMENTS);
        Boss_Whipper boss = Parent_Tentacle.Parent_Boss;
        
        float target_height = 0.0f;
        float velocity = UNRAISED_VELOCITY;

        // Modify offset to raise segment into the air if raising tentacle.
        if (boss.bRaised)
        {        
            distance = BASE_DISTANCE + ((Index + 1) * DISTANCE_BETWEEN_SEGMENTS_RAISED); 
            target_height = (Index * DISTANCE_BETWEEN_SEGMENTS);
            velocity = RAISED_VELOCITY;
            m_waiting_for_squash = true;
        }
        else
        {
            if (m_waiting_for_squash)
            {
                if (Height == 0.0f)
                {
			        FX.Spawn_Untracked("whipper_boss_dirt_puff_small", this.Position, 0.0f, Layer, this);
                    m_squash_timer = SQUASH_DURATION;
                    m_waiting_for_squash = false;
                }
            }
        }

        // Lerp to target offset and height.
        //Offset_Distance = Math.Lerp(Offset_Distance, distance, lerp_factor * delta_t);
        //Height = Math.Lerp(Height, target_height, lerp_factor * delta_t);
        Offset_Distance = Math.FixedVelocityLerp(Offset_Distance, distance, velocity * delta_t);
        Height = Math.FixedVelocityLerp(Height, target_height,velocity * delta_t);

        // Lerp to bosses angle. The further out we are in the chain, the slower we lerp to the angle, giving
        // the impression of some form of IK.
        Boss_Angle_Tracking = Math.Lerp(this.Boss_Angle_Tracking, boss.Angle, lerp_factor * delta_t);

        float final_angle = this.Boss_Angle_Tracking + Parent_Tentacle.Angle;

        // Position this segment relative to bosses rotation.
        Vec3 offset = boss.Center;
        offset = offset + (Vec3(Math.Sin(final_angle), Math.Cos(final_angle), 0.0f) * Offset_Distance) + Vec3(0.0f, -Height, 0.0f); 

        // Lerp based on the tentacle retreat factor.
        this.Position = Math.Lerp(boss.Center, offset, boss.Tentacle_Retract_Factor);   
    }

	// -------------------------------------------------------------------------
	//  Sprite / animation updating. 
	// -------------------------------------------------------------------------
	void Update_Sprites()
	{
        string animation = BODY_IDLE_ANIMATION;

		// Use shocked sprite if neccessary.
		if (Parent_Tentacle.Parent_Boss.m_status_ailment == StatusAilmentType.Shocked)
		{
			animation = BODY_SHOCK_ANIMATION;
        }

        m_squash_timer -= Time.Get_Delta_Seconds();
        if (m_squash_timer > 0.0f)
        {
            animation  += BODY_SQUASH_MODIFIER;
        }

        body_sprite.Animation = animation;
        body_sprite.Offset = Vec2(0.0f, 0.0f);
        body_sprite.Depth_Bias = Height;
		body_sprite.Visible = Parent_Tentacle.Parent_Boss.Is_Visible();

        // Set shadow position.
        float delta = 1.0f - ((Math.Min(BOB_HEIGHT, Height) / BOB_HEIGHT) * BOB_SCALE_REDUCTION);
        shadow_sprite.Draw_Scale = Vec2(delta, delta);
        shadow_sprite.Offset = Vec2(0.0f, (Height * Parent_Tentacle.Parent_Boss.Tentacle_Retract_Factor) - BASE_DISTANCE);
        shadow_sprite.Area = Vec4(-6.0, 8.0, 12.0, 10.0);
        shadow_sprite.Frame = "actor_shape_oval_0";
		shadow_sprite.Visible = Parent_Tentacle.Parent_Boss.Is_Visible();

        // Disable boid if we are in the air.        
		boid.Maximum_Speed		= Height > 8.0f ? 0.0f : 5000.0f;
		boid.Maximum_Force		= Height > 8.0f ? 0.0f : 5000.0f;
    }

    void Blink()
    {
		body_sprite.Play_Oneshot_Animation(BODY_BLINK_ANIMATION, 1.0f);
    }

	// -------------------------------------------------------------------------
	// Damage handling!
	// -------------------------------------------------------------------------
    public override int Get_Team_Index()
	{
		return Enemy_Manager.Default_Team_Index;
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
		// Ignore own projectiels.
		if (instigator == this)
		{
			return false;
		}
        
        // Already dead?
        if (Is_Dead)
        {
            return false;
        }

        // Ignore tentacle damage.
        Boss_Whipper boss = Parent_Tentacle.Parent_Boss;
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
        damage = boss.Adjust_Damage(damage, type);
        
        // Damer per frame limit.
        float damage_remaining = Math.Max(0.0f, MAX_DAMAGE_PER_FRAME - Damage_Per_Frame_Counter);
        damage = Math.Min(damage, damage_remaining);
        Damage_Per_Frame_Counter += damage;

        // Server controls boss damage.
        if (Network.Is_Server())
        {
            if (penetration_count == 0)
            {
                float Diff = Health - Math.Max(Health - damage, 0.0f);
                boss.Segment_Damaged(this.Global_Index, Diff);
            }

            Health -= damage;
         
            if (Health <= 0.0f)
            {
                boss.Kill_Segment(this.Global_Index);
            }
        }	
        
        // Play hurt animation!
        string animation = BODY_HURT_ANIMATION;
        if (m_squash_timer > 0.0f)
        {
            animation += BODY_SQUASH_MODIFIER;
        }
		body_sprite.Play_Oneshot_Animation(animation, 1.0f);
			
		return true;
	}
	
	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{	
		SpriteComponent shadow_sprite
		{
			Color 			= Vec4(0.0, 0.0, 0.0, 200.0);
			Area			= Vec4(-6.0, 8.0, 12.0, 10.0);
			Scale			= true;
			Depth_Bias 		= -128.0;
			Sort_By_Transparency = true;
            Visible = false;
		}		
		SpriteComponent body_sprite
		{
			Color		= Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias	= 0.0f;
            Visible = false;
		}	
		BoidComponent boid
		{
			Center				= Vec2(0, 0);				
			Neighbour_Distance	= 40.0f;
			Seperation_Distance	= 20.0f;
			Maximum_Speed		= 5000.0f;
			Maximum_Force		= 5000.0f;
			Cohesion_Weight		= 0.0f;
			Avoidance_Weight	= 0.0f;
			Seperation_Weight	= 0.0f;
		}	
		CollisionComponent body_hitbox
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(-10, -10, 20, 20);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Damage;
		}
	}
}

[
	Placeable(false), 
	Name("Whipper Boss Tentacle"), 
	Description("The whip!") 
]
public class Boss_Whipper_Tentacle : Actor
{  
    // Tentacle parts.
    public const int MAX_SEGMENT_COUNT = 7;
    private Boss_Whipper_Tentacle_Segment[] m_segments = new Boss_Whipper_Tentacle_Segment[0];

    // Boss that owns us.
    public Boss_Whipper Parent_Boss = null;

    // Angle to produde from boss.
    public float Angle = 0.0f;

    // Tentacle index.
    public int Index = 0;
    
	// -------------------------------------------------------------------------
	//  Coonstructor.
	// -------------------------------------------------------------------------
	Boss_Whipper_Tentacle()
	{
		Bounding_Box = Vec4(0, 0, 20, 20);
	}

	// -------------------------------------------------------------------------
	// We don't really need to do anything, we are driven by the boss.
	// -------------------------------------------------------------------------
	default state Active
	{
        event void On_Tick()
		{
            // Boss dead, time to remove tentacle.
            if (!Scene.Is_Spawned(Parent_Boss))
            {
                Scene.Despawn(this);
                return;
            }

            Create_Segments();
            Update_Segments();
		}	
	}
		 
	// -------------------------------------------------------------------------
	//  Creates all the tentacle segments.
	// -------------------------------------------------------------------------
	void Create_Segments()
	{
        float delta_t = Time.Get_Delta_Seconds();

        while (MAX_SEGMENT_COUNT > m_segments.Length())
        {
            Boss_Whipper_Tentacle_Segment tentacle = <Boss_Whipper_Tentacle_Segment>Scene.Spawn(typeof(Boss_Whipper_Tentacle_Segment), null);
            tentacle.Parent = this;
            tentacle.Parent_Tentacle = this;
            tentacle.Index = m_segments.Length();
            tentacle.Position = Parent_Boss.Center;
            tentacle.Layer = Parent_Boss.Layer;
            tentacle.Global_Index = (Index * MAX_SEGMENT_COUNT) + m_segments.Length();
            m_segments.AddLast(tentacle);
            Log.Write("[BOSS] Added segment to tentacle "+this.Angle+" for a total of " + m_segments.Length());
        }
    }
    
    void Update_Segments()
	{
        // If dead we do not want to update indexes, we want to kill each segment progressively.
        if (Parent_Boss.Is_Dieing)
        {
            int tentacle_index = 0;
            for (int i = 0; i < m_segments.Length(); i++)
            {
                Boss_Whipper_Tentacle_Segment tentacle = m_segments[i];
                if (!tentacle.Is_Dead && tentacle.Index >= Parent_Boss.Tentacle_Segment_Auto_Kill_Index)
                {   
                    Parent_Boss.Kill_Segment(tentacle.Global_Index);
                }
            }
        }
        else
        {
            // We set the segment indexes to a contiguous index so we can "pop out" individual segments.
            int tentacle_index = 0;
            for (int i = 0; i < m_segments.Length(); i++)
            {
                Boss_Whipper_Tentacle_Segment tentacle = m_segments[i];
                if (!tentacle.Is_Dead)
                {
                    tentacle.Index = tentacle_index;
                    tentacle_index++;
                }
            }
        }
    }
    	
    void Blink()
    {
        for (int i = 0; i < m_segments.Length(); i++)
        {
            Boss_Whipper_Tentacle_Segment tentacle = m_segments[i];
            if (!tentacle.Is_Dead)
            {
                tentacle.Blink();
            }
        }
    }

	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{			
	}
}
