// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_PARTICLES_PARTICLEFX_
#define _ENGINE_UI_PARTICLES_PARTICLEFX_

#include "Engine/Engine/FrameTime.h"
#include "Engine/Config/ConfigFile.h"
#include "Engine/Scene/Collision/CollisionManager.h"
#include <vector>

struct AtlasAnimation;
struct AtlasFrame;
class SoundHandle;
class ParticleManager;
struct ParticleFX_Curve;
class UIScene_PfxEditor;

struct ParticleFX_EmitterDrawMethod
{
	enum Type
	{
		Sprite,
		Numbers,
		Beam,

		COUNT
	};
	

	static const char* To_String(ParticleFX_EmitterDrawMethod::Type type)
	{
		static const char* g_names[] = {
			"Sprite",
			"Numbers",
			"Beam"
		};

		return g_names[(int)type];
	}

	static ParticleFX_EmitterDrawMethod::Type Parse(const char* value)
	{
		if (stricmp(value, "Sprite") == 0)
			return Sprite;
		if (stricmp(value, "Text") == 0 || stricmp(value, "Numbers") == 0)
			return Numbers;
		if (stricmp(value, "Beam") == 0)
			return Beam;

		DBG_ASSERT(false);
		return Sprite;
	}
};

struct ParticleFX_EmitterCollectionType
{
	enum Type
	{
		Coin,

		COUNT
	};


	static const char* To_String(ParticleFX_EmitterCollectionType::Type type)
	{
		static const char* g_names[] = {
			"Coin"
		};

		return g_names[(int)type];
	}

	static ParticleFX_EmitterCollectionType::Type Parse(const char* value)
	{
		if (stricmp(value, "Coin") == 0)
			return Coin;

		DBG_ASSERT(false);
		return Coin;
	}
};

struct ParticleFX_DamageType
{
	enum Type
	{
		// WARNING: Keep synced with CollisionDamageType.
		NONE,

		Projectile,
		Melee,
		Fire,
		Ice,
		Explosive,
		Acid,
		Shock,
		Poison,
		Laser,

		Buff_Ammo,
		Buff_Health,
		Buff_Damage,
		Buff_Price,
		Buff_Reload,
		Buff_ROF,
		Buff_Speed,
		Buff_XP,

		COUNT
	};

	static const char* To_String(ParticleFX_DamageType::Type type)
	{
		static const char* g_names[] = {
			"NONE",
			"Projectile",
			"Melee",
			"Fire",
			"Ice",
			"Explosive",
			"Acid",
			"Shock",
			"Poison",
			"Laser",
			"Buff_Ammo",
			"Buff_Health",
			"Buff_Damage",
			"Buff_Price",
			"Buff_Reload",
			"Buff_ROF",
			"Buff_Speed",
			"Buff_XP",
		};

		return g_names[(int)type];
	}

	static ParticleFX_DamageType::Type Parse(const char* value)
	{
		if (stricmp(value, "None") == 0)		return NONE;
		if (stricmp(value, "Projectile") == 0)	return Projectile;
		if (stricmp(value, "Melee") == 0)		return Melee;
		if (stricmp(value, "Fire") == 0)		return Fire;
		if (stricmp(value, "Ice") == 0)			return Ice;
		if (stricmp(value, "Explosive") == 0)	return Explosive;
		if (stricmp(value, "Acid") == 0)		return Acid;
		if (stricmp(value, "Shock") == 0)		return Shock;
		if (stricmp(value, "Poison") == 0)		return Poison;
		if (stricmp(value, "Laser") == 0)		return Laser;
		if (stricmp(value, "Buff_Ammo") == 0)	return Buff_Ammo;
		if (stricmp(value, "Buff_Health") == 0)	return Buff_Health;
		if (stricmp(value, "Buff_Damage") == 0)	return Buff_Damage;
		if (stricmp(value, "Buff_Price") == 0)	return Buff_Price;
		if (stricmp(value, "Buff_Reload") == 0)	return Buff_Reload;
		if (stricmp(value, "Buff_ROF") == 0)	return Buff_ROF;
		if (stricmp(value, "Buff_Speed") == 0)	return Buff_Speed;
		if (stricmp(value, "Buff_XP") == 0)		return Buff_XP;

		DBG_ASSERT(false);
		return Projectile;
	}

	static CollisionDamageType::Type Get_Collision_Damage_Type(ParticleFX_DamageType::Type type)
	{
		return (CollisionDamageType::Type)((int)type);
	}
};

struct ParticleFX_CollisionResponse
{
	enum Type
	{
		Bounce,
		Stop,

		COUNT
	};
	
	static const char* To_Name(ParticleFX_CollisionResponse::Type type)
	{
		static const char* g_names[] = {
			"Bounce",
			"Stop"
		};

		return g_names[(int)type];
	}
	
	static const char* To_String(ParticleFX_CollisionResponse::Type type)
	{
		static const char* g_names[] = {
			"Bounce",
			"Stop"
		};

		return g_names[(int)type];
	}

	static ParticleFX_CollisionResponse::Type Parse(const char* value)
	{
		if (stricmp(value, "Bounce") == 0)
			return Bounce;
		if (stricmp(value, "Stop") == 0)
			return Stop;

		DBG_ASSERT(false);
		return Stop;
	}
};

struct ParticleFX_EmitterBlendMode
{
	enum Type
	{
		Alpha,
		Lighten,

		COUNT
	};

	static const char* To_Name(ParticleFX_EmitterBlendMode::Type type)
	{
		static const char* g_names[] = {
			"Alpha",
			"Lighten"
		};

		return g_names[(int)type];
	}

	static const char* To_String(ParticleFX_EmitterBlendMode::Type type)
	{
		static const char* g_names[] = {
			"Alpha",
			"Lighten"
		};

		return g_names[(int)type];
	}

	static ParticleFX_EmitterBlendMode::Type Parse(const char* value)
	{
		if (stricmp(value, "Alpha") == 0)
			return Alpha;
		if (stricmp(value, "Lighten") == 0)
			return Lighten;

		DBG_ASSERT(false);
		return Alpha;
	}
};

struct ParticleFX_SpawnShape
{
	enum Type
	{
		Point,
		Circle,
		Rect,

		COUNT
	};
	
	static const char* To_Name(ParticleFX_SpawnShape::Type type)
	{
		static const char* g_names[] = {
			"Point",
			"Circle",
			"Rect"
		};

		return g_names[(int)type];
	}
	
	static const char* To_String(ParticleFX_SpawnShape::Type type)
	{
		static const char* g_names[] = {
			"Point",
			"Circle",
			"Rect"
		};

		return g_names[(int)type];
	}

	static ParticleFX_SpawnShape::Type Parse(const char* value)
	{
		if (stricmp(value, "Point") == 0)
			return Point;
		if (stricmp(value, "Circle") == 0)
			return Circle;
		if (stricmp(value, "Rect") == 0)
			return Rect;

		DBG_ASSERT(false);
		return Point;
	}
};

struct ParticleFX_CurveType
{
	enum Type
	{
		Sprite_Frame,
		Direction,
		Spawn_Rate,
		Velocity_X,
		Velocity_Y,
		Velocity_Height,
		Velocity_Rotation,
		Velocity_Direction,
		Color_R,
		Color_G,
		Color_B,
		Color_A,
		Scale_X,
		Scale_Y,
		Lifetime,
		Rotation,
		Height,
		Damage,

		COUNT
	};
	
	static const char* To_Name(ParticleFX_CurveType::Type type)
	{
		static const char* g_names[] = {
			"Sprite Frame",
			"Direction",
			"Spawn Rate",
			"Velocity X",
			"Velocity Y",
			"Velocity Height",
			"Velocity Rotation",
			"Velocity Direction",
			"Color Red",
			"Color Green",
			"Color Blue",
			"Color Alpha",
			"Scale X",
			"Scale Y",
			"Lifetime",
			"Rotation",
			"Height",
			"Damage"
		};

		return g_names[(int)type];
	}

	static const char* To_String(ParticleFX_CurveType::Type type)
	{
		static const char* g_names[] = {
			"Sprite_Frame",
			"Direction",
			"Spawn_Rate",
			"Velocity_X",
			"Velocity_Y",
			"Velocity_Height",
			"Velocity_Rotation",
			"Velocity_Direction",
			"Color_R",
			"Color_G",
			"Color_B",
			"Color_A",
			"Scale_X",
			"Scale_Y",
			"Lifetime",
			"Rotation",
			"Height",
			"Damage"
		};

		return g_names[(int)type];
	}

	static ParticleFX_CurveType::Type Parse(const char* value)
	{
		if (stricmp(value, "Sprite_Frame") == 0)
			return Sprite_Frame;
		if (stricmp(value, "Direction") == 0)
			return Direction;
		if (stricmp(value, "Spawn_Rate") == 0)
			return Spawn_Rate;
		if (stricmp(value, "Velocity_X") == 0)
			return Velocity_X;
		if (stricmp(value, "Velocity_Y") == 0)
			return Velocity_Y;
		if (stricmp(value, "Velocity_Height") == 0)
			return Velocity_Height;
		if (stricmp(value, "Velocity_Rotation") == 0)
			return Velocity_Rotation;
		if (stricmp(value, "Velocity_Direction") == 0)
			return Velocity_Direction;
		if (stricmp(value, "Color_R") == 0)
			return Color_R;
		if (stricmp(value, "Color_G") == 0)
			return Color_G;
		if (stricmp(value, "Color_B") == 0)
			return Color_B;
		if (stricmp(value, "Color_A") == 0)
			return Color_A;
		if (stricmp(value, "Scale_X") == 0)
			return Scale_X;
		if (stricmp(value, "Scale_Y") == 0)
			return Scale_Y;
		if (stricmp(value, "Lifetime") == 0)
			return Lifetime;
		if (stricmp(value, "Rotation") == 0)
			return Rotation;
		if (stricmp(value, "Height") == 0)
			return Height;
		if (stricmp(value, "Damage") == 0)
			return Damage;

		DBG_ASSERT(false);
		return Sprite_Frame;
	}
};

struct ParticleFX_EventTrigger
{
	enum Type
	{
		Time,
		Bounce,
		Collision,
		Penetration,
		Collect,
		OwnerTrigger,
		OwnerDestroy,
		
		COUNT
	};
	
	static const char* To_Name(ParticleFX_EventTrigger::Type type)
	{
		static const char* g_names[] = {
			"Time",
			"Bounce",
			"Collision",
			"Penetration",
			"Collect",
			"Owner Trigger",
			"Owner Destroy"
		};

		return g_names[(int)type];
	}

	static const char* To_String(ParticleFX_EventTrigger::Type type)
	{
		static const char* g_names[] = {
			"Time",
			"Bounce",
			"Collision",
			"Penetration",
			"Collect",
			"OwnerTrigger",
			"OwnerDestroy"
		};

		return g_names[(int)type];
	}

	static ParticleFX_EventTrigger::Type Parse(const char* value)
	{
		if (stricmp(value, "time") == 0)
			return Time;
		else if (stricmp(value, "bounce") == 0)
			return Bounce;
		else if (stricmp(value, "collision") == 0)
			return Collision;
		else if (stricmp(value, "penetration") == 0)
			return Penetration;
		else if (stricmp(value, "collect") == 0)
			return Collect;
		else if (stricmp(value, "ownertrigger") == 0)
			return OwnerTrigger;
		else if (stricmp(value, "ownerdestroy") == 0)
			return OwnerDestroy;

		DBG_ASSERT(false);
		return Collision;
	}
};

struct ParticleFX_EventType
{
	enum Type
	{
		Spawn_Attached,
		Spawn,
		Play_Sound,
		Destroy,
		Limit_Neighbour_Radius,
		Script_Event,
		Script_Event_Global,

		COUNT
	};
	
	static const char* To_Name(ParticleFX_EventType::Type type)
	{
		static const char* g_names[] = {
			"Spawn Attached",
			"Spawn",
			"Play Sound",
			"Destroy",
			"Limit Neighbour Radius",
			"Script Event",
			"Script Event Global"
		};

		return g_names[(int)type];
	}

	static const char* To_String(ParticleFX_EventType::Type type)
	{
		static const char* g_names[] = {
			"Spawn_Attached",
			"Spawn",
			"Play_Sound",
			"Destroy",
			"Limit_Neighbour_Radius",
			"Script_Event",
			"Script_Event_Global"
		};

		return g_names[(int)type];
	}

	static ParticleFX_EventType::Type Parse(const char* value)
	{
		if (stricmp(value, "spawn_attached") == 0)
			return Spawn_Attached;
		else if (stricmp(value, "spawn") == 0)
			return Spawn;
		else if (stricmp(value, "play_sound") == 0)
			return Play_Sound;
		else if (stricmp(value, "destroy") == 0)
			return Destroy;
		else if (stricmp(value, "limit_neighbour_radius") == 0)
			return Limit_Neighbour_Radius;
		else if (stricmp(value, "script_event") == 0 || stricmp(value, "script event") == 0)
			return Script_Event;
		else if (stricmp(value, "script_event_global") == 0 || stricmp(value, "script event global") == 0)
			return Script_Event_Global;

		DBG_ASSERT(false);
		return Spawn_Attached;
	}
};

struct ParticleFX_CurvePoint
{
	float time;
	float value;

	ParticleFX_CurvePoint(float t, float v)
		: time(t)
		, value(v)
	{
	}

	ParticleFX_CurvePoint()
		: time(0)
		, value(0)
	{
	}
};

struct ParticleFX_Curve_Range
{
	float start, end;

	ParticleFX_Curve_Range()
		: start(0.0f)
		, end(0.0f)
	{
	}

	ParticleFX_Curve_Range(ParticleFX_Curve* curve)
	{
		Randomise(curve);
	}

	void Randomise(ParticleFX_Curve* curve);
};

struct ParticleFX_Curve
{
	enum
	{
		precalculated_accuracy = 64
	};

	std::vector<ParticleFX_CurvePoint>	points;
	float								start_min;
	float								start_max;
	float								end_min;
	float								end_max;

	bool								is_static;
	float								static_value;

	float								precalculated[precalculated_accuracy + 1];

	ParticleFX_Curve()
		: start_min(0)
		, start_max(0)
		, end_min(0)
		, end_max(0)
		, is_static(false)
	{
		points.push_back(ParticleFX_CurvePoint(0, 1));
		points.push_back(ParticleFX_CurvePoint(1, 0));

		Precalculate();
	}

	ParticleFX_Curve(float smin, float smax, float emin, float emax)
		: start_min(smin)
		, start_max(smax)
		, end_min(emin)
		, end_max(emax)
		, is_static(false)
	{
		points.push_back(ParticleFX_CurvePoint(0, 1));
		points.push_back(ParticleFX_CurvePoint(1, 0));

		Precalculate();
	}

	void Precalculate()
	{
		Precalculate_Curve(points, precalculated, precalculated_accuracy);
	}
	
	void Precalculate_Curve(std::vector<ParticleFX_CurvePoint> points, float* output, int output_size);

	INLINE float Get(float time, ParticleFX_Curve_Range& range)
	{
		if (is_static == true)
		{
			return range.start + ((range.end - range.start) * static_value);
		}

		const static float inv_precalc = 1.0f / precalculated_accuracy;

		int	start_index = (int)floorf(time * precalculated_accuracy);
		int	end_index   = (int)ceilf(time * precalculated_accuracy);

		float start_time = start_index * inv_precalc;
		float end_time = end_index * inv_precalc;

		float delta = (end_time - start_time) == 0 ? 0.0f : (time - start_time) / (end_time - start_time);
		float start	= precalculated[start_index];
		float end	= precalculated[end_index];

		float final_delta = start + ((end - start) * delta);

		return range.start + ((range.end - range.start) * final_delta);
	}
};

struct ParticleFX_Event
{
	float							time;
	ParticleFX_EventTrigger::Type	trigger;
	ParticleFX_EventType::Type		type;
	std::string						param;
	float							param_float;
	std::string						modifier;
	int								modifier_hash;
	float							probability;

	double							last_global_trigger;
};

struct ParticleFX_EmitterConfig
{
	enum
	{
		max_events = 10,
	};

	std::string							name;
	int									global_max_particles;
	ParticleFX_EmitterDrawMethod::Type	draw_method;
	ParticleFX_EmitterBlendMode::Type	blend_mode;

	bool								beam_use_seperate_tail_head;

	bool								lock_rotation_to_direction;

	bool								keep_effect_alive;

	std::string							render_pass;
	int									render_pass_hash;

	float								cycle_length;
	float								cycle_count;
	int									max_particles_per_cycle;

	bool								important;

	std::string							editor_sprite_name;
	AtlasAnimation*						sprite_anim;
	AtlasFrame*							sprite_frame;
	bool								affected_by_attractors;
	bool								move_based_on_direction;

	bool								destroy_on_spawn_collision;

	bool								is_projectile;

	bool								enable_collision;
	ParticleFX_CollisionResponse::Type	collision_response;
	float								collision_scale_x;
	float								collision_scale_y;
	float								collision_offset_x;
	float								collision_offset_y;
	float								collision_dir_offset;

	float								draw_offset_x;
	float								draw_offset_y;

	bool								enable_damage;
	bool								constant_damage;
	ParticleFX_DamageType::Type			damage_type;
	float								damage_radius;
	float								damage_offset_x;
	float								damage_offset_y;

	ParticleFX_SpawnShape::Type			spawn_shape;
	float								spawn_inner_radius;
	float								spawn_outer_radius;

	bool								offset_by_height;
	float								spawn_offset_x;
	float								spawn_offset_y;

	bool								scale_proportional;
	bool								add_emitter_direction;

	float								owner_trigger_delay_min;
	float								owner_trigger_delay_max;

	bool									collection_enabled;
	int										collection_value;
	ParticleFX_EmitterCollectionType::Type	collection_type;

	float								depth_bias;
	int									layer_offset;

	bool								track_effect;
	bool								draw_shadow;

	bool								invert_scale_on_bounce;

	int									current_particle_count;

	bool								allow_bouncing;

	std::string							editor_loop_sound;
	std::string							editor_penetration_sound;
	SoundHandle*						loop_sound;
	SoundHandle*						penetration_sound;		

	float								penetration_speed_scalar;

	ParticleFX_Curve					curves[ParticleFX_CurveType::COUNT];
	ParticleFX_Event					events[max_events];
	int									event_count;

	// Calculated on start.
	bool								can_cull; // Takes into account sub-effects.

	ParticleFX_EmitterConfig()
		: destroy_on_spawn_collision(true)
		, owner_trigger_delay_min(0.0f)
		, owner_trigger_delay_max(0.0f)
		, draw_offset_x(0.0f)
		, draw_offset_y(0.0f)
		, collision_dir_offset(0.0f)
		, is_projectile(false)
	{
	}
};

struct ParticleFX_AttractorConfig
{
	std::string						name;
	float							x_offset;
	float							y_offset;
	float							strength;
	float							radius;
	bool							track_effect;
};

class ParticleFX
{
	MEMORY_ALLOCATOR(ParticleFX, "UI");

private:	
	friend class ParticleManager;

	std::string								m_name;
	int										m_name_hash;
	//bool									m_persist_till_particles_dead;

	std::vector<ParticleFX_EmitterConfig>	m_emitters;
	std::vector<ParticleFX_AttractorConfig>	m_attractors;

protected:
	friend class ParticleFXFactory;
	friend class UIScene_PfxEditor;
	
	bool Load_Elements(ConfigFile* config);

	static ParticleFX* Load(const char* url);
	void Save(const char* url);

public:
	enum
	{
		max_emitters = 8,
		max_attractors = 8
	};

	ParticleFX();
	~ParticleFX();

	int Get_Emitter_Count()
	{
		return m_emitters.size();
	}
	ParticleFX_EmitterConfig* Get_Emitter(int index)
	{
		return &m_emitters.at(index);
	}

	int Get_Attractor_Count()
	{
		return m_attractors.size();
	}

	ParticleFX_AttractorConfig* Get_Attractor(int index)
	{
		return &m_attractors.at(index);
	}

	std::string Get_Name();
	int Get_Name_Hash();

};

#endif

