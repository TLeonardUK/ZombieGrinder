// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_PARTICLES_PARTICLEMANAGER_
#define _ENGINE_SCENE_PARTICLES_PARTICLEMANAGER_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Patterns/Singleton.h"
#include "Engine/Tasks/TaskManager.h"

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector3.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Generic/Threads/ConditionVariable.h"
#include "Generic/Threads/Semaphore.h"
#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"
#include "Generic/Threads/Atomic.h"
#include "Generic/Threads/ThreadSyncPoint.h"

#include "Engine/Renderer/Atlases/Atlas.h"

#include "Engine/Scene/Collision/CollisionHashTable.h"

#include "Engine/Particles/ParticleFX.h"

#include "Engine/Audio/Sounds/SoundChannel.h"

class ParticleFX;
class ParticleFXHandle;
struct ParticleEmitter;
struct ParticleAttractor;
struct ParticleInstance;
struct ParticleFX_EmitterConfig;
struct ParticleFX_AttractorConfig;
class Geometry;
class Texture;
class Camera;
struct CollisionHandle;
class CVMObject;

#define PARTICLE_GRAVITY						16.0f
#define PARTICLE_MIN_BOUNCE_VELOCITY			8.0f

#define PARTICLE_SHADOW_REDUCTION_MAX_HEIGHT	64.0f

//#define PARTICLE_USE_HALF_GEOMETRY

struct ParticleInstanceHandle;
struct ParticleInstigatorHandle;

struct ParticleUpgradeModifiers
{
	enum Type
	{
		// Base states.
		Damage,
		AmmoCapacity,
		RateOfFire,
		ReloadSpeed,
		ProjectileSpeed,
		Accuracy,
		Penetrating,
		Durability,

		// Specials
		TriBarrel,
		QuadBarrel,
		MultiTap,
		Suicidal,

		COUNT
	};
};

struct ParticleEffect
{
	int					incarnation;
	bool				is_alive;
	bool				was_simulated;
	
	int					demo_id;
	bool				demo_relevant;
	int					demo_instigator_team;

	ParticleInstanceHandle* owner;

	ParticleFX*			type;
	Vector3				position;
	Vector3				target;
	float				direction;
	double				start_time;
	double				last_frame_time;
	double				elapsed;

	bool				ignore_spawn_collision;

	bool				is_position_pending;
	bool				is_direction_pending;
	bool				is_target_pending;
	Vector3				pending_position;
	float				pending_direction;
	Vector3				pending_target;

	Vector3				last_position;
	bool				has_last_position;

	Vector3				m_demo_interp_position;
	float				m_demo_interp_direction;

	int					meta_number;
	int					modifier_hash;
	int					subtype;
	CVMObject*			weapon_type; // STAT_TODO

	bool				force_dispose;
	bool				force_dispose_started;
	bool				is_visible;
	bool				is_paused;

	ParticleInstigatorHandle*	instigator;
	int							instigator_incarnation;

	int					emitter_count;
	ParticleEmitter*	emitters[ParticleFX::max_emitters];

	int					attractor_count;
	ParticleAttractor*	attractors[ParticleFX::max_attractors];

	std::vector<float>	upgrade_modifiers;

//	int					emitter_start_index;
//	int					emitter_end_index;
//	int					attractor_start_index;
//	int					attractor_end_index;
};

struct ParticleEmitter
{
//	int							index;
	ParticleEffect*				effect;
//	int							effect_index;
	bool						is_alive_flag;

	int							demo_instigator_team;

	ParticleFX_EmitterConfig*	type;
	float						accumulated;
	float						life_elapsed;
	ParticleFX_Curve_Range		spawn_rate_range;
	float						spawns_accumulated;
	Vector3						last_spawn_position;
	float						last_spawn_direction;

	int							cycle_index;
	int							spawns_this_cycle;
};

struct ParticleAttractor
{
//	int							index;
	ParticleEffect*				effect;
//	int							effect_index;

	Vector3						spawn_position;
	float						spawn_direction;

	ParticleFX_AttractorConfig*	type;
};

struct ParticleSound
{
	int				incarnation;
	SoundChannel*	channel;
	bool			kill_flag;
};

struct ParticleSoundHandle
{
	ParticleManager*			manager;
	int							incarnation;
	ParticleSound*				sound;

	INLINE bool Is_Valid();
	INLINE void Dispose();
	INLINE void Update(const Vector3& pos);
};

struct ParticleInstance
{
	ParticleEmitter*			emitter_instance;
	ParticleFX_EmitterConfig*	emitter_type;
//	int						emitter_index;
	bool					is_alive_flag;
	float					last_update_delta;

	bool					invoke_game_trigger;
	float					invoke_game_trigger_delay;

	bool					velocity_affected_last_frame;

	bool					is_penetrating;
	bool					was_penetrating;

	int						demo_instigator_team;

	bool					is_owner;

	ParticleSoundHandle		sound_handle;
	SoundHandle*			last_sound;

	ParticleInstigatorHandle*	instigator;	
	int							instigator_incarnation;

	int						subtype;
	CVMObject*				weapon_type; // STAT_TODO

	int						incarnation;
	bool					is_alive;

	bool					neighbour_exists;

	Vector3					draw_spawn_position;
	Vector3					spawn_position;
	Vector3					last_spawn_position;
	float					spawn_direction;
	float					alive_time;

	bool					using_collision;
	Vector3					collision_last_center;
	float					collision_last_angle;

	float					lifetime;
	Vector3					position;
	Vector3					draw_position;
	float					direction;
	float					rotation;

	bool					bouncing;
	float					height;
	float					height_velocity;
	float					height_velocity_scale;

	int						spawned_in_collision;
	bool					checked_spawn_collision;
	bool					ignore_spawn_collision;

	int						collision_resolve_count;

	Vector2					velocity_scale;

	float					damage;
	int						ricochet_count;
	int						penetration_count;

	ParticleFX_Curve_Range	sprite_frame_range;
	ParticleFX_Curve_Range	velocity_x_range;
	ParticleFX_Curve_Range	velocity_y_range;
	ParticleFX_Curve_Range	velocity_height_range;
	ParticleFX_Curve_Range	velocity_rotation_range;
	ParticleFX_Curve_Range	velocity_direction_range;
	ParticleFX_Curve_Range	color_r_range;
	ParticleFX_Curve_Range	color_g_range;
	ParticleFX_Curve_Range	color_b_range;
	ParticleFX_Curve_Range	color_a_range;
	ParticleFX_Curve_Range	scale_x_range;
	ParticleFX_Curve_Range	scale_y_range;
};

struct ParticleInstanceHandle
{
	ParticleInstance* instance;
	int				  incarnation;
	int				  index;

	int m_reset_count;

	bool Is_Valid()
	{
		return instance != NULL && incarnation == instance->incarnation;
	}

	ParticleInstance* Get()
	{
		if (!Is_Valid())
		{
			return NULL;
		}
		return instance;
	}
};

class ParticleBuffer;

struct RenderParticle
{
	int					incarnation;
	ParticleBuffer*		buffer;
	AtlasFrame*			frame;
	float				x, y, z;
	float				angle;
	float				red, green, blue, alpha;
	float				scale_x, scale_y;
	bool				flip_h, flip_v;
	int					render_index;

	static bool Sort_Particle_Predicate(const RenderParticle* a, const RenderParticle* b)
	{
		/*		
		static const float Z_THRESHOLD = 0.1f;
		float diff = (a->z - b->z);
		return (diff > Z_THRESHOLD && diff <= -Z_THRESHOLD) ? a->particle_index < b->particle_index : a->z < b->z;
		*/
		return a->z == b->z ? a->incarnation < b->incarnation : a->z < b->z;
	}
};

struct ParticleEvent
{
	ParticleFX_Event* event;
	ParticleInstance* particle;
};

struct ParticleEventNeighbourMarker
{
	Vector3 position;
	ParticleFX_EmitterConfig* emitter_type;
};

struct ParticleCollisionRequest
{
	ParticleInstance*	particle;
	int					incarnation;
	Vector3				new_position;
	Vector3				old_position;
	Vector3				base_position;
	Rect2D				area;
	int					collision_resolve_count;
	bool				is_damage;
	bool				is_beam;
	Vector3				effect_position;
	Vector3				spawn_position;

	CollisionHandle*	handle;
};

class ParticleSimulationTask : public Task
{
private:
	int m_worker_index;

public:
	ParticleSimulationTask(int index);

	void Run();
};

class ParticleBuffer
{
public:
	void* Geometry_Vertices;
	int Geometry_Vertices_Offset;
	int Geometry_Particle_Stride;

	int Max_Particles;
	int Particle_Count;

	int Render_Vertex_Offset;
	int Render_Vertex_Count;

	int Required_Buffer_Count;

	int Render_Pass_Hash;
	Texture* Render_Texture;
	ParticleFX_EmitterBlendMode::Type Blend_Mode;

	INLINE void Write(int index, AtlasFrame* frame, float x, float y, float z, float angle, float r, float g, float b, float a, float scale_x, float scale_y, bool flip_h, bool flip_v)
	{
		// Only write particle if we have enough space in our geometry buffer.
		if (index < Max_Particles)
		{
#ifdef PARTICLE_USE_HALF_GEOMETRY
			s16* ptr = (s16*)((char*)Geometry_Vertices + (index * Geometry_Particle_Stride));
#else
			float* ptr = (float*)((char*)Geometry_Vertices + (index * Geometry_Particle_Stride));
#endif

			// No render space D:
			// Probably on a null renderer.
			if (ptr == NULL)
			{
				return;
			}

			// Transformation.
			float s = sinf(angle);
			float c = cosf(angle);

			if (flip_h == true)
			{
				s = -s;
			}

			if (flip_v == true)
			{
				c = -c;
			}

			float ix = c * scale_x;
			float iy =-s * scale_y;
			float jx = s * scale_x;
			float jy = c * scale_y;


			// Position
			float x0 = -frame->Origin.X;
			float y0 = -frame->Origin.Y;
			float x1 = -frame->Origin.X + frame->Rect.Width;
			float y1 = -frame->Origin.Y + frame->Rect.Height;
			
			float tx = x;
			float ty = y;

			// Calculate UV's (with V inverted) TODO: Sort this shit out, we do this in a bunch of places and its opengl specific D:
#ifdef PARTICLE_USE_HALF_GEOMETRY
			s16 uv_left		= Math::Float_To_Half(frame->UV.X);
			s16 uv_right	= Math::Float_To_Half(frame->UV.X + frame->UV.Width);
			s16 uv_top		= Math::Float_To_Half(1.0f - frame->UV.Y);
			s16 uv_bottom	= Math::Float_To_Half(uv_top - frame->UV.Height);
		
			s16 c_r = Math::Float_To_Half(r);
			s16 c_g = Math::Float_To_Half(g);
			s16 c_b = Math::Float_To_Half(b);
			s16 c_a = Math::Float_To_Half(a);

			s16 c_z = Math::Float_To_Half(z)
				
			s16 tl_x = Math::Float_To_Half(x0 * ix + y0 * iy + tx);
			s16 tl_y = Math::Float_To_Half(x0 * jx + y0 * jy + ty);

			s16 bl_x = Math::Float_To_Half(x0 * ix + y1 * iy + tx);		
			s16 bl_y = Math::Float_To_Half(x0 * jx + y1 * jy + ty);

			s16 br_x = Math::Float_To_Half(x1 * ix + y1 * iy + tx);		
			s16 br_y = Math::Float_To_Half(x1 * jx + y1 * jy + ty);

			s16 tr_x = Math::Float_To_Half(x1 * ix + y0 * iy + tx);	
			s16 tr_y = Math::Float_To_Half(x1 * jx + y0 * jy + ty);
#else
			float uv_left	= frame->UV.X;
			float uv_right	= frame->UV.X + frame->UV.Width;
			float uv_top	= 1.0f - frame->UV.Y;
			float uv_bottom	= uv_top - frame->UV.Height;
		
			float c_r = r;
			float c_g = g;
			float c_b = b;
			float c_a = a;

			float c_z = z;
				
			float tl_x = x0 * ix + y0 * iy + tx;
			float tl_y = x0 * jx + y0 * jy + ty;

			float bl_x = x0 * ix + y1 * iy + tx;		
			float bl_y = x0 * jx + y1 * jy + ty;

			float br_x = x1 * ix + y1 * iy + tx;		
			float br_y = x1 * jx + y1 * jy + ty;

			float tr_x = x1 * ix + y0 * iy + tx;	
			float tr_y = x1 * jx + y0 * jy + ty;
#endif

			// Top-Left
			*(ptr++) = tl_x;		*(ptr++) = tl_y;		*(ptr++) = c_z;
			*(ptr++) = uv_left;		*(ptr++) = uv_top;
			*(ptr++) = c_r;			*(ptr++) = c_g;			*(ptr++) = c_b;		*(ptr++) = c_a;
			
			// Bottom-Left
			*(ptr++) = bl_x;		*(ptr++) = bl_y;		*(ptr++) = c_z;
			*(ptr++) = uv_left;		*(ptr++) = uv_bottom;
			*(ptr++) = c_r;			*(ptr++) = c_g;			*(ptr++) = c_b;		*(ptr++) = c_a;

			// Bottom-Right
			*(ptr++) = br_x;		*(ptr++) = br_y;		*(ptr++) = c_z;
			*(ptr++) = uv_right;	*(ptr++) = uv_bottom;
			*(ptr++) = c_r;			*(ptr++) = c_g;			*(ptr++) = c_b;		*(ptr++) = c_a;
			
			// Top-Right
			*(ptr++) = tr_x;		*(ptr++) = tr_y;		*(ptr++) = c_z;
			*(ptr++) = uv_right;	*(ptr++) = uv_top;
			*(ptr++) = c_r;			*(ptr++) = c_g;			*(ptr++) = c_b;		*(ptr++) = c_a;
		}
		else
		{  
		//	Dam shadows :(
		//	DBG_ASSERT(Max_Particles >= 0);
		//	DBG_LOG("Ran out of particle render space (%i/%i)! Spawn less plz :(", index, Max_Particles);
		}
	}

};

struct ParticleEffectHandle
{
private:
	friend class ParticleManager;

	ParticleEffect* m_effect;
	int				m_incarnation;

public:
	ParticleEffectHandle()
		: m_effect(NULL)
		, m_incarnation(-1)
	{
	}

	ParticleEffectHandle(ParticleEffect* effect, int incarnation)
		: m_effect(effect)
		, m_incarnation(incarnation)
	{
	}

	bool Is_Finished()
	{
		return m_effect == NULL || m_effect->incarnation != m_incarnation || m_effect->is_alive == false;
	}

	void Set_Position(Vector3 pos)
	{
		if (!Is_Finished())
		{
			m_effect->pending_position = pos;
			m_effect->is_position_pending = true;
		}
	}

	void Set_Meta_Number(int value)
	{
		if (!Is_Finished())
		{
			m_effect->meta_number = value;
		}
	}

	void Set_Sub_Type(int value)
	{
		if (!Is_Finished())
		{
			m_effect->subtype = value;
		}
	}

	// STAT_TODO
	void Set_Weapon_Type(CVMObject* value)
	{
		if (!Is_Finished())
		{
			m_effect->weapon_type = value;
		}

	}
	
	void Set_Upgrade_Modifiers(std::vector<float> value)
	{
		if (!Is_Finished())
		{
			m_effect->upgrade_modifiers = value;
		}
	}

	void Set_Modifier(std::string value)
	{
		if (!Is_Finished())
		{
			m_effect->modifier_hash = StringHelper::Hash(value.c_str());
		}
	}

	void Set_Target(Vector3 pos)
	{
		if (!Is_Finished())
		{
			m_effect->pending_target = pos;
			m_effect->is_target_pending = true;
		}
	}

	void Set_Direction(float dir)
	{
		if (!Is_Finished())
		{
			m_effect->pending_direction = dir;
			m_effect->is_direction_pending = true;
		}
	}

	void Set_Instigator(ParticleInstigatorHandle* instigator);

	void Dispose()
	{
		if (!Is_Finished())
		{
			m_effect->force_dispose = true;
		}
	}

	void Set_Paused(bool val)
	{
		if (!Is_Finished())
		{
			m_effect->is_paused = val;
		}
	}
	
	void Set_Ignore_Spawn_Collision(bool val)
	{
		if (!Is_Finished())
		{
			m_effect->ignore_spawn_collision = val;
		}
	}

	void Set_Visible(bool val)
	{
		if (!Is_Finished())
		{
			m_effect->is_visible = val;
		}
	}

};

struct ParticleCollector
{
	int					demo_id;

	int					incarnation;

	bool				force_dispose;
	bool				is_paused;

	Vector3				m_demo_interp_position;

	Vector3				position;
	float				radius;
	float				strength;

	int					read_index;
	int					collection_count[2][ParticleFX_EmitterCollectionType::COUNT];
};

struct ParticleCollectorHandle
{
private:
	friend class ParticleManager;

	ParticleCollector*	m_collector;
	int					m_incarnation;

public:
	ParticleCollectorHandle()
		: m_collector(NULL)
		, m_incarnation(-1)
	{
	}

	ParticleCollectorHandle(ParticleCollector* effect, int incarnation)
		: m_collector(effect)
		, m_incarnation(incarnation)
	{
	}

	bool Is_Finished()
	{
		return m_collector == NULL || m_collector->incarnation != m_incarnation;
	}

	void Set_Position(Vector3 pos)
	{
		if (!Is_Finished())
		{
			m_collector->position = pos;
		}
	}

	void Set_Paused(bool val)
	{
		if (!Is_Finished())
		{
			m_collector->is_paused = val;
		}
	}

	void Dispose()
	{
		if (!Is_Finished())
		{
			m_collector->force_dispose = true;
		}
	}

	int Get_Collected(ParticleFX_EmitterCollectionType::Type type)
	{
		if (!Is_Finished())
		{
			return m_collector->collection_count[m_collector->read_index][(int)type];
		}
		return 0;
	}

};

struct ParticleInstigatorHandle
{
private:
	friend class ParticleManager;

	void* m_instigator;
	int	  m_incarnation;
	int	  m_index;

	int m_reset_count;

public:
	ParticleInstigatorHandle()
		: m_instigator(NULL)
		, m_incarnation(-1)
		, m_index(-1)
		, m_reset_count(0)
	{
	}

	int Get_Incarnation()
	{
		return m_incarnation;
	}

	void* Get_Instigator()
	{
		return m_instigator;
	}

};

class ParticleManager : 
	public Singleton<ParticleManager>
{
	MEMORY_ALLOCATOR(ParticleManager, "Scene");

private:
	enum
	{
		max_particles						= 32 * 1000,
		max_render_particles				= max_particles,
		max_effects							= 8 * 1000,
		max_emitters						= 8 * 1000,
		max_collectors						= 256,
		max_attractors						= 2 * 1000,
		max_events_per_frame				= 1 * 1000,
		max_collision_requests_per_frame	= 1 * 1000,
		max_particle_spawns_per_frame		= 1 * 1000,
		max_instance_handles				= 4 * 1024,
		max_instigator_handles				= 1 * 1024,
		max_sounds							= 100,
		max_workers							= 7,
		geometry_buffer_count				= 2,
		buffer_count						= 16
	};

	int m_demo_id_tracker;

	ParticleInstigatorHandle m_instigator_handles_mem[max_instigator_handles];
	ParticleInstigatorHandle* m_instigator_handles[max_instigator_handles];
	int m_instigator_handle_count;

	ParticleInstanceHandle m_instance_handles_mem[max_instance_handles];
	ParticleInstanceHandle* m_instance_handles[max_instance_handles];
	int m_instance_handle_count;

	TaskID m_worker_ids[max_workers];
	ParticleSimulationTask* m_workers[max_workers];
	int m_worker_count;

	ParticleEffect m_effects_mem[max_effects];
	ParticleEffect* m_effects[max_effects];
	int m_effect_count;
	
	ParticleEmitter m_emitters_mem[max_emitters];
	ParticleEmitter* m_emitters[max_emitters];
	int m_emitter_count;

	ParticleCollector m_collectors_mem[max_collectors];
	ParticleCollector* m_collectors[max_collectors];
	int m_collector_count;

	ParticleAttractor m_attractors_mem[max_attractors];
	ParticleAttractor* m_attractors[max_attractors];
	int m_attractor_count;
	
	ParticleSound m_sounds_mem[max_sounds];
	ParticleSound* m_sounds[max_sounds];
	int m_sound_count;

	RenderParticle m_render_particle_buffer[max_particles];
	RenderParticle* m_render_particle_buffer_sorted[max_particles];				// BLLLLLLLLLLLLLLARH
	RenderParticle* m_render_particle_buffer_final_sorted[max_particles];				// BLLLLLLLLLLLLLLARH
	int  m_render_particle_count;

//	int m_render_simulate_workers_finished;
//	Semaphore* m_render_simulate_workers_semaphore;

//	int m_render_sort_workers_finished;
//	Semaphore* m_render_sort_workers_semaphore;

	ThreadSyncPoint* m_sort_blocker;
	ThreadSyncPoint* m_simulate_blocker;

	ParticleInstance m_particles[max_particles];
	int m_particle_count;
	
	Geometry* m_geometry_buffers[geometry_buffer_count];
	int m_geometry_write_buffer_index;

	Geometry* m_current_geometry_write_buffer;
	void* m_write_geometry_buffer;
	int m_current_geometry_write_buffer_size;

	ParticleBuffer m_buffers[buffer_count];
	int m_buffer_count;

	ParticleEvent m_simulation_events[max_events_per_frame];
	int m_simulation_event_count;

	ParticleCollisionRequest m_collision_requests[max_collision_requests_per_frame];
	int m_simulation_collision_request_count;
	
	ParticleCollisionRequest m_pending_collision_requests[max_collision_requests_per_frame];
	int m_pending_collision_request_count;

	int m_incarnation_index;

	int m_simulate_collector_count;
	int m_simulate_effect_count;
	int m_simulate_particle_count;
	float m_simulate_delta_t;
	float m_simulate_elapsed_time;
	bool m_final_sort_complete;
	Mutex* m_mutex;

	int m_reset_count;

//	int m_particles_simulated;
//	int m_workers_run;
//	int m_particles_rendered;

	int m_simulation_collision_offset;
	int m_simulation_collision_modulus;
	int m_last_collision_count_requested;

	Rect2D m_simulation_camera_bounding_box;
	Rect2D m_simulation_camera_cull_bounding_box;

	Camera* m_render_camera;

	Random m_rand;

	bool m_draw_debug;

	int m_skip_modulus;
	int m_skip_modulus_counter;

	std::vector<int> m_spawned_demo_ids;
	std::vector<int> m_spawned_collector_ids;

protected:
	friend class ParticleSimulationTask;
	friend struct ParticleSoundHandle;

	void Simulate_Particle(float time, float delta, ParticleInstance* effect, int particle_index, int worker_index);
	void Simulate_Effect(float time, ParticleEffect* effect);
	void Simulate(int worker_index);

	INLINE void Write_Render_Particle(
		int incarnation,
		ParticleBuffer* buffer, 
		AtlasFrame* frame, 
		float draw_pos_x, 
		float draw_pos_y, 
		float draw_pos_z, 
		float draw_dir, 
		float red,
		float green, 
		float blue, 
		float alpha, 
		float scale_x, 
		float scale_y,
		bool flip_h,
		bool flip_v)
	{
		int index = Atomic::Increment32(&m_render_particle_count) - 1;
		if (index < max_render_particles)
		{
			RenderParticle& p = m_render_particle_buffer[index];
			p.incarnation = incarnation;
			p.buffer = buffer;
			p.frame = frame;
			p.x = draw_pos_x;
			p.y = draw_pos_y;
			p.z = draw_pos_z;
			p.angle = draw_dir;
			p.red = red;
			p.green = green;
			p.blue = blue;
			p.alpha = alpha;
			p.scale_x = scale_x;
			p.scale_y = scale_y;
			p.flip_h = flip_h;
			p.flip_v = flip_v;
		}
	}

	INLINE ParticleBuffer* Get_Particle_Buffer(Texture* texture, int render_pass, ParticleFX_EmitterBlendMode::Type blend_mode)
	{
		for (int i = 0; i < m_buffer_count; i++)
		{
			if (m_buffers[i].Render_Texture == texture &&
				m_buffers[i].Render_Pass_Hash == render_pass &&
				m_buffers[i].Blend_Mode == blend_mode)
			{
				return &m_buffers[i];
			}
		}

		int new_index = Atomic::Increment32(&m_buffer_count) - 1;
		DBG_ASSERT(new_index < buffer_count);

		m_buffers[new_index].Render_Texture = texture;
		m_buffers[new_index].Render_Pass_Hash = render_pass;
		m_buffers[new_index].Blend_Mode = blend_mode;
		m_buffers[new_index].Geometry_Vertices_Offset = 0;
		m_buffers[new_index].Geometry_Particle_Stride = 0;
		m_buffers[new_index].Max_Particles = 0;
		m_buffers[new_index].Particle_Count = 0;
		m_buffers[new_index].Render_Vertex_Offset = 0;
		m_buffers[new_index].Render_Vertex_Count = 0;
		m_buffers[new_index].Required_Buffer_Count = 0;

		return &m_buffers[new_index];
	}

	void Run_Event(ParticleEvent& evt);

	float Get_Draw_Depth(float pos);

	void Reset_Particle_Sounds();
	ParticleSoundHandle Alloc_Particle_Sound(SoundHandle* handle);

	bool Can_Cull(ParticleFX_EmitterConfig* pfx);

public:
	ParticleManager();
	~ParticleManager();

	bool Init();

	void Reset_For_Demo();
	void Reset();

	void Set_Draw_Debug(bool value);

	ParticleCollector* Find_Collector_By_Demo_ID(int demo_id);
	ParticleEffect* Find_Effect_By_Demo_Id(int demo_id);
	void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta, bool bScrubbing);

	// STAT_TODO 
	ParticleEffectHandle Spawn(const char* name, Vector3 position, float direction, int meta_number = 0, std::string modifier = "", int subtype = 0, CVMObject* weapon_type = NULL, std::vector<float> upgrade_modifiers = std::vector<float>());
	ParticleEffectHandle Spawn(ParticleFX* handle, Vector3 position, float direction, int meta_number = 0, std::string modifier = "", int subtype = 0, CVMObject* weapon_type = NULL, std::vector<float> upgrade_modifiers = std::vector<float>());
	ParticleEffectHandle Spawn(ParticleFXHandle* handle, Vector3 position, float direction, int meta_number = 0, std::string modifier = "", int subtype = 0, CVMObject* weapon_type = NULL, std::vector<float> upgrade_modifiers = std::vector<float>());
	ParticleEffectHandle Spawn(ParticleFX* handle, Vector3 position, float direction, int meta_number = 0, int modifier_hash = 0, int subtype = 0, CVMObject* weapon_type = NULL, std::vector<float> upgrade_modifiers = std::vector<float>());

	ParticleCollectorHandle Create_Collector(Vector3 position, float radius, float strength);

	ParticleInstanceHandle* Alloc_Particle_Instance_Handle(ParticleInstance* instance);
	void Free_Particle_Instance_Handle(ParticleInstanceHandle* handle);
	void Update_Particle_Instance_Handle(ParticleInstance* old_location, ParticleInstance* new_location);

	ParticleInstigatorHandle* Alloc_Instigator_Handle(void* instigator);
	void Free_Instigator_Handle(ParticleInstigatorHandle* handle);

	void Invoke_Game_Trigger(ParticleInstigatorHandle* instigator);

	void Set_Skip_Modulus(int mod);

	int Get_Particle_Count();
	int Get_Effect_Count();
	int Get_Emitter_Count();
	int Get_Attractor_Count();
	int Get_Collector_Count();

	void Tick(const FrameTime& time);
	void Draw_Pass(const FrameTime& time, std::string pass_name);

	void Update_Demo_Properties(ParticleEffect* effect);

};

#endif

