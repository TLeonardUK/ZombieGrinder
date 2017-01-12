// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Particles/ParticleManager.h"
#include "Engine/Particles/ParticleFX.h"
#include "Engine/Particles/ParticleFXHandle.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/GameRunner.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Renderer/Shaders/ShaderProgram.h"
#include "Engine/Renderer/Shaders/ShaderProgramHandle.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/RenderPipelineTypes.h"
#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Collision/CollisionManager.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Scene/Map/Map.h"

#include "Generic/Math/Random.h"
#include "Generic/Threads/Atomic.h"

#include "Generic/Stats/Stats.h"

#include "Generic/Helper/CollisionHelper.h"

#include "Generic/Threads/ThreadSyncPoint.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include "Engine/IO/BinaryStream.h"

#include <algorithm>

DEFINE_FRAME_STATISTIC("Particles/Particle Count", int, g_particle_active_particle_count, true);
DEFINE_FRAME_STATISTIC("Particles/Effect Count", int, g_particle_active_effect_count, true);
DEFINE_FRAME_STATISTIC("Particles/Stall Time", float, g_particle_stall_time, true);
DEFINE_FRAME_STATISTIC("Particles/Event Time", float, g_particle_event_time, false);
DEFINE_FRAME_STATISTIC("Particles/Collision Time", float, g_particle_collision_time, false);
DEFINE_FRAME_STATISTIC("Particles/Culling Time", float, g_particle_culling_time, false);
DEFINE_FRAME_STATISTIC("Particles/Geometry Upload Time", float, g_particle_geom_update_time, true);

void ParticleEffectHandle::Set_Instigator(ParticleInstigatorHandle* instigator)
{
	if (!Is_Finished())
	{
		m_effect->instigator = instigator;
		if (instigator != NULL)
		{
			m_effect->instigator_incarnation = instigator->Get_Incarnation();
		}				
		ParticleManager::Get()->Update_Demo_Properties(m_effect);
	}
}

ParticleManager::ParticleManager()
	: m_effect_count(0)
	, m_collector_count(0)
	, m_simulate_effect_count(0)
	, m_simulate_particle_count(0)
	, m_geometry_write_buffer_index(0)
	, m_buffer_count(0)
	, m_current_geometry_write_buffer(NULL)
	, m_write_geometry_buffer(NULL)
	, m_current_geometry_write_buffer_size(0)
	, m_incarnation_index(0)
	, m_draw_debug(false)
	, m_simulation_collision_offset(0)
	, m_simulation_collision_modulus(0)
	, m_last_collision_count_requested(0)
	, m_render_camera(NULL)
	, m_final_sort_complete(false)
	, m_rand((int)Platform::Get()->Get_Ticks())
	, m_skip_modulus(1)
	, m_skip_modulus_counter(0)
	, m_reset_count(0)
	, m_demo_id_tracker(0)
//	, m_workers_run(0)
//	, m_particles_simulated(0)
//	, m_particles_rendered(0)
{
	DBG_LOG("sizeof(ParticleManager) = %i kb", sizeof(ParticleManager) / 1024);

	m_worker_count = Min(max_workers, TaskManager::Get()->Get_Worker_Count());

	for (int i = 0; i < m_worker_count; i++)
	{
		m_workers[i] = new ParticleSimulationTask(i);
		m_worker_ids[i] = -1;
	}

	for (int i = 0; i < geometry_buffer_count; i++)
	{
		m_geometry_buffers[i] = NULL;
	}

	for (int i = 0; i < max_sounds; i++)
	{
		memset(&m_sounds_mem[i], 0, sizeof(ParticleSound));
	}

	m_mutex = Mutex::Create();
	m_sort_blocker = new ThreadSyncPoint();
	m_simulate_blocker = new ThreadSyncPoint();

	Reset();
}

void ParticleManager::Set_Draw_Debug(bool value)
{
	m_draw_debug = value;
}

int ParticleManager::Get_Particle_Count()
{
	return m_particle_count;
}

int ParticleManager::Get_Effect_Count()
{
	return m_effect_count;
}

int ParticleManager::Get_Emitter_Count()
{
	return m_emitter_count;
}

int ParticleManager::Get_Attractor_Count()
{
	return m_attractor_count;
}

int ParticleManager::Get_Collector_Count()
{
	return m_collector_count;
}

void ParticleManager::Set_Skip_Modulus(int mod)
{
	m_skip_modulus = mod;
}

ParticleManager::~ParticleManager()
{
	for (int i = 0; i < geometry_buffer_count; i++)
	{
		SAFE_DELETE(m_geometry_buffers[i]);
	}
	for (int i = 0; i < m_worker_count; i++)
	{
		SAFE_DELETE(m_workers[i]);
	}
	SAFE_DELETE(m_mutex);
}

ParticleEffect* ParticleManager::Find_Effect_By_Demo_Id(int demo_id)
{
	for (int i = 0; i < m_effect_count; i++)
	{
		ParticleEffect* effect = m_effects[i];
		if (effect->demo_id == demo_id && effect->is_alive)
		{
			return effect;
		}
	}
	return NULL;
}

ParticleCollector* ParticleManager::Find_Collector_By_Demo_ID(int demo_id)
{
	for (int i = 0; i < m_collector_count; i++)
	{
		ParticleCollector* effect = m_collectors[i];
		if (effect->demo_id == demo_id)
		{
			return effect;
		}
	}
	return NULL;
}

void ParticleManager::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta, bool bScrubbing)
{
	bool bApplySettings = (frameDelta == 1.0f) && !bScrubbing;

	int total_read = 0;

	// Serialize effects.
	for (int i = 0; ; i++)
	{
		if (bSaving)
		{
			if (i >= m_effect_count)
			{
				break;
			}
		}
		else
		{
			bool bMore = stream->Read<bool>();
			if (!bMore)
			{
				break;
			}
		}

		ParticleEffect* effect = !bSaving ? NULL : m_effects[i];
		if (bSaving && !effect->demo_relevant)
		{
			continue;
		}

		if (bSaving)
		{
			stream->Write<bool>(true);
		}

		int demo_id;
		int fx_hash;
		Vector3 position;

		float direction;
		int meta_number;
		int modifier_hash;
		int subtype;

		bool is_paused;
		bool is_visible;
		//bool is_alive;

		bool force_dispose;

		int demo_instigator_team;

		if (effect)
		{
			demo_id = effect->demo_id;
			fx_hash = effect->type->Get_Name_Hash();
			position = effect->position;
			direction = effect->direction;
			meta_number = effect->meta_number;
			modifier_hash = effect->modifier_hash;
			subtype = effect->subtype;
			is_paused = effect->is_paused;
			is_visible = effect->is_visible;
		//	is_alive = effect->is_alive;
			force_dispose = effect->force_dispose;
			demo_instigator_team = effect->demo_instigator_team;
		}

		stream->Serialize<int>(bSaving, demo_id);	
		stream->Serialize<int>(bSaving, fx_hash);
		stream->Serialize<float>(bSaving, position.X);
		stream->Serialize<float>(bSaving, position.Y);
		stream->Serialize<float>(bSaving, position.Z);
		stream->Serialize<float>(bSaving, direction);
		stream->Serialize<int>(bSaving, meta_number);
		stream->Serialize<int>(bSaving, modifier_hash);
		stream->Serialize<int>(bSaving, subtype);
		stream->Serialize<bool>(bSaving, is_paused);
		stream->Serialize<bool>(bSaving, is_visible);
		//stream->Serialize<bool>(bSaving, is_alive);
		stream->Serialize<bool>(bSaving, force_dispose);
		stream->Serialize<int>(bSaving, demo_instigator_team);

		// Add any new effects.
		if (!bSaving && bApplySettings)
		{			
			effect = Find_Effect_By_Demo_Id(demo_id);
			if (effect == NULL)
			{
				if (std::find(m_spawned_demo_ids.begin(), m_spawned_demo_ids.end(), demo_id) == m_spawned_demo_ids.end())
				{
					ParticleFXHandle* pfx = ResourceFactory::Get()->Get_ParticleFX(fx_hash);
					if (pfx)
					{
					///	DBG_LOG("[Demo] Found new effect %i, spawning '%s'.", demo_id, pfx->Get()->Get_Name().c_str());

						effect = Spawn(pfx->Get(), position, direction, meta_number, modifier_hash, subtype, NULL).m_effect;
						effect->demo_id = demo_id;
						effect->demo_relevant = true;
						m_spawned_demo_ids.push_back(demo_id);
					}
				}
			}
		}

		// Interpolate positions etc!
		if (!bSaving && effect)
		{
			// Lerp position & direction.
			if (frameDelta == 0.0f)
			{
				effect->m_demo_interp_position = effect->position;
				effect->m_demo_interp_direction = effect->direction;
			}

			effect->position.X = Math::Lerp(effect->m_demo_interp_position.X, position.X, frameDelta);
			effect->position.Y = Math::Lerp(effect->m_demo_interp_position.Y, position.Y, frameDelta);
			effect->position.Z = Math::Lerp(effect->m_demo_interp_position.Z, position.Z, frameDelta);
			effect->direction  = Math::Lerp(effect->m_demo_interp_direction,   direction, frameDelta);

			if (effect->type->Get_Name() == "love_thrower_fire")
			{
			//	DBG_LOG("[%i] Updating %s @ %f,%f - %f,%f - %f,%f", effect->type->Get_Name().c_str(), effect->m_demo_interp_position.X, effect->m_demo_interp_position.Y, position.X, position.Y, effect->position.X, effect->position.Y);
			}

			effect->is_paused = is_paused;
			effect->is_visible = is_visible;
		//	effect->is_alive = is_alive;
			effect->force_dispose = force_dispose;
			effect->demo_instigator_team = demo_instigator_team;

			for (int i = 0; i < effect->emitter_count; i++)
			{
				effect->emitters[i]->demo_instigator_team = demo_instigator_team;
			}
		}

		total_read++;
	}

	if (bSaving)
	{
		stream->Write<bool>(false);
	}

	// Serialize attractors.
	for (int i = 0; ; i++)
	{
		if (bSaving)
		{
			if (i > m_collector_count)
			{
				break;
			}
		}
		else
		{
			bool bMore = stream->Read<bool>();
			if (!bMore)
			{
				break;
			}
		}

		ParticleCollector* effect = !bSaving ? NULL : m_collectors[i];
		if (bSaving)
		{
			stream->Write<bool>(true);
		}

		int demo_id;
		bool is_paused;
		Vector3 position;
		float radius;
		float strength;

		if (effect)
		{
			demo_id = effect->demo_id;
			position = effect->position;
			radius = effect->radius;
			strength = effect->strength;
			is_paused = effect->is_paused;
		}

		stream->Serialize<int>(bSaving, demo_id);
		stream->Serialize<float>(bSaving, position.X);
		stream->Serialize<float>(bSaving, position.Y);
		stream->Serialize<float>(bSaving, position.Z);
		stream->Serialize<float>(bSaving, radius);
		stream->Serialize<float>(bSaving, strength);
		stream->Serialize<bool>(bSaving, is_paused);

		// Add any new effects.
		if (!bSaving && bApplySettings)
		{
			effect = Find_Collector_By_Demo_ID(demo_id);
			if (effect == NULL)
			{
				if (std::find(m_spawned_collector_ids.begin(), m_spawned_collector_ids.end(), demo_id) == m_spawned_collector_ids.end())
				{
					DBG_LOG("[Demo] Found new collector %i, spawning.", demo_id);

					effect = Create_Collector(position, radius, strength).m_collector;
					effect->demo_id = demo_id;
				}
			}
		}

		// Interpolate positions etc!
		if (!bSaving && effect)
		{
			// Lerp position & direction.
			if (frameDelta == 0.0f)
			{
				effect->m_demo_interp_position = effect->position;
			}

			effect->position.X = Math::Lerp(effect->m_demo_interp_position.X, position.X, frameDelta);
			effect->position.Y = Math::Lerp(effect->m_demo_interp_position.Y, position.Y, frameDelta);
			effect->position.Z = Math::Lerp(effect->m_demo_interp_position.Z, position.Z, frameDelta);
			effect->is_paused = is_paused;
			effect->strength = strength;
			effect->radius = radius;
		}
	}

	if (bSaving)
	{
		stream->Write<bool>(false);
	}
}

bool ParticleManager::Init()
{	
	GeometryBufferFormat format;
#ifdef PARTICLE_USE_HALF_GEOMETRY
	format.Add(GeometryBufferElementType::Position, GeometryBufferDataType::Half3);
	format.Add(GeometryBufferElementType::TexCoord, GeometryBufferDataType::Half2);
	format.Add(GeometryBufferElementType::Color,    GeometryBufferDataType::Half4);
#else
	format.Add(GeometryBufferElementType::Position, GeometryBufferDataType::Float3);
	format.Add(GeometryBufferElementType::TexCoord, GeometryBufferDataType::Float2);
	format.Add(GeometryBufferElementType::Color,    GeometryBufferDataType::Float4);
#endif

	// Allocate a big ass buffer than can store all our particles in it. Each
	// emitter will carve off parts of this each frame.
	for (int i = 0; i < geometry_buffer_count; i++)
	{
		m_geometry_buffers[i] = Renderer::Get()->Create_Geometry(format, PrimitiveType::Quad, max_particles, true);
	}

	// Calculate damage types for effects.
	DBG_LOG("[Particle FX Culling Table]");

	ThreadSafeHashTable<ParticleFXHandle*, unsigned int>& pfx = ResourceFactory::Get()->Get_ParticleFXs();
	for (ThreadSafeHashTable<ParticleFXHandle*, unsigned int>::Iterator iter = pfx.Begin(); iter != pfx.End(); iter++)
	{
		ParticleFXHandle* handle = *iter;
		ParticleFX* effect = handle->Get();
	
		for (int i = 0; i < effect->Get_Emitter_Count(); i++)
		{
			ParticleFX_EmitterConfig* config = effect->Get_Emitter(i);
			config->can_cull = Can_Cull(config);

			DBG_LOG("\t[%s:%s] %s", effect->Get_Name().c_str(), config->name.c_str(), config->can_cull ? "Can Cull" : "Can NOT Cull");
		}
	}

	return true;
}

bool ParticleManager::Can_Cull(ParticleFX_EmitterConfig* pfx)
{
	// Damage causing is a no-no.
	if (pfx->enable_damage)
	{
		return false;
	}

	// Important effects are a no-no.
	if (pfx->important == true)
	{
		return false;
	}

	// Cannot cull collectibles.
	if (pfx->collection_enabled == true)
	{
		return false;
	}

	// Check children.
	for (int i = 0; i < pfx->event_count; i++)
	{
		ParticleFX_Event& evt = pfx->events[i];
		switch (evt.type)
		{
		// Check if sub-particles can be culled.
		case ParticleFX_EventType::Spawn_Attached:
		case ParticleFX_EventType::Spawn:
			{
				ParticleFXHandle* sub_pfx_handle = ResourceFactory::Get()->Get_ParticleFX(evt.param.c_str());
				ParticleFX* sub_pfx = sub_pfx_handle->Get();

				for (int j = 0; j < sub_pfx->Get_Emitter_Count(); j++)
				{
					ParticleFX_EmitterConfig* config = sub_pfx->Get_Emitter(j);
					if (!Can_Cull(config))
					{
						return false;
					}
				}

				break;
			}

		// Never cull script event causing particles.
		case ParticleFX_EventType::Script_Event:
		case ParticleFX_EventType::Script_Event_Global:
			{
				return false;
			}
		}
	}

	return true;
}

void ParticleManager::Invoke_Game_Trigger(ParticleInstigatorHandle* instigator)
{
	if (instigator == NULL)
	{
		return;
	}
	
	// SLOW TODO: Oh jesus christ fix this.

	for (int i = 0; i < max_particles; i++)
	{
		if (m_particles[i].instigator == instigator &&
			m_particles[i].instigator_incarnation == instigator->m_incarnation)
		{
			m_particles[i].invoke_game_trigger = true;
			m_particles[i].invoke_game_trigger_delay = (float)Random::Static_Next_Double(m_particles[i].emitter_type->owner_trigger_delay_min, m_particles[i].emitter_type->owner_trigger_delay_max);
		}
	}
}

void ParticleManager::Reset()
{
	TaskManager* manager = TaskManager::Try_Get();
	if (manager != NULL)
	{
		// Wait until simulation tasks are finished.
		for (int i = 0; i < m_worker_count; i++)
		{
			if (m_worker_ids[i] >= 0)
			{
				manager->Wait_For(m_worker_ids[i]);
				m_worker_ids[i] = -1;
			}
		}
	}

	m_reset_count++;
	m_simulation_event_count = 0;
	m_simulate_collector_count = 0;
	m_pending_collision_request_count = 0;
	m_simulation_collision_request_count = 0;
	m_buffer_count = 0;
	m_attractor_count = 0;
	m_emitter_count = 0;
	m_effect_count = 0;
	m_particle_count = 0;
	m_collector_count = 0;
	m_simulate_effect_count = 0;
	m_simulate_particle_count = 0;
	m_simulate_effect_count = 0;
	m_final_sort_complete = false;
	m_instance_handle_count = 0;
	m_instigator_handle_count = 0;

	for (int i = 0; i < max_effects; i++)
		m_effects[i] = &m_effects_mem[i];
	for (int i = 0; i < max_attractors; i++)
		m_attractors[i] = &m_attractors_mem[i];
	for (int i = 0; i < max_emitters; i++)
		m_emitters[i] = &m_emitters_mem[i];
	for (int i = 0; i < max_collectors; i++)
		m_collectors[i] = &m_collectors_mem[i];
	for (int i = 0; i < max_instance_handles; i++)
		m_instance_handles[i] = &m_instance_handles_mem[i];
	for (int i = 0; i < max_instigator_handles; i++)
		m_instigator_handles[i] = &m_instigator_handles_mem[i];

	for (int i = 0; i < max_effects; i++)
	{
		ParticleEffect& effect = *m_effects[i];
		effect.is_alive = false;
	}	

	// Reset all sound effects.
	Reset_Particle_Sounds();

	Reset_For_Demo();
}

void ParticleManager::Reset_For_Demo()
{
	m_spawned_demo_ids.clear();
	m_spawned_collector_ids.clear();
}

void ParticleManager::Run_Event(ParticleEvent& evt)
{
	float rand_val = (float)m_rand.Next_Double();
	if (rand_val > evt.event->probability)
	{
		return;
	}
	if (evt.event->modifier_hash != 0)
	{
		if (evt.particle->emitter_instance == NULL ||
			evt.particle->emitter_instance->effect == NULL)
		{
			return;
		}

		if (evt.event->modifier_hash != evt.particle->emitter_instance->effect->modifier_hash)
		{
			return;
		}
	}

	switch (evt.event->type)
	{
	case ParticleFX_EventType::Play_Sound:
		{
			double time = Platform::Get()->Get_Ticks();

			double timer = time - evt.event->last_global_trigger;
			if (timer >= evt.event->time)
			{
				SoundHandle* sfx = ResourceFactory::Get()->Get_Sound(evt.event->param.c_str());
				if (sfx != NULL)
				{
					sfx->Get()->Play(NULL, evt.particle->spawn_position + evt.particle->position, false);
				}
				else
				{
					DBG_LOG("Particle effect attempted to play unknown sound '%s'.", evt.event->param.c_str());
				}

				evt.event->last_global_trigger = time;
			}
			break;
		}
	case ParticleFX_EventType::Spawn:
		{
			ParticleFXHandle* pfx = ResourceFactory::Get()->Get_ParticleFX(evt.event->param.c_str());
			if (pfx != NULL)
			{
				std::vector<float> upgrade_modifiers;
				if (evt.particle->emitter_instance != NULL && evt.particle->emitter_instance->effect != NULL)
				{
					upgrade_modifiers = evt.particle->emitter_instance->effect->upgrade_modifiers;
				}

				if (evt.particle->using_collision)
				{
					// STAT_TODO
					ParticleEffectHandle handle = Spawn(pfx, evt.particle->collision_last_center, evt.particle->collision_last_angle, 0, "", evt.particle->subtype, evt.particle->weapon_type, upgrade_modifiers);
					if (handle.m_effect != NULL)
					{
						handle.m_effect->instigator = evt.particle->instigator;
						handle.m_effect->instigator_incarnation = evt.particle->instigator_incarnation;
						handle.m_effect->demo_relevant = false;
						Update_Demo_Properties(handle.m_effect);
					}
				}
				else
				{
					// STAT_TODO
					ParticleEffectHandle handle = Spawn(pfx, evt.particle->spawn_position + evt.particle->position, evt.particle->spawn_direction + evt.particle->direction, 0, "", evt.particle->subtype, evt.particle->weapon_type, upgrade_modifiers);
					if (handle.m_effect != NULL)
					{
						handle.m_effect->instigator = evt.particle->instigator;
						handle.m_effect->instigator_incarnation = evt.particle->instigator_incarnation;
						handle.m_effect->demo_relevant = false;
						Update_Demo_Properties(handle.m_effect);
					}
				}
			}
			else
			{
				DBG_LOG("Particle effect attempted to spawn unknown effect '%s'.", evt.event->param.c_str());
			}
			break;
		}
	case ParticleFX_EventType::Spawn_Attached:
		{
			ParticleFXHandle* pfx = ResourceFactory::Get()->Get_ParticleFX(evt.event->param.c_str());
			if (pfx != NULL)
			{
				std::vector<float> upgrade_modifiers;
				if (evt.particle->emitter_instance != NULL && evt.particle->emitter_instance->effect != NULL)
				{
					upgrade_modifiers = evt.particle->emitter_instance->effect->upgrade_modifiers;
				}

				if (evt.particle->using_collision && evt.particle->emitter_type->draw_method != ParticleFX_EmitterDrawMethod::Beam)
				{
					// STAT_TODO
					ParticleEffectHandle handle = Spawn(pfx, evt.particle->collision_last_center + Vector3(0.0f, -evt.particle->height, 0.0f), evt.particle->collision_last_angle, 0, "", evt.particle->subtype, evt.particle->weapon_type, upgrade_modifiers);
					if (handle.m_effect != NULL)
					{
						handle.m_effect->instigator = evt.particle->instigator;
						handle.m_effect->instigator_incarnation = evt.particle->instigator_incarnation;
						handle.m_effect->owner = Alloc_Particle_Instance_Handle(evt.particle);
						handle.m_effect->demo_relevant = false;
						Update_Demo_Properties(handle.m_effect);
					}
				}
				else
				{
					// STAT_TODO
					ParticleEffectHandle handle = Spawn(pfx, evt.particle->spawn_position + evt.particle->position, evt.particle->spawn_direction + evt.particle->direction, 0, "", evt.particle->subtype, evt.particle->weapon_type, upgrade_modifiers);
					if (handle.m_effect != NULL)
					{
						handle.m_effect->instigator = evt.particle->instigator;
						handle.m_effect->instigator_incarnation = evt.particle->instigator_incarnation;
						handle.m_effect->owner = Alloc_Particle_Instance_Handle(evt.particle);
						handle.m_effect->demo_relevant = false;
						Update_Demo_Properties(handle.m_effect);
					}
				}
			}
			else
			{
				DBG_LOG("Particle effect attempted to spawn unknown effect '%s'.", evt.event->param.c_str());
			}
			break;
		}
	case ParticleFX_EventType::Destroy:
		{
			// Die plz.
			evt.particle->is_alive = false;
			evt.particle->is_alive_flag = false;
			break;
		}
	case ParticleFX_EventType::Script_Event:
		{
			GameEngine::Get()->Get_Runner()->Particle_Script_Event(evt.particle, evt.event->param);
			break;
		}
	case ParticleFX_EventType::Script_Event_Global:
		{
			GameEngine::Get()->Get_Runner()->Particle_Global_Script_Event(evt.particle, evt.event->param);
			break;
		}
	}
}

void ParticleManager::Tick(const FrameTime& time)
{
	GameEngine* engine = GameEngine::Get();
	TaskManager* manager = TaskManager::Get();

	//DBG_LOG("Instigators: %i", m_instigator_handle_count);

	bool workers_exist = false;

	// Wait until simulation tasks are finished.
	{
		PROFILE_SCOPE("Waiting");
		STATISTICS_TIMED_SCOPE(g_particle_stall_time);

		for (int i = 0; i < m_worker_count; i++)
		{
			if (m_worker_ids[i] >= 0)
			{
				manager->Wait_For(m_worker_ids[i]);
				m_worker_ids[i] = -1;

				workers_exist = true;
			}
		}

		if (workers_exist == true)
		{
	//		DBG_ASSERT(m_workers_run == m_worker_count);
	//		DBG_ASSERT(m_particles_simulated == m_simulate_particle_count);
		}
	}

	// Run simulation events.
	{
		PROFILE_SCOPE("Events");
		STATISTICS_TIMED_SCOPE(g_particle_event_time);

		int total_events = Min(m_simulation_event_count, max_events_per_frame);

		std::vector<ParticleEventNeighbourMarker> neighbour_positions;
		std::vector<ParticleInstance*> ignored_particles;

		for (int i = 0; i < total_events; i++)
		{
			ParticleEvent& evt = m_simulation_events[i];

			// If doing a limit radius check, ignore future events for particle if within a neighbours range.
			if (evt.event->type == ParticleFX_EventType::Limit_Neighbour_Radius)
			{
				bool bNeighbourExists = false;

				Vector3 src_pos = Vector3(evt.particle->spawn_position.X + evt.particle->draw_position.X, evt.particle->spawn_position.Y + evt.particle->draw_position.Y, 0.0f);

				for (unsigned int j = 0; j < neighbour_positions.size(); j++)
				{
					float distance = (neighbour_positions[j].position - src_pos).Length();
					if (distance < evt.event->param_float)
					{
						bNeighbourExists = true;
						break;
					}
				}

				if (bNeighbourExists)
				{
					ignored_particles.push_back(evt.particle);
					evt.particle->neighbour_exists = true;
				}
				else if (evt.particle->neighbour_exists == false)
				{
					ParticleEventNeighbourMarker n;
					n.emitter_type = evt.particle->emitter_type;
					n.position = src_pos;
					neighbour_positions.push_back(n);
				}
			}

			if (std::find(ignored_particles.begin(), ignored_particles.end(), evt.particle) != ignored_particles.end())
			{
				continue;
			}

			if (evt.event->trigger == ParticleFX_EventTrigger::OwnerTrigger)
			{
				if (evt.particle->invoke_game_trigger_delay > 0.0f)
				{
					continue;
				}

				if (evt.particle->neighbour_exists)
				{
					evt.particle->is_alive = false;
					evt.particle->is_alive_flag = false;
					continue;
				}

				evt.particle->invoke_game_trigger = false;
				evt.particle->invoke_game_trigger_delay = 0.0f;
			}

			Run_Event(evt);
		}

		m_simulation_event_count = 0;
	}

	// Push collision into collision manager.
	{
		PROFILE_SCOPE("Collision");
		STATISTICS_TIMED_SCOPE(g_particle_collision_time);

		// Process old collision requests.
		for (int i = 0; i < m_pending_collision_request_count; i++)
		{
			ParticleCollisionRequest& request = m_pending_collision_requests[i];
			if (request.particle->is_alive == true && request.particle->incarnation == request.incarnation)
			{
				if (request.particle->incarnation != request.handle->Get_Instigator_Incarnation())
				{
					continue;
				} 

				if (request.particle->ignore_spawn_collision)
				{
					if (request.particle->checked_spawn_collision == false)
					{
						request.particle->checked_spawn_collision = true;

						if (request.handle->Has_Collided())
						{
							request.particle->spawned_in_collision = 1;
						}
					}
					else if (request.particle->spawned_in_collision > 0)
					{
						if (request.handle->Has_Collided() == false && !request.is_damage)
						{
							request.particle->spawned_in_collision--;
							if (request.particle->spawned_in_collision <= 0)
							{
								request.particle->ignore_spawn_collision = false;
							}
						}
					}

					request.handle->Clear_Collided();
				}

				// Don't perform request if collision has been resolved on a previous deferred request. Otherwise
				// we will end up with all kinds of wierd shit going on, resolving then jumping back into the collision.
				if (request.is_damage == true || request.collision_resolve_count == request.particle->collision_resolve_count)
				{
					ParticleFX_EmitterConfig* type = request.particle->emitter_type;

					if (!request.particle->ignore_spawn_collision && request.particle->emitter_type->destroy_on_spawn_collision)
					{
						if (!request.is_damage)
						{
							if (request.particle->checked_spawn_collision == false)
							{
								request.particle->checked_spawn_collision = true;

								if (request.handle->Has_Collided())
								{
									request.particle->is_alive = false;
									request.particle->is_alive_flag = false;
								}
							}
						}
					}

					// Damage events do not affect position.
					if (request.is_damage == false)
					{						
						if (request.is_beam)
						{				
							if (request.particle->emitter_instance != NULL)
							{
								if (request.handle->Has_Collided())
								{
									Vector3 intersection_point = request.handle->Get_Closest_Intersection_Point(Vector3(request.area.X, request.area.Y, 0.0f));

									request.particle->draw_position = intersection_point - request.spawn_position; 
									request.particle->position = request.particle->draw_position;
								}
								else
								{
									request.particle->draw_position = Vector3(request.area.Width, request.area.Height, 0.0f) - request.spawn_position; 
								}
							}
						}
						else
						{
							request.particle->draw_position = request.handle->Get_Position() - request.base_position;
							if (request.handle->Has_Been_Velocity_Affected() && !request.handle->Has_Collided())
							{
								request.particle->position = request.particle->position + (request.handle->Get_Position() - request.handle->Get_Start_Position());
								request.particle->velocity_affected_last_frame = true;
							}
						}
					}

					if (request.handle->Has_Collided() && request.is_damage == false)
					{
						request.particle->collision_resolve_count++;

						if (request.is_beam == false)
						{
							// Damage effects do not produce a collision response.
							switch (type->collision_response)
							{
							case ParticleFX_CollisionResponse::Bounce:
								{
									request.particle->position = request.particle->draw_position;
									request.particle->ricochet_count++;

									CollisionEvent* first_event = NULL;
									int event_count = request.handle->Get_Events(first_event);

									for (int j = 0; j < event_count; j++)
									{
										if (first_event->Type == CollisionEventType::Touch)
										{
											Rect2D area = first_event->From->Get_Area();
											Vector3 from_pos = first_event->From->Get_Position();
											Vector2 from_center = Vector2(from_pos.X, from_pos.Y) + area.Center();
											Rect2D to_area = first_event->To->Get_Area() + first_event->To->Get_Position();

											// Left/Right
											if (from_center.Y >= to_area.Y &&
												from_center.Y < to_area.Y + to_area.Height)
											{
												request.particle->velocity_scale.X = -request.particle->velocity_scale.X;
											}
										
											// Top/Bottom
											else
											{
												request.particle->velocity_scale.Y = -request.particle->velocity_scale.Y;
											}
										}

										first_event = first_event->Next;
									}

									break;
								}
							case ParticleFX_CollisionResponse::Stop:
							default:
								{
									request.particle->position = request.particle->draw_position;
									break;
								}
							}
						}

						// If beam just set position to draw position.
						else
						{
					//		request.particle->position = request.particle->draw_position;
						}

						for (int j = 0; j < type->event_count; j++)
						{
							ParticleFX_Event& evt = type->events[j];
							if (evt.trigger == ParticleFX_EventTrigger::Collision)
							{
								ParticleEvent fire_evt;
								fire_evt.event = &evt;
								fire_evt.particle = request.particle;

								Run_Event(fire_evt);
							}
						}
					}

					// Check if we need to fire any collision events.
					// Damage events only cause collision events if collision was accepted.
					if (request.is_damage == true && request.handle->Get_Damage_Accepted_Flag())
					{
						request.handle->Clear_Damage_Accepted_Flag();
						request.particle->is_penetrating = true;
						request.particle->penetration_count++;

						bool bFireEvent = true;

						if (request.particle->emitter_type->is_projectile &&
							request.particle->emitter_instance &&
							request.particle->emitter_instance->effect)
						{
							if (request.particle->emitter_instance->effect->upgrade_modifiers[(int)ParticleUpgradeModifiers::Penetrating] != 0.0f)
							{
								if (request.particle->penetration_count <= request.particle->emitter_instance->effect->upgrade_modifiers[(int)ParticleUpgradeModifiers::Accuracy])
								{
									bFireEvent = false;
								}
							}
						}

						if (bFireEvent)
						{
							for (int j = 0; j < type->event_count; j++)
							{
								ParticleFX_Event& evt = type->events[j];
								if (evt.trigger == ParticleFX_EventTrigger::Penetration)
								{
									ParticleEvent fire_evt;
									fire_evt.event = &evt;
									fire_evt.particle = request.particle;

									Run_Event(fire_evt);
								}
							}
						}
					}
				}
			}
		}

		m_pending_collision_request_count = 0;
		m_last_collision_count_requested = m_simulation_collision_request_count;

		// Push in new collision requests.
		int total_requests = Min(m_simulation_collision_request_count, max_collision_requests_per_frame);
		for (int i = 0; i < total_requests; i++)
		{
			ParticleCollisionRequest request = m_collision_requests[i];
			
			if (request.is_beam == true)
			{
				if (request.is_damage == true)
				{
					request.handle = CollisionManager::Get()->Create_Handle(
						CollisionType::Non_Solid, 
						CollisionShape::Line, 
						CollisionGroup::Damage, 
						CollisionGroup::NONE,
						request.area,
						request.base_position + request.new_position,
						request.base_position + request.old_position,
						false,
						NULL,
						request.particle,
						request.particle->incarnation);
				}
				else
				{
					request.handle = CollisionManager::Get()->Create_Handle(
						CollisionType::Non_Solid, 
						CollisionShape::Line, 
						CollisionGroup::Particle, 
						CollisionGroup::Environment,
						request.area,
						request.base_position + request.new_position,
						request.base_position + request.old_position,
						false,
						NULL,
						request.particle,
						request.particle->incarnation);
				}
			}
			else
			{
				if (request.is_damage == true)
				{
					request.handle = CollisionManager::Get()->Create_Handle(
						CollisionType::Non_Solid, 
						CollisionShape::Circle, 
						CollisionGroup::Damage, 
						CollisionGroup::NONE,
						request.area,
						request.base_position + request.new_position,
						request.base_position + request.old_position,
						false,
						NULL,
						request.particle,
						request.particle->incarnation);
				}
				else
				{
					request.handle = CollisionManager::Get()->Create_Handle(
						CollisionType::Solid, 
						CollisionShape::Rectangle, 
						CollisionGroup::Particle, 
						CollisionGroup::Environment,
						request.area,
						request.base_position + request.new_position,
						request.base_position + request.old_position,
						false,
						NULL,
						request.particle,
						request.particle->incarnation);
				}
			}

			m_pending_collision_requests[m_pending_collision_request_count++] = request;
		}

		m_simulation_collision_request_count = 0;
	}

	// Cull all dead effects.
	{
		PROFILE_SCOPE("Culling");
		STATISTICS_TIMED_SCOPE(g_particle_culling_time);

		MutexLock lock(m_mutex);

		// Kill old effects.
		for (int i = 0; i < m_effect_count; i++)
		{
			ParticleEffect& effect = *m_effects[i];
			bool is_alive = false;

			if (!effect.was_simulated)
			{
				is_alive = true;
				effect.was_simulated = false;
			}
			else
			{
				bool bDoChecks = true;

				if (effect.force_dispose == true)
				{
					if (effect.force_dispose_started)
					{
						bDoChecks = false;
					}
					else
					{
						effect.force_dispose_started = true;
					}
				}

				if (bDoChecks)
				{
					for (int j = 0; j < effect.emitter_count; j++)
					{
						ParticleEmitter& emitter = *effect.emitters[j];
						if (emitter.is_alive_flag == true)
						{
							is_alive = true;
						}
						emitter.is_alive_flag = false;
					}
				}
			}

			// Goodbye!
			if (is_alive == false)
			{
				effect.is_alive = false;

				//DBG_LOG("KILLING EFFECT '%s'.", effect.type->Get_Name().c_str());

				if (effect.owner != NULL)
				{
					Free_Particle_Instance_Handle(effect.owner);
					effect.owner = NULL;
				}

				// Swap removed effects with ones from the end of the list. So our effects
				// goes into the "dead area" and the original alive one at the end gets shunted down.
				ParticleEffect* tmp = m_effects[m_effect_count - 1];
				m_effects[--m_effect_count] = m_effects[i];
				m_effects[i] = tmp;

				i--;

				continue;
			}
			else
			{
				// TODO: We should just have 2 State vars and a reference and just switch each frame. this is all bs.

				if (effect.is_position_pending)
				{
					effect.position = effect.pending_position;
					effect.is_position_pending = false;

					// Add estimated movement this frame, helps hide the three frame latency.
					if (effect.has_last_position)
					{
						effect.position = effect.position + ((effect.pending_position - effect.last_position) * 1); 
					}

					effect.has_last_position = true;
					effect.last_position = effect.pending_position;
				}
				if (effect.is_direction_pending)
				{
					effect.direction = effect.pending_direction;
					effect.is_direction_pending = false;
				}
				if (effect.is_target_pending)
				{
					effect.target = effect.pending_target;
					effect.is_target_pending = false;
				}
			}
		}

		// Kill dead emitters.
		for (int i = 0; i < m_emitter_count; i++)
		{
			ParticleEmitter& emitter = *m_emitters[i];
			if (!emitter.effect->is_alive)
			{
				//DBG_LOG("KILLING EMITTER '%s'.", emitter.type->name.c_str());
				//bool owner_alive = (emitter.effect->owner->Is_Valid());

				// Swap removed emitter with ones from the end of the list. So our effects
				// goes into the "dead area" and the original alive one at the end gets shunted down.
				ParticleEmitter* tmp = m_emitters[m_emitter_count - 1];
				m_emitters[--m_emitter_count] = m_emitters[i];
				m_emitters[i] = tmp;
			}
		}

		// Kill dead collectors.
		for (int i = 0; i < m_collector_count; i++)
		{
			ParticleCollector& collector = *m_collectors[i];
			if (collector.force_dispose)
			{
				// Swap removed emitter with ones from the end of the list. So our effects
				// goes into the "dead area" and the original alive one at the end gets shunted down.
				ParticleCollector* tmp = m_collectors[m_collector_count - 1];
				m_collectors[--m_collector_count] = m_collectors[i];
				m_collectors[i] = tmp;
			}
		}

		// Kill dead attractors.
		for (int i = 0; i < m_attractor_count; i++)
		{
			ParticleAttractor& emitter = *m_attractors[i];
			if (!emitter.effect->is_alive)
			{
				// Swap removed emitter with ones from the end of the list. So our effects
				// goes into the "dead area" and the original alive one at the end gets shunted down.
				ParticleAttractor* tmp = m_attractors[m_attractor_count - 1];
				m_attractors[--m_attractor_count] = m_attractors[i];
				m_attractors[i] = tmp;
			}
		}

		// Kill dead sounds.
		for (int i = 0; i < m_sound_count; i++)
		{
			ParticleSound& emitter = *m_sounds[i];
			if (emitter.kill_flag)
			{
				//DBG_LOG("Disposing of sound 0x%08x (Now %i Sounds)", m_sounds[i], m_sound_count - 1);
			
				Atomic::Increment32(&emitter.incarnation);
				SAFE_DELETE(emitter.channel);

				emitter.kill_flag = false;

				// Swap removed emitter with ones from the end of the list. So our effects
				// goes into the "dead area" and the original alive one at the end gets shunted down.
				ParticleSound* tmp = m_sounds[m_sound_count - 1];
				m_sounds[--m_sound_count] = m_sounds[i];
				m_sounds[i] = tmp;
			}
		}

		// Kill old particles.
		for (int i = 0; i < m_particle_count; i++)
		{
			ParticleInstance& particle = m_particles[i];
			if (particle.is_alive_flag == false)
			{
				//DBG_LOG("Despawning Particle %s (%f,%f,%f)", particle.emitter_type->name.c_str(), particle.position.X, particle.position.Y, particle.position.Z);
			
				if (particle.sound_handle.Is_Valid())
				{
					particle.sound_handle.Dispose();
				}

				/*DBG_LOG("<despawn %s> pos=%f,%f dir=%f spawn_pos=%f,%f spawn_dir=%f",
					particle.emitter_type->name.c_str(),
					particle.position.X, particle.position.Y, particle.direction,
					particle.spawn_position.X, particle.spawn_position.Y, particle.spawn_direction);
				*/	

				// Copy the last particle to the now vacated slot.
				ParticleInstance& last_particle = m_particles[m_particle_count - 1];

				m_particles[i] = m_particles[--m_particle_count];
				m_particles[m_particle_count].is_alive = false;
				i--;

				if (last_particle.is_owner)
				{
					Update_Particle_Instance_Handle(&last_particle, &particle);
				}

				continue;
			}
			else
			{
				particle.is_alive_flag = false;
				/*
				DBG_LOG("<reset alive flag %s> pos=%f,%f dir=%f spawn_pos=%f,%f spawn_dir=%f",
					particle.emitter_type->name.c_str(),
					particle.position.X, particle.position.Y, particle.direction,
					particle.spawn_position.X, particle.spawn_position.Y, particle.spawn_direction);*/
			}
		}
	}

	// Unlock verts from last write buffer
	if (m_current_geometry_write_buffer != NULL)
	{
		PROFILE_SCOPE("Upload Geometry");
		STATISTICS_TIMED_SCOPE(g_particle_geom_update_time);

		m_current_geometry_write_buffer->Unlock_Vertices((char*)m_write_geometry_buffer + m_current_geometry_write_buffer_size);
	}

	// Update current geometry buffer.
	m_geometry_write_buffer_index = 1 - m_geometry_write_buffer_index;
	m_current_geometry_write_buffer = m_geometry_buffers[m_geometry_write_buffer_index];
	m_write_geometry_buffer = m_current_geometry_write_buffer->Lock_Vertices();

	int geom_offset = 0;
	int stride = m_current_geometry_write_buffer->Get_Stride(); // 4 verts per particle.


	// Count total amount of required buffer space.
	int total_required_buffer_space = 0;
	for (int i = 0; i < m_buffer_count; i++)
	{
		ParticleBuffer& buffer = m_buffers[i];
		total_required_buffer_space += buffer.Required_Buffer_Count;
	}

	// Calculate how much buffer space to give to each buffer.
	int free_space = max_particles - total_required_buffer_space;
	int spawn_buffer_space = Min(free_space, max_particle_spawns_per_frame * m_buffer_count);
	int spawn_buffer_space_per_buffer = m_buffer_count == 0 ? 0 : spawn_buffer_space / m_buffer_count;

	// Setup each particle buffer.
	int render_particles = 0;
	int space_remaining = max_particles;
	for (int i = 0; i < m_buffer_count; i++)
	{
		ParticleBuffer& buffer			= m_buffers[i];
		
		// Rendering for previous frame.
		buffer.Render_Vertex_Offset			= buffer.Geometry_Vertices_Offset;
		buffer.Render_Vertex_Count			= Min(buffer.Max_Particles, buffer.Particle_Count) * 4;

		// Simulation for next frame.
		buffer.Geometry_Vertices_Offset		= geom_offset;
		buffer.Geometry_Vertices			= (char*)m_write_geometry_buffer + geom_offset;
		buffer.Geometry_Particle_Stride		= stride * 4;
		DBG_ASSERT(buffer.Required_Buffer_Count >= 0);

		int target_space = Max(0, buffer.Required_Buffer_Count + spawn_buffer_space_per_buffer);
		buffer.Max_Particles				= Min(space_remaining, target_space);
		buffer.Particle_Count				= 0;
		buffer.Required_Buffer_Count		= 0;

		space_remaining -= buffer.Max_Particles;
		geom_offset += (buffer.Max_Particles * 4) * stride;
	} 

	m_current_geometry_write_buffer_size = geom_offset;

	// Setup collectors.
	for (int i = 0; i < m_collector_count; i++)
	{
		ParticleCollector& collector = *m_collectors[i];
		collector.read_index = 1 - collector.read_index;
	
		for (int j = 0; j < ParticleFX_EmitterCollectionType::COUNT; j++)
		{
			collector.collection_count[1 - collector.read_index][j] = 0;
		}
	}

	// Add task for the next frame.
	m_simulate_particle_count				= m_particle_count;
	m_simulate_effect_count					= m_effect_count;
	m_simulate_collector_count				= m_collector_count;
	m_simulate_delta_t						= time.Get_Delta();
	m_render_particle_count					= 0;
	m_simulate_elapsed_time					= time.Get_Frame_Time();
	//m_workers_run							= 0;
	//m_particles_simulated					= 0;

	// Update stats.
	g_particle_active_particle_count.Set(m_simulate_particle_count);
	g_particle_active_effect_count.Set(m_simulate_effect_count);

	// Just some debug code to make sure we don't render with previous frame particles.
	for (int i = 0; i < max_particles; i++)
	{
		m_render_particle_buffer_sorted[i] = NULL;
		m_render_particle_buffer_final_sorted[i] = NULL;
	}

	//DBG_LOG("BUFFER COUNT: %i", m_buffer_count);

	if (m_render_camera != NULL)
	{
		m_simulation_camera_bounding_box = GameEngine::Get()->Get_Scene()->Get_Full_Camera_Bounding_Box();//m_render_camera->Get_Bounding_Viewport();	
		
		// Inflate the culling bound box a bit so we don't see particles flicking when they appear because of defered updating.
		m_simulation_camera_cull_bounding_box = m_simulation_camera_bounding_box.Inflate(m_simulation_camera_bounding_box.Width * 0.1f, m_simulation_camera_bounding_box.Height * 0.1f);
	}

	//DBG_LOG("===============================================");

	m_sort_blocker->Start(m_worker_count);
	m_simulate_blocker->Start(m_worker_count);

	m_simulation_collision_modulus	= m_last_collision_count_requested == 0 ? 1 : (int)ceilf(m_last_collision_count_requested / (float)max_collision_requests_per_frame);
	m_simulation_collision_offset++;
	m_last_collision_count_requested = 0;
	m_final_sort_complete = false;

	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_ids[i] = manager->Add_Task(m_workers[i]);	
		manager->Queue_Task(m_worker_ids[i]);
	}
}

ParticleEffectHandle ParticleManager::Spawn(const char* name, Vector3 position, float direction, int meta_number, std::string modifier, int subtype, CVMObject* weapon_type, std::vector<float> upgrade_modifiers)
{
	ParticleFXHandle* fx = ResourceFactory::Get()->Get_ParticleFX(name);
	return Spawn(fx, position, direction, meta_number, modifier, subtype, weapon_type, upgrade_modifiers);
}

ParticleEffectHandle ParticleManager::Spawn(ParticleFXHandle* handle, Vector3 position, float direction, int meta_number, std::string modifier, int subtype, CVMObject* weapon_type, std::vector<float> upgrade_modifiers)
{
	return Spawn(handle->Get(), position, direction, meta_number, modifier, subtype, weapon_type, upgrade_modifiers);
}

ParticleEffectHandle ParticleManager::Spawn(ParticleFX* fx, Vector3 position, float direction, int meta_number, std::string modifier, int subtype, CVMObject* weapon_type, std::vector<float> upgrade_modifiers)
{
	return Spawn(fx, position, direction, meta_number, (modifier == "" ? 0 : StringHelper::Hash(modifier.c_str())), subtype, weapon_type, upgrade_modifiers);
}

ParticleEffectHandle ParticleManager::Spawn(ParticleFX* fx, Vector3 position, float direction, int meta_number, int modifier_hash, int subtype, CVMObject* weapon_type, std::vector<float> upgrade_modifiers)
{
	MutexLock lock(m_mutex);

	if (m_effect_count >= max_effects)
		return ParticleEffectHandle(NULL, 0);

	if (m_emitter_count + fx->m_emitters.size() >= max_emitters)
		return ParticleEffectHandle(NULL, 0);
	
	if (m_attractor_count + fx->m_attractors.size() >= max_attractors)
		return ParticleEffectHandle(NULL, 0);

	// Create particle effect.
	ParticleEffect* effect			= m_effects[m_effect_count];	// NOTE: THIS IS NOT ATOMIC, WTF.
	effect->owner					= NULL;
	effect->incarnation				= Atomic::Increment32(&m_incarnation_index);
	effect->is_alive				= true;
	effect->type					= fx;
	effect->position				= position;
	effect->has_last_position		= false;
	effect->direction				= direction;
	effect->start_time				= Platform::Get()->Get_Ticks();
	effect->last_frame_time			= effect->start_time;
	effect->emitter_count			= 0;
	effect->attractor_count			= 0;
	effect->force_dispose			= false;
	effect->force_dispose_started	= false;
	effect->is_visible				= true;
	effect->is_paused				= false;
	effect->ignore_spawn_collision	= false;
	effect->instigator				= NULL;
	effect->instigator_incarnation	= NULL;
	effect->meta_number				= meta_number;
	effect->modifier_hash			= modifier_hash;
	effect->is_position_pending		= false;
	effect->is_direction_pending	= false;
	effect->is_target_pending		= false;
	effect->subtype					= subtype;
	effect->weapon_type				= weapon_type;
	effect->was_simulated			= false;
	effect->demo_id					= Atomic::Increment32(&m_demo_id_tracker);
	effect->demo_relevant			= true;
	effect->demo_instigator_team	= -1;
	effect->upgrade_modifiers		= upgrade_modifiers;

	if (upgrade_modifiers.size() == 0)
	{
		for (int i = 0; i < ParticleUpgradeModifiers::COUNT; i++)
		{
			effect->upgrade_modifiers.push_back(0.0f);
		}
	}

	m_effect_count++;

	// Create emitters.
	for (std::vector<ParticleFX_EmitterConfig>::iterator iter = fx->m_emitters.begin(); iter != fx->m_emitters.end(); iter++)
	{
		ParticleFX_EmitterConfig& config = *iter;
		config.render_pass_hash = StringHelper::Hash(config.render_pass.c_str());

		ParticleEmitter& emitter		= *m_emitters[m_emitter_count];	
		emitter.effect					= effect;
		emitter.is_alive_flag			= true;
		emitter.type					= &config;
		emitter.accumulated				= 0.0f;
		emitter.life_elapsed			= 0.0f;
		emitter.spawns_accumulated		= 0.0f;
		emitter.spawns_this_cycle		= 0;
		emitter.cycle_index				= 0;
		emitter.last_spawn_direction	= direction;
		emitter.last_spawn_position		= position;
		emitter.spawn_rate_range.Randomise(&config.curves[(int)ParticleFX_CurveType::Spawn_Rate]);
		emitter.demo_instigator_team	= effect->demo_instigator_team;

		effect->emitters[effect->emitter_count++] = &emitter;

		m_emitter_count++;
	}

	// Create attractors.
	for (std::vector<ParticleFX_AttractorConfig>::iterator iter = fx->m_attractors.begin(); iter != fx->m_attractors.end(); iter++)
	{
		ParticleFX_AttractorConfig& config = *iter;
		ParticleAttractor& attractor	= *m_attractors[m_attractor_count];
		attractor.effect				= effect;
		attractor.type					= &config;
		attractor.spawn_position		= position;
		attractor.spawn_direction		= direction;
		
		effect->attractors[effect->attractor_count++] = &attractor;

		m_attractor_count++;
	}

	ParticleEffectHandle handle;
	handle.m_effect = effect;
	handle.m_incarnation = effect->incarnation;

	//DBG_LOG("Spawened effect %i", effect->index);

	return handle;
}

ParticleCollectorHandle ParticleManager::Create_Collector(Vector3 position, float radius, float strength)
{
	MutexLock lock(m_mutex);

	ParticleCollector* collector = m_collectors[m_collector_count++];
	collector->force_dispose = false;
	collector->incarnation = Atomic::Increment32(&m_incarnation_index);
	collector->position = position;
	collector->radius = radius;
	collector->strength = strength;
	collector->is_paused = false;
	collector->read_index = 0;
	collector->demo_id = Atomic::Increment32(&m_demo_id_tracker);

	for (int i = 0; i < ParticleFX_EmitterCollectionType::COUNT; i++)
	{
		collector->collection_count[0][i] = 0;
		collector->collection_count[1][i] = 0;
	}

	ParticleCollectorHandle handle;
	handle.m_collector = collector;
	handle.m_incarnation = collector->incarnation;

	return handle;
}

float ParticleManager::Get_Draw_Depth(float y)
{
	return y - m_simulation_camera_bounding_box.Y;
}

void ParticleManager::Draw_Pass(const FrameTime& time, std::string pass_name)
{
	PROFILE_SCOPE("Particle Pass");

	Renderer* renderer = Renderer::Get();
	RenderPipeline* pipeline = RenderPipeline::Get();
	RenderPipeline_Shader* shader = pipeline->Get_Active_Shader();

	int render_buffer =  1 - m_geometry_write_buffer_index;
	Geometry* geometry = m_geometry_buffers[render_buffer];

	int pass_name_hash = StringHelper::Hash(pass_name.c_str());

	//if (pass_name != "geometry")
	//{
	//	return;
	//}

	//DBG_LOG("Pass:%s", pass_name.c_str());

	int stride = geometry->Get_Stride(); // 4 verts per particle.
	m_render_camera = RenderPipeline::Get()->Get_Active_Camera();

	// Go through each buffer and draw Render_Offset to Render_Count quads.
//	DBG_LOG("[Rendering %i buffers (Pass:%s, Shader:%s)]", m_buffer_count, pass_name.c_str(), shader->Shader_Program->Get()->Get_Resource()->name );
	for (int i = 0; i < m_buffer_count; i++)
	{
		ParticleBuffer& buffer = m_buffers[i];	
		if (buffer.Render_Pass_Hash == pass_name_hash && 
			buffer.Render_Vertex_Count > 0)
		{
			renderer->Set_Alpha_Test(true);
			renderer->Set_Blend(true);

			switch (buffer.Blend_Mode)
			{
			case ParticleFX_EmitterBlendMode::Lighten:
				{
					renderer->Set_Blend_Function(RendererOption::E_Src_Alpha_One);
					break;
				}
			default:
				{
					renderer->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
					break;
				}
			}

			// Gross
			if (pass_name == "lighting" || pass_name == "distortion")
			{
				renderer->Set_Depth_Test(false); 
				renderer->Set_Depth_Write(false);
			}
			else
			{
				renderer->Set_Depth_Test(true); 
				renderer->Set_Depth_Write(false); // We use painters algorithm for rendering particles, and nothing else gives a fuck about particle depth as we are drawn on top of everything.
			}
			renderer->Bind_Texture(buffer.Render_Texture, 0);

			if (shader != NULL)
			{
				shader->Shader_Program->Get()->Bind_Texture("g_texture", 0);
				shader->Shader_Program->Get()->Bind_Bool("g_texture_enabled", true);	
				shader->Shader_Program->Get()->Bind_Float("g_tiled", false);
				shader->Shader_Program->Get()->Bind_Vector("g_draw_color", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			}

			int prim_offset = (buffer.Render_Vertex_Offset / stride) / 4;
			int prim_count = buffer.Render_Vertex_Count / 4;

			geometry->Render_Partial(prim_offset, prim_count);

//			DBG_ONSCREEN_LOG(0x50000000 + i, Color::White, 1.0f, "Buffer %i - Rendering %i to %i primitives (buffer size %i).", i, prim_offset, prim_offset + prim_count, geometry->Get_Max_Primitives());

//			DBG_LOG("\tBuffer[%i/%i] %i to %i", i, pass_name_hash, prim_offset, prim_count);

			renderer->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
			renderer->Set_Depth_Test(false); 
			renderer->Set_Depth_Write(true); 
		}
	}

	if (m_draw_debug)
	{
		if (pass_name == "geometry")
		{
			PrimitiveRenderer pr;	

			// Draw emitter nodes.
			for (int i = 0; i < m_effect_count; i++)
			{
				ParticleEffect& effect = *m_effects[i];
				pr.Draw_Solid_Quad(Rect2D(effect.position.X - 1.0f, effect.position.Y - 1.0f, 2.0f, 2.0f), Color::Red);
			}


			for (int i = 0; i < m_simulate_collector_count; i++)
			{
				ParticleCollector& collector = *m_collectors[i];
				if (collector.is_paused == false)
				{
					Vector2 attractor_pos = Vector2(collector.position.X, collector.position.Y);
					pr.Draw_Solid_Oval(Rect2D(attractor_pos.X - collector.radius, attractor_pos.Y - collector.radius, collector.radius * 2.0f, collector.radius * 2.0f), Color(0.0, 0.0, 255.0f, 64.0f));
				}
			}

			//pr.Draw_Wireframe_Quad(m_simulation_camera_cull_bounding_box, 0.0f, Color::Green, 3.0f);
		}
	}
}

void ParticleManager::Simulate_Particle(float elapsed_time, float sim_delta, ParticleInstance* particle, int particle_index, int worker_index)
{	
	ParticleFX_EmitterConfig* type = particle->emitter_type;

	ParticleEmitter* emitter = particle->emitter_instance;
	ParticleEffect* effect = emitter != NULL ? emitter->effect : NULL;

	bool first_frame = (particle->alive_time == 0.0f);

	particle->alive_time += elapsed_time;

	bool force_kill = false;
	bool is_visible = true;
	if (effect != NULL)
		is_visible = effect->is_visible;

	if (effect != NULL && effect->force_dispose)
		force_kill = effect->force_dispose;

	// Don't render on first frame as we don't have space in the buffer reserved for us yet and will
	// cause nasty flicking of particles.
	if (first_frame)
	{
		is_visible = true;
	}

//	Atomic::Increment32(&m_particles_simulated);

	float delta = particle->alive_time / (particle->lifetime * 1000);
	if (delta <= 1.0f || particle->last_update_delta < 1.0f)
	{
		delta = Min(1.0f, delta);

		int   sprite_frame		= (int)type->curves[ParticleFX_CurveType::Sprite_Frame].Get(delta, particle->sprite_frame_range);
		float velocity_x		= type->curves[ParticleFX_CurveType::Velocity_X].Get(delta, particle->velocity_x_range) * particle->velocity_scale.X;
		float velocity_y		= type->curves[ParticleFX_CurveType::Velocity_Y].Get(delta, particle->velocity_y_range) * particle->velocity_scale.Y;
		float velocity_height	= type->curves[ParticleFX_CurveType::Velocity_Height].Get(delta, particle->velocity_height_range);
		float velocity_rot		= type->curves[ParticleFX_CurveType::Velocity_Rotation].Get(delta, particle->velocity_rotation_range);
		float velocity_dir		= type->curves[ParticleFX_CurveType::Velocity_Direction].Get(delta, particle->velocity_direction_range);
		float color_r			= type->curves[ParticleFX_CurveType::Color_R].Get(delta, particle->color_r_range);
		float color_g			= type->curves[ParticleFX_CurveType::Color_G].Get(delta, particle->color_g_range);
		float color_b			= type->curves[ParticleFX_CurveType::Color_B].Get(delta, particle->color_b_range);
		float color_a			= type->curves[ParticleFX_CurveType::Color_A].Get(delta, particle->color_a_range);
		float scale_x			= type->curves[ParticleFX_CurveType::Scale_X].Get(delta, particle->scale_x_range);
		float scale_y			= type->curves[ParticleFX_CurveType::Scale_Y].Get(delta, particle->scale_y_range);
		bool flip_h				= false;
		bool flip_v				= false;

		if (type->scale_proportional == true)
		{
			scale_x = (scale_x + scale_y) * 0.5f;
			scale_y = scale_x;
		}

		if (type->invert_scale_on_bounce == true)
		{
			flip_h = particle->velocity_scale.X < 0.0f;
			flip_v = particle->velocity_scale.Y < 0.0f;
		}

		bool is_penetrating = particle->is_penetrating;
		bool was_penetrating = particle->was_penetrating;

		particle->is_penetrating = false;
		particle->was_penetrating = is_penetrating;

		if (is_penetrating)
		{
			velocity_x *= particle->emitter_type->penetration_speed_scalar;	
			velocity_y *= particle->emitter_type->penetration_speed_scalar;	
		}

		// Figure out looping sound to play.
		SoundHandle* target_sound = particle->emitter_type->loop_sound;
		if (is_penetrating)
		{
			if (particle->emitter_type->penetration_sound != NULL)
			{
				target_sound = particle->emitter_type->penetration_sound;
			}
		}

		if (target_sound != NULL)
		{
			// Dispose of old looping sound.
			if (particle->last_sound != target_sound)
			{
				if (particle->sound_handle.Is_Valid())
				{
					particle->sound_handle.Dispose();
				}
			}

			// Play sound if non active.
			if (!particle->sound_handle.Is_Valid())
			{
				/*DBG_LOG("Alloc_Particle_Sound, Particle=0x%08x, Penetration=%i, Valid=(manager=0x%08x, index=%i, incarnation=%i/%i)", 
					particle, 
					is_penetrating, 
					particle->sound_handle.manager, 
					particle->sound_handle.sound, 
					particle->sound_handle.manager == NULL ? 0 : particle->sound_handle.sound->incarnation,
					particle->sound_handle.incarnation
				);*/
				particle->sound_handle = Alloc_Particle_Sound(target_sound);
			}
			
			particle->last_sound = target_sound;
		}
		else if (particle->sound_handle.Is_Valid())
		{
			particle->sound_handle.Dispose();
		}

		// Update position of sound.
		if (particle->sound_handle.Is_Valid())
		{
			particle->sound_handle.Update(particle->spawn_position + particle->position);
		}

		// Figure out basic rendering stuff.
		Vector3 start_position = particle->position;

		AtlasFrame* frame = type->sprite_frame;
		if ((frame == NULL || sprite_frame > 0) && type->sprite_anim != NULL)
			frame = type->sprite_anim->Frames[sprite_frame % type->sprite_anim->Frames.size()];

		if (frame == NULL)
			return;

		// Simulate angular velocity.
		particle->direction += DegToRad(velocity_dir) * sim_delta;
		particle->rotation  += DegToRad(velocity_rot) * sim_delta;
	
		// Do bounce velocity.
		if (particle->bouncing == true)
		{
			if (particle->height_velocity_scale == 0.0f)
			{
				particle->height_velocity_scale = 1.0f;
				particle->height_velocity = 0.0f;
			}

			particle->height_velocity -= PARTICLE_GRAVITY * sim_delta;		
			particle->height += (particle->height_velocity * particle->height_velocity_scale) * sim_delta;

			if (particle->height <= 0.0f)
			{
				// Check if we need to fire any bounce events.
				for (int i = 0; i < type->event_count; i++)
				{
					ParticleFX_Event& evt = type->events[i];
					if (evt.trigger == ParticleFX_EventTrigger::Bounce)
					{
						int evt_index = Atomic::Increment32(&m_simulation_event_count) - 1;
						if (evt_index < max_events_per_frame)
						{
							m_simulation_events[evt_index].event	  = &evt;
							m_simulation_events[evt_index].particle   = particle;
						}
					}
				}

				particle->height_velocity = velocity_height;
				particle->height = 0.0f;
				particle->bouncing = (particle->height_velocity > PARTICLE_MIN_BOUNCE_VELOCITY);
			}
		}

		float direction = particle->spawn_direction + particle->direction;

		float draw_dir = type->lock_rotation_to_direction 
			? direction 
			: type->add_emitter_direction ? direction + particle->rotation : particle->rotation;

		// Simulate velocity.
		if (type->move_based_on_direction == true)
		{
			particle->position.X += (cos(direction - HALFPI) * velocity_x) * sim_delta;
			particle->position.Y += (sin(direction - HALFPI) * velocity_y) * sim_delta;
		}
		else
		{
			particle->position.X += velocity_x * sim_delta;
			particle->position.Y += velocity_y * sim_delta;
		}	

		// Apply attractors.
		if (type->affected_by_attractors == true && effect != NULL)
		{
			for (int i = 0; i < effect->attractor_count; i++)
			{
				ParticleAttractor& attractor = *effect->attractors[i];
				float base_x = attractor.spawn_position.X;
				float base_y = attractor.spawn_position.Y;
				float base_dir = attractor.spawn_direction;

				if (attractor.type->track_effect == true)
				{
					base_x = effect->position.X;
					base_y = effect->position.Y;
					base_dir = effect->direction;
				}

				// Apply offset to position.
				base_x += (cos(base_dir - HALFPI) * attractor.type->x_offset);
				base_y += (sin(base_dir - HALFPI) * attractor.type->y_offset);

				Vector2 particle_pos = Vector2(particle->spawn_position.X + particle->position.X, particle->spawn_position.Y + particle->position.Y);
				Vector2 attractor_pos = Vector2(base_x, base_y);

				// Work out how far away we are from attractor.
				float dist = (particle_pos - attractor_pos).Length();
				float attraction = Clamp(1.0f - (dist / attractor.type->radius), 0.0f, 1.0f);				
				float angle_to_attractor = atan2(attractor_pos.Y - particle_pos.Y, attractor_pos.X - particle_pos.X);

				//particle->direction = Math::Lerp(particle->direction, angle_to_attractor - HALFPI, attraction * 0.1f);
				particle->position.X += (cos(angle_to_attractor) * (attractor.type->strength * attraction)) * sim_delta;
				particle->position.Y += (sin(angle_to_attractor) * (attractor.type->strength * attraction)) * sim_delta;
			}
		}

		bool disable_collision = false;

		// Apply collectors.
		if (type->collection_enabled == true)
		{
			bool collecting = false;

			for (int i = 0; i < m_simulate_collector_count; i++)
			{
				ParticleCollector& collector = *m_collectors[i];
				if (collector.is_paused == false)
				{
					Vector2 particle_pos = Vector2(particle->spawn_position.X + particle->position.X, particle->spawn_position.Y + particle->position.Y);
					Vector2 attractor_pos = Vector2(collector.position.X, collector.position.Y);

					// Work out how far away we are from attractor.
					float dist = (particle_pos - attractor_pos).Length();
					float attraction = Clamp(1.0f - (dist / collector.radius), 0.0f, 1.0f);				
					float angle_to_attractor = atan2(attractor_pos.Y - particle_pos.Y, attractor_pos.X - particle_pos.X);

					// Collected?
					if (dist <= collector.radius * 0.1f)
					{
						// Check if we need to fire any bounce events.
						for (int j = 0; j < type->event_count; j++)
						{
							ParticleFX_Event& evt = type->events[j];
							if (evt.trigger == ParticleFX_EventTrigger::Collect)
							{
								int evt_index = Atomic::Increment32(&m_simulation_event_count) - 1;
								if (evt_index < max_events_per_frame)
								{
									m_simulation_events[evt_index].event	  = &evt;
									m_simulation_events[evt_index].particle   = particle;
								}
							}
						}

						Atomic::Add32(&collector.collection_count[1 - collector.read_index][type->collection_type], type->collection_value);
						force_kill = true;
					}

					//particle->direction = Math::Lerp(particle->direction, angle_to_attractor - HALFPI, attraction * 0.1f);
					particle->position.X += (cos(angle_to_attractor) * (collector.strength * attraction)) * sim_delta;
					particle->position.Y += (sin(angle_to_attractor) * (collector.strength * attraction)) * sim_delta;
					particle->height = Min(16.0f, particle->height + (collector.strength * attraction) * sim_delta);

					if (attraction > 0.0f)
					{
						collecting = true;
						disable_collision = true;		
					}
				}
			}

			if (collecting == false && particle->height > 0.0f && (particle->height_velocity == 0.0f || particle->bouncing == false))
			{
				particle->height = Max(0.0f, particle->height - (PARTICLE_GRAVITY * sim_delta));
			}
		}

		// Do collision movement on different particles each frame so we
		// can still do collision even when not enough requests per frame. Its not perfect and particles
		// we clip into things but its considerably better than having no collision on half the particles :P.
		bool can_do_collision = true;
		bool not_moved = false;
		
		if (disable_collision)
		{
			can_do_collision = false;
		}

		if (type->enable_collision)
		{
			// If we haven't moved disable collision.
			if ((start_position - particle->position).Length_Squared() < 0.001f && !type->track_effect && !particle->velocity_affected_last_frame)
			{
				not_moved = true;
			}

			// Modulus if we have to many collisions.
			if (can_do_collision)
				can_do_collision = ((particle_index + m_simulation_collision_offset) % m_simulation_collision_modulus) == 0;		
		}

		// Perform collision.
		if (type->enable_collision && (can_do_collision && !not_moved))
		{
			if (particle->draw_position != particle->position || type->track_effect)
			{
				float collision_scale_x = type->collision_scale_x;
				float collision_scale_y = type->collision_scale_y;
				float collision_offset_x = type->collision_offset_x * -sin(direction + type->collision_dir_offset);//* -sin(draw_dir + type->collision_dir_offset);
				float collision_offset_y = type->collision_offset_y *  cos(direction + type->collision_dir_offset); //* cos(draw_dir + type->collision_dir_offset);

				if (flip_h)
				{
					collision_offset_x = -collision_offset_x;
				}

				if (flip_v)
				{
					collision_offset_y = -collision_offset_y;
				}

				// Request a collision based movement for the next frame plz.
				int evt_index = Atomic::Increment32(&m_simulation_collision_request_count) - 1;
				if (evt_index < max_collision_requests_per_frame)
				{
					m_collision_requests[evt_index].particle = particle;
					m_collision_requests[evt_index].incarnation = particle->incarnation;

					if (type->track_effect == true)
					{
						Vector3 diff = particle->spawn_position - particle->last_spawn_position;
						m_collision_requests[evt_index].old_position = Vector3(0.0f, 0.0f, 0.0f);
						m_collision_requests[evt_index].new_position = diff;
						m_collision_requests[evt_index].base_position = particle->last_spawn_position;
					}
					else
					{
						m_collision_requests[evt_index].old_position = start_position;
						m_collision_requests[evt_index].new_position = particle->position;
						m_collision_requests[evt_index].base_position = particle->spawn_position;
					}
					m_collision_requests[evt_index].is_damage = false;
					m_collision_requests[evt_index].is_beam	= false;
					m_collision_requests[evt_index].area = Rect2D(-((frame->Origin.X + collision_offset_x) * scale_x * collision_scale_x), 
																  -((frame->Origin.Y + collision_offset_y) * scale_y * collision_scale_y), 
																  frame->Rect.Width * scale_x * collision_scale_x, 
																  frame->Rect.Height * scale_y * collision_scale_y);
					m_collision_requests[evt_index].collision_resolve_count = particle->collision_resolve_count;

					if (effect != NULL)
					{
						m_collision_requests[evt_index].effect_position = effect->position;
					}

					if (type->draw_method == ParticleFX_EmitterDrawMethod::Beam)
					{
						Vector3 beam_start = Vector3(particle->spawn_position.X, particle->spawn_position.Y, 0.0f);
						Vector3 beam_end   = particle->spawn_position + start_position;

						m_collision_requests[evt_index].area			= Rect2D(beam_start.X, beam_start.Y, beam_end.X, beam_end.Y);
						m_collision_requests[evt_index].base_position	= Vector3(0.0f, 0.0f, 0.0f);
						m_collision_requests[evt_index].new_position	= Vector3(0.0f, 0.0f, 0.0f);
						m_collision_requests[evt_index].old_position	= Vector3(0.0f, 0.0f, 0.0f);
						m_collision_requests[evt_index].is_beam			= true;
						m_collision_requests[evt_index].spawn_position	= particle->spawn_position;
					}

					particle->collision_last_center = Vector3(
						m_collision_requests[evt_index].base_position.X + m_collision_requests[evt_index].old_position.X + m_collision_requests[evt_index].area.X + (m_collision_requests[evt_index].area.Width * 0.5f),
						m_collision_requests[evt_index].base_position.Y + m_collision_requests[evt_index].old_position.Y + m_collision_requests[evt_index].area.Y + (m_collision_requests[evt_index].area.Height * 0.5f),
						m_collision_requests[evt_index].base_position.Z + m_collision_requests[evt_index].old_position.Z
					);

					particle->collision_last_angle = atan2(-(particle->position.X - start_position.X), particle->position.Y - start_position.Y);
					particle->using_collision = true;
				}
				else
				{
					particle->draw_position = particle->position;
				}
			}
		}
		else
		{
			particle->draw_position = particle->position;
		}

		// Add damage collision.
		if (type->enable_damage && can_do_collision)
		{
			float collision_scale_x = type->collision_scale_x;			// TODO: Blah this has no place here, remove.
			float collision_scale_y = type->collision_scale_y;
			float damage_offset_x = type->damage_offset_x * -sin(draw_dir);
			float damage_offset_y = type->damage_offset_y * cos(draw_dir);

			if (flip_h)
			{
				damage_offset_x = -damage_offset_x;
			}

			if (flip_v)
			{
				damage_offset_y = -damage_offset_y;
			}

			Rect2D col_box = Rect2D(
				-((frame->Origin.X + damage_offset_x) * scale_x * collision_scale_x), 
				-((frame->Origin.Y + damage_offset_y) * scale_y * collision_scale_y), 
				frame->Rect.Width * scale_x * collision_scale_x, 
				frame->Rect.Height * scale_y * collision_scale_y);

			// Request a collision based movement for the next frame plz.
			int evt_index = Atomic::Increment32(&m_simulation_collision_request_count) - 1;
			if (evt_index < max_collision_requests_per_frame)
			{
				m_collision_requests[evt_index].particle = particle;
				m_collision_requests[evt_index].incarnation = particle->incarnation;

				if (type->track_effect == true)
				{
					Vector3 diff = particle->spawn_position - particle->last_spawn_position;
					m_collision_requests[evt_index].old_position = Vector3(0.0f, 0.0f, 0.0f);
					m_collision_requests[evt_index].new_position = diff;
					m_collision_requests[evt_index].base_position = particle->last_spawn_position;
				}
				else
				{
					m_collision_requests[evt_index].old_position = start_position;
					m_collision_requests[evt_index].new_position = particle->position;
					m_collision_requests[evt_index].base_position = particle->spawn_position;
				}

				m_collision_requests[evt_index].is_damage = true;
				m_collision_requests[evt_index].is_beam	= false;
				m_collision_requests[evt_index].area = Rect2D((col_box.X + (col_box.Width * 0.5f)) - type->damage_radius, 
															  (col_box.Y + (col_box.Height * 0.5f)) - type->damage_radius, 
															  type->damage_radius * 2,  
															  type->damage_radius * 2);

				if (type->draw_method == ParticleFX_EmitterDrawMethod::Beam)
				{
					Vector3 beam_start = Vector3(particle->spawn_position.X, particle->spawn_position.Y, 0.0f);
					Vector3 beam_end   = particle->spawn_position + start_position;

					m_collision_requests[evt_index].area			= Rect2D(beam_start.X, beam_start.Y, beam_end.X, beam_end.Y);
					m_collision_requests[evt_index].base_position	= Vector3(0.0f, 0.0f, 0.0f);
					m_collision_requests[evt_index].new_position	= Vector3(0.0f, 0.0f, 0.0f);
					m_collision_requests[evt_index].old_position	= Vector3(0.0f, 0.0f, 0.0f);
					m_collision_requests[evt_index].is_beam			= true;
					m_collision_requests[evt_index].spawn_position	= particle->spawn_position;
				}

				m_collision_requests[evt_index].collision_resolve_count = particle->collision_resolve_count;
			}
		}

		// Check if we need to fire any timed events.
		//bool game_trigger_invoked = false;
		bool owner_destroyed = (effect == NULL || effect->owner == NULL || !effect->owner->Is_Valid());
		for (int i = 0; i < type->event_count; i++)
		{
			ParticleFX_Event& evt = type->events[i];
			
			// Fire timed events.
			if (evt.trigger == ParticleFX_EventTrigger::Time && evt.time <= delta && evt.time >= particle->last_update_delta)
			{
				int evt_index = Atomic::Increment32(&m_simulation_event_count) - 1;
				if (evt_index < max_events_per_frame)
				{
					m_simulation_events[evt_index].event	  = &evt;
					m_simulation_events[evt_index].particle   = particle;
				}
			}

			// Fire game triggers.
			else if (evt.trigger == ParticleFX_EventTrigger::OwnerTrigger && particle->invoke_game_trigger == true)
			{
				int evt_index = Atomic::Increment32(&m_simulation_event_count) - 1;
				if (evt_index < max_events_per_frame)
				{
					m_simulation_events[evt_index].event	  = &evt;
					m_simulation_events[evt_index].particle   = particle;
				//	game_trigger_invoked = true;
				}
			}

			// Fire owner-destroy triggers.
			else if (evt.trigger == ParticleFX_EventTrigger::OwnerDestroy && owner_destroyed == true)
			{
				int evt_index = Atomic::Increment32(&m_simulation_event_count) - 1;
				if (evt_index < max_events_per_frame)
				{
					m_simulation_events[evt_index].event	  = &evt;
					m_simulation_events[evt_index].particle   = particle;
				}
			}
		}

	/*	if (game_trigger_invoked)
		{
			particle->invoke_game_trigger = false;
			particle->invoke_game_trigger_delay = 0.0f;
		}
		*/

		if (particle->invoke_game_trigger == true)
		{
			particle->invoke_game_trigger_delay -= (elapsed_time / 1000.0f);
		}

		// Track effect.
		if (type->track_effect && effect != NULL)
		{
			particle->last_spawn_position  = particle->spawn_position ;
			particle->spawn_position  = effect->position;
			particle->spawn_direction = effect->direction;
		}

		/*DBG_LOG("<%s> pos=%f,%f dir=%f spawn_pos=%f,%f spawn_dir=%f force_kill=%i",
			particle->emitter_type->name.c_str(),
			particle->position.X, particle->position.Y, particle->direction,
			particle->spawn_position.X, particle->spawn_position.Y, particle->spawn_direction,
			force_kill);*/

		// Work out rough bb of the particle.
		float draw_pos_x = particle->spawn_position.X + particle->draw_position.X;
		float draw_pos_y = particle->spawn_position.Y + particle->draw_position.Y;
		float size_x = frame->Rect.Width * scale_x;
		float size_y = frame->Rect.Height * scale_y;
		float bb_size = Max(size_x, size_y);

		draw_pos_x += particle->emitter_type->draw_offset_x;
		draw_pos_y += particle->emitter_type->draw_offset_y;

		// Rough bounding box taking into account possible rotation.
		Rect2D bb = Rect2D(draw_pos_x - bb_size, draw_pos_y - bb_size, bb_size * 2.0f, bb_size * 2.0f);
		
		// Dump this particle into its appropriate buffer.
		ParticleBuffer* buffer = Get_Particle_Buffer(frame->TexturePtr->TexturePtr, type->render_pass_hash, type->blend_mode);

		switch (type->draw_method)
		{
		case ParticleFX_EmitterDrawMethod::Beam:
			{
				float base_pos_x = particle->spawn_position.X;//particle->draw_spawn_position.X;
				float base_pos_y = particle->spawn_position.Y;//particle->draw_spawn_position.Y;

				base_pos_x += particle->emitter_type->draw_offset_x;
				base_pos_y += particle->emitter_type->draw_offset_y;

				draw_pos_x = base_pos_x + particle->draw_position.X;
				draw_pos_y = base_pos_y + particle->draw_position.Y;

				Vector3 target = Vector3(draw_pos_x, draw_pos_y, 0.0f);//effect->target 

				// Calculate distance between position and target.
				Vector3 vec = target - Vector3(base_pos_x, base_pos_y, 0.0f);
				float angle = atan2(vec.Y, vec.X);
				float distance = vec.Length();

				// Work out distances for head/tail/middle.
				float unscaled_segment_height = frame->PixelBounds.Height * scale_y;
				float segment_height	      = unscaled_segment_height * scale_y;

				float head_length	= Min(segment_height, distance * 0.33f);
				float tail_length	= Min(segment_height, distance * 0.33f);
				float middle_length = Max(distance - (head_length + tail_length), distance * 0.33f);

				int middle_segments = (int)ceilf(middle_length / segment_height);

				int beam_frames = type->sprite_anim == NULL ? 1 : type->sprite_anim->Frames.size() / 3;

				// Draw quads from position to target.
				Atomic::Add32(&buffer->Required_Buffer_Count, middle_segments + 3);

				if (is_visible) // TODO: Bounding box check plz.
				{
					// Blah totally wrong if we want to sync with actors.
					float draw_pos_z = Actor::Calculate_Draw_Depth(Rect2D(draw_pos_x, draw_pos_y, 0.0f, 0.0f), m_simulation_camera_bounding_box, particle->spawn_position.Z + type->layer_offset, type->depth_bias);

					float distance_offset = 0;
					draw_pos_x = base_pos_x;
					draw_pos_y = base_pos_y;

					// Work out head frame.
					if (type->sprite_anim != NULL)
					{
						if (type->beam_use_seperate_tail_head)
						{
							frame = type->sprite_anim->Frames[sprite_frame % type->sprite_anim->Frames.size()];
						}
						else
						{
							frame = type->sprite_anim->Frames[sprite_frame % type->sprite_anim->Frames.size()];
						}
					}

					// Draw head.
					Write_Render_Particle(
						particle->incarnation, buffer, frame, 
						draw_pos_x, draw_pos_y, draw_pos_z, 
						angle - HALFPI, 
						color_r, color_g, color_b, color_a, 
						scale_x, head_length / unscaled_segment_height, 
						flip_h, flip_v);

					// Move position onwards.
					distance_offset = head_length;
					draw_pos_x = base_pos_x + (cos(angle) * distance_offset);
					draw_pos_y = base_pos_y + (sin(angle) * distance_offset);

					// Draw middle segments.
					float remaining_middle = middle_length;
					while (remaining_middle > 0.0f)
					{
						// Work out head frame.
						if (type->sprite_anim != NULL)
						{
							if (type->beam_use_seperate_tail_head)
							{
								frame = type->sprite_anim->Frames[(beam_frames + sprite_frame) % type->sprite_anim->Frames.size()];
							}
							else
							{
								sprite_frame++;
								frame = type->sprite_anim->Frames[sprite_frame % type->sprite_anim->Frames.size()];
							}
						}

						float this_segment_length = Min(segment_height, remaining_middle);						
						float distance_sub = 0.0f;//segment_height - (segment_height - this_segment_length);

						draw_pos_x = base_pos_x + (cos(angle) * (distance_offset - (distance_sub)));
						draw_pos_y = base_pos_y + (sin(angle) * (distance_offset - (distance_sub)));

						Write_Render_Particle(
							particle->incarnation, buffer, frame, 
							draw_pos_x, draw_pos_y, draw_pos_z, 
							angle - HALFPI, 
							color_r, color_g, color_b, color_a, 
							scale_x, this_segment_length / unscaled_segment_height, 
							flip_h, flip_v);

						distance_offset += this_segment_length;
						remaining_middle -= this_segment_length;
					}

					// Move position onwards.
					distance_offset = head_length + middle_length;
					draw_pos_x = base_pos_x + (cos(angle) * distance_offset);
					draw_pos_y = base_pos_y + (sin(angle) * distance_offset);

					// Work out head frame.
					if (type->sprite_anim != NULL)
					{
						if (type->beam_use_seperate_tail_head)
						{
							frame = type->sprite_anim->Frames[((beam_frames * 2) + sprite_frame) % type->sprite_anim->Frames.size()];
						}
						else
						{
							sprite_frame++;
							frame = type->sprite_anim->Frames[sprite_frame % type->sprite_anim->Frames.size()];
						}
					}

					// Draw tail.
					Write_Render_Particle(
						particle->incarnation, buffer, frame, 
						draw_pos_x, draw_pos_y, draw_pos_z, 
						angle - HALFPI, 
						color_r, color_g, color_b, color_a, 
						scale_x, tail_length / unscaled_segment_height, 
						flip_h, flip_v);
				}
			}
			break;
		case ParticleFX_EmitterDrawMethod::Sprite:
			{
				AtlasFrame* ml_parent_frame = frame;
				if (type->sprite_anim != NULL)
				{
					ml_parent_frame = type->sprite_anim->Frames[0];
				}

				unsigned int ml_frame_count = ml_parent_frame->MultiLayerFrames.size();

				// Figure out max number of sprites we may need.
				int quads_required = 1;
				if (type->draw_shadow == true)
				{
					quads_required++;
				}

				if (ml_frame_count > 0)
				{
					quads_required += (ml_frame_count - 1);
				}

				Atomic::Add32(&buffer->Required_Buffer_Count, quads_required);	

				// Cull particles that are offscreen. 
				if (/*m_simulation_camera_cull_bounding_box.Intersects(bb) &&*/ is_visible)
				{
					// Blah totally wrong if we want to sync with actors.
					float draw_pos_z = Actor::Calculate_Draw_Depth(Rect2D(draw_pos_x, draw_pos_y, 0.0f, 0.0f), m_simulation_camera_bounding_box, particle->spawn_position.Z + type->layer_offset, type->depth_bias);

					// Draw shadow.
					if (type->draw_shadow == true)
					{
						float shadow_reduction = particle->height / PARTICLE_SHADOW_REDUCTION_MAX_HEIGHT;
						float shadow_scale = 1.0f - shadow_reduction;

						Write_Render_Particle(particle->incarnation, buffer, frame, draw_pos_x, draw_pos_y, draw_pos_z - (MAP_LAYER_DEPTH * 0.5f), draw_dir, 0.0f, 0.0f, 0.0f, (color_a * 0.5f) * shadow_scale, scale_x * shadow_scale, scale_y * shadow_scale, flip_h, flip_v);
					}

					// Draw each multi-layer.
					if (ml_frame_count > 0 && type->sprite_anim != NULL)
					{
						for (unsigned int i = 0; i < ml_frame_count; i++)
						{
							AtlasMultiLayerFrame& ml_frame = ml_parent_frame->MultiLayerFrames.at(i);
							AtlasFrame* sub_frame = type->sprite_anim->Frames[(sprite_frame + ml_frame.FrameOffset) % type->sprite_anim->Frames.size()];

							// TODO: Fix
							// Right so this is not *correct* but its how we want it to render. Blah, at some point we might want to cahnge all this multi layer rendering BS.
 							//float sub_draw_pos_z = Actor::Calculate_Draw_Depth(Rect2D(draw_pos_x, draw_pos_y, 0.0f, 0.0f), m_simulation_camera_bounding_box, particle->spawn_position.Z + type->layer_offset + ml_frame.LayerOffset, type->depth_bias);
							float sub_draw_pos_z = Actor::Calculate_Draw_Depth(Rect2D(draw_pos_x, draw_pos_y, 0.0f, 0.0f), m_simulation_camera_bounding_box, particle->spawn_position.Z + type->layer_offset, type->depth_bias + (ml_frame.LayerOffset * sub_frame->Rect.Height));

							Write_Render_Particle(particle->incarnation, buffer, sub_frame, draw_pos_x, draw_pos_y - particle->height, sub_draw_pos_z, draw_dir, color_r, color_g, color_b, color_a, scale_x, scale_y, flip_h, flip_v);
						}
					}
					else
					{
						Write_Render_Particle(particle->incarnation, buffer, frame, draw_pos_x, draw_pos_y - particle->height, draw_pos_z, draw_dir, color_r, color_g, color_b, color_a, scale_x, scale_y, flip_h, flip_v);
					}
				}

				break;
			}

		case ParticleFX_EmitterDrawMethod::Numbers:
			{
				int number = effect == NULL ? 0 : abs(effect->meta_number);
				int digits = number == 0 ? 1 : (1 + (int)floorf(log10f((float)number)));

				DBG_ASSERT(digits >= 0);
				Atomic::Add32(&buffer->Required_Buffer_Count, digits);	

				// Cull particles that are offscreen. 
				if (m_simulation_camera_cull_bounding_box.Intersects(bb) && is_visible)
				{
					// Blah totally wrong if we want to sync with actors.
					float draw_pos_z = Actor::Calculate_Draw_Depth(Rect2D(draw_pos_x, draw_pos_y, 0.0f, 0.0f), m_simulation_camera_bounding_box, particle->spawn_position.Z + type->layer_offset, type->depth_bias);// particle->spawn_position.Z + (type->layer_offset * MAP_LAYER_DEPTH) + Get_Draw_Depth(draw_pos_y) + type->depth_bias;

					float full_width = size_x * digits;
					float start_x = (draw_pos_x + (full_width * 0.5f)) - size_x;

					for (int i = 0; i < digits; i++)
					{
						int num = number % 10;
						
						if (type->sprite_anim != NULL)
						{
							frame = type->sprite_anim->Frames[num % type->sprite_anim->Frames.size()];
						}

						Write_Render_Particle(particle->incarnation, buffer, frame, start_x, draw_pos_y - particle->height, draw_pos_z, draw_dir, color_r, color_g, color_b, color_a, scale_x, scale_y, flip_h, flip_v);
						
						number /= 10;
						start_x -= size_x;
					}
				}

				break;
			}
		}

		particle->velocity_affected_last_frame = false;
		particle->last_update_delta = delta;
		particle->is_alive_flag = !force_kill;
	}

	// Keep emitter alive till we are dead :3.
	if (emitter != NULL && type->keep_effect_alive)
	{
		//DBG_LOG("Alive[%i] %s lifetime=%f", particle_index, type->name.c_str(), particle->lifetime)
		//DBG_LOG("Being kept alive by %s (lifetime %.2f)", emitter->type->name.c_str(), particle->lifetime);
		emitter->is_alive_flag = true;	
	}
}

void ParticleManager::Simulate_Effect(float elapsed_time, ParticleEffect* effect)
{
	static const float step_size = 1000.0f / 60.0f;
	
	float frame_elapsed	= elapsed_time;
	bool any_alive = false;

	effect->was_simulated = true;

	// Track owning particle?
	bool owner_dead = false;
	if (effect->owner != NULL)
	{
		if (!effect->owner->Is_Valid())
		{
 			owner_dead = true;
		}
		else
		{
			ParticleInstance* pi = effect->owner->Get();

			if (pi->using_collision && pi->emitter_type->draw_method != ParticleFX_EmitterDrawMethod::Beam)
			{
				effect->position = pi->collision_last_center + Vector3(0.0f, -pi->height, 0.0f);
				effect->direction = pi->collision_last_angle;
			}
			else
			{
				effect->position = pi->spawn_position + pi->position + Vector3(0.0f, -pi->height, 0.0f);
				effect->direction = pi->spawn_direction + pi->direction;
			}
		}
	}

	/*if (effect->type->Get_Name() == "projectile_light_small")
	{
		DBG_LOG("Projectile light still alive. owner_dead=%i owner=0x%08x", owner_dead, effect->owner);
	}*/

	// Update each emitter.
	for (int i = 0; i < effect->emitter_count; i++)
	{
		ParticleEmitter& emitter = *effect->emitters[i];

		if (emitter.type->cycle_count == 0)
		{
			if (owner_dead)
			{
				//DBG_LOG("OWNER DEAD fucking sigh - %s.", emitter.type->name.c_str());
			}
			else
			{
				emitter.is_alive_flag = true;
			}
		}

		if (emitter.effect->is_paused)
		{
			emitter.is_alive_flag = true;
			continue;
		}

		// Accumulate simulation time.
		emitter.accumulated += frame_elapsed;

		// Use up as much simulation time as we can.
		int	  cycle_index	= 0;
		float cycle_elapsed = 0.0f;
		float cycle_left	= 0.0f;
		float step			= 0.0f;
		float cycle_delta	= 0.0f;
		float force_delta	= 0.0f;

		float cycle_length_ms = emitter.type->cycle_length * 1000;
		
		while (true)
		{	
			if (force_delta != 0.0f)
			{
				cycle_delta = force_delta;
				step = 0.0f;
			}
			else
			{
				// Work out what cycle we are on and how much time has elapsed on it.
				cycle_index	= (int)(emitter.life_elapsed / cycle_length_ms);
				cycle_elapsed = emitter.life_elapsed - (cycle_index * cycle_length_ms);

				// How much time is left of cycle?
				cycle_left	= cycle_length_ms - cycle_elapsed;

				// Work out step cycle. Step cycle should never go over time left of cycle, or we will never
				// trigger events and such that happen at 1.0f;
				step = Min(emitter.accumulated, Min(step_size, cycle_left));
			
				// Work out cycle delta.
				cycle_delta	= cycle_elapsed / cycle_length_ms;
			}

			// Still alive?
			if (emitter.type->cycle_count == 0 || cycle_index < emitter.type->cycle_count)
			{
				if (emitter.spawns_this_cycle == 0 && emitter.type->cycle_count == 1)
				{
					emitter.is_alive_flag = true;
				}

				if (emitter.cycle_index != cycle_index)
				{
					emitter.spawns_this_cycle = 0;
					emitter.cycle_index = cycle_index;
				}

				// Spawn some particles.
				float spawn_rate  = emitter.type->curves[ParticleFX_CurveType::Spawn_Rate].Get(cycle_delta, emitter.spawn_rate_range) / 1000.0f;
				float spawn_count = step * spawn_rate;

				emitter.spawns_accumulated += spawn_count;

				// Spread spawns over position delta.
				float spawn_delta_offset = 1.0f / ((int)emitter.spawns_accumulated);
				float spawn_delta = spawn_delta_offset;
				bool has_spawned = (emitter.spawns_accumulated >= 1.0f);

				while (emitter.spawns_accumulated >= 1.0f && 
					   owner_dead == false &&
					   (emitter.spawns_this_cycle < emitter.type->max_particles_per_cycle || emitter.type->max_particles_per_cycle == 0) &&
					   m_particle_count < max_particles && 
					   (emitter.type->current_particle_count < emitter.type->global_max_particles || emitter.type->global_max_particles == 0))
				{
					// Remove particle from accumulator.
					emitter.spawns_accumulated -= 1.0f;
					emitter.spawns_this_cycle++;

					// Skip particles based on gfx effect quality.
					if (m_skip_modulus > 1)
					{
						// Only allow skipping of non-damage-causing effects.
						if (emitter.type->can_cull)
						{
							int counter = m_skip_modulus_counter++;
							if (counter >= m_skip_modulus)
							{
								m_skip_modulus_counter = 0;
							}
							else
							{
								spawn_delta += spawn_delta_offset;
								continue;
							}
						}
					}

					ParticleFX_Curve_Range lifetime_range(&emitter.type->curves[(int)ParticleFX_CurveType::Lifetime]);
					ParticleFX_Curve_Range rotation_range(&emitter.type->curves[(int)ParticleFX_CurveType::Rotation]);
					ParticleFX_Curve_Range direction_range(&emitter.type->curves[(int)ParticleFX_CurveType::Direction]);
					ParticleFX_Curve_Range height_range(&emitter.type->curves[(int)ParticleFX_CurveType::Height]);
					ParticleFX_Curve_Range damage_range(&emitter.type->curves[(int)ParticleFX_CurveType::Damage]);

					// Interpolate position based on last position we spawned at.
					Vector3 spawn_pos		= emitter.last_spawn_position.Lerp(effect->position, spawn_delta);
					float   spawn_dir		= Math::Lerp(emitter.last_spawn_direction, effect->direction, spawn_delta);
					float	spawn_height	= emitter.type->curves[ParticleFX_CurveType::Height].Get(cycle_delta, height_range);
					float   dir_varience	= DegToRad(emitter.type->curves[ParticleFX_CurveType::Direction].Get(cycle_delta, direction_range));
					float	start_dir		= spawn_dir + dir_varience;

					// Upgrade modifier.
					if (emitter.type->is_projectile)
					{
						if (emitter.effect->upgrade_modifiers[(int)ParticleUpgradeModifiers::Accuracy] != 0.0f)
						{
							float mod = Max(emitter.effect->upgrade_modifiers[(int)ParticleUpgradeModifiers::Accuracy], -0.95f);
							float modifier = 1.0f / (1.0f + mod);
							float variance = dir_varience * modifier;
							start_dir = spawn_dir + variance;
						}
					}

					// Offset spawn position.
					spawn_pos.X += (float)(cos(start_dir + HALFPI) * emitter.type->spawn_offset_x);
					spawn_pos.Y += (float)(sin(start_dir + HALFPI) * emitter.type->spawn_offset_x);
					spawn_pos.X += (float)(cos(start_dir) * emitter.type->spawn_offset_y);
					spawn_pos.Y += (float)(sin(start_dir) * emitter.type->spawn_offset_y);

				/*	if (emitter.type->name == "Projectile")
					{
						DBG_LOG("SPAWNING %s at spawn=%f,%f effect=%f,%f demo=%i cycle=%i spawns=%i",
							emitter.type->name.c_str(),
							spawn_pos.X, spawn_pos.Y, 
							effect->position.X, effect->position.Y,
							effect->demo_id,
							emitter.cycle_index,
							emitter.spawns_this_cycle);
					}*/

					if (emitter.type->offset_by_height)
					{
						spawn_pos.Y += spawn_height;
					}

					int particle_index = Atomic::Increment32(&m_particle_count) - 1;
					if (particle_index >= max_particles)
					{
						Atomic::Decrement32(&m_particle_count);
						continue;
					}
					Atomic::Increment32(&emitter.type->current_particle_count); 

					ParticleInstance& instance = m_particles[particle_index]; 	
					instance.is_alive_flag				= true;
					instance.last_sound					= NULL;
					instance.incarnation				= Atomic::Increment32(&m_incarnation_index);
					instance.is_alive					= true;
					instance.is_owner					= false;
					instance.emitter_instance			= &emitter;
					instance.emitter_type				= emitter.type;
					instance.alive_time					= 0.0f;
					instance.spawn_direction			= start_dir;
					instance.spawn_position				= spawn_pos;
					instance.draw_spawn_position		= spawn_pos;
					instance.position					= Vector3(0,0,0);
					instance.draw_position				= instance.position;
					instance.velocity_scale				= Vector2(1.0f, 1.0f);
					instance.last_spawn_position		= Vector3(0.0f, 0.0f, 0.0f);
					instance.demo_instigator_team		= emitter.demo_instigator_team;
					instance.direction					= 0.0f;
					instance.collision_resolve_count	= 0;
					instance.height						= spawn_height;
					instance.height_velocity			= 0.0f;
					instance.height_velocity_scale		= 0.0f;
					instance.neighbour_exists			= false;
					instance.rotation					= DegToRad(emitter.type->curves[ParticleFX_CurveType::Rotation].Get(cycle_delta, rotation_range));
					instance.last_update_delta			= 0.0f;
					instance.bouncing					= emitter.type->allow_bouncing;//true;
					instance.using_collision			= false;
					instance.spawned_in_collision		= 0;
					instance.checked_spawn_collision	= false;
					instance.ignore_spawn_collision		= effect->ignore_spawn_collision;
					instance.invoke_game_trigger		= false;
					instance.invoke_game_trigger_delay	= 0.0f;
					instance.is_penetrating				= false;
					instance.was_penetrating			= false;
					instance.sound_handle.manager		= NULL;
					instance.ricochet_count				= 0;
					instance.penetration_count			= 0;
					instance.subtype					= emitter.effect->subtype;
					instance.weapon_type				= emitter.effect->weapon_type;
					instance.instigator					= emitter.effect->instigator;
					instance.instigator_incarnation		= emitter.effect->instigator_incarnation;
					instance.damage						= emitter.type->curves[ParticleFX_CurveType::Damage].Get(cycle_delta, damage_range);
					instance.lifetime					= emitter.type->curves[ParticleFX_CurveType::Lifetime].Get(cycle_delta, lifetime_range);
					instance.sprite_frame_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Sprite_Frame]);
					instance.velocity_x_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Velocity_X]);
					instance.velocity_y_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Velocity_Y]);
					instance.velocity_height_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Velocity_Height]);
					instance.velocity_rotation_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Velocity_Rotation]);
					instance.velocity_direction_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Velocity_Direction]);
					instance.color_r_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Color_R]);
					instance.color_g_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Color_G]);
					instance.color_b_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Color_B]);
					instance.color_a_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Color_A]);
					instance.scale_x_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Scale_X]);
					instance.scale_y_range.Randomise(&emitter.type->curves[(int)ParticleFX_CurveType::Scale_Y]);

					// Upgrade modifier.
					if (emitter.effect->upgrade_modifiers[(int)ParticleUpgradeModifiers::Damage] != 0.0f)
					{
						instance.damage *= (1.0f + emitter.effect->upgrade_modifiers[(int)ParticleUpgradeModifiers::Damage]);
					}
					if (emitter.type->is_projectile && emitter.effect->upgrade_modifiers[(int)ParticleUpgradeModifiers::ProjectileSpeed] != 0.0f)
					{
						float modifier = (1.0f + emitter.effect->upgrade_modifiers[(int)ParticleUpgradeModifiers::ProjectileSpeed]);
						instance.velocity_scale.X *= modifier;
						instance.velocity_scale.Y *= modifier;
					}

				//	DBG_LOG("Spawned[%i] %s lifetime=%f cycle_delta=%f range=%f,%f", particle_index, emitter.type->name.c_str(), instance.lifetime, cycle_delta, lifetime_range.start, lifetime_range.end)

					//DBG_LOG("Spawned from emitter %i", instance.emitter_index);
					
					// Spawn shape offset.
					switch (emitter.type->spawn_shape)
					{
					case ParticleFX_SpawnShape::Circle:
						{
							float radius = (float)Random::Static_Next_Double(emitter.type->spawn_inner_radius, emitter.type->spawn_outer_radius);
							float dir = (float)Random::Static_Next_Double(0.0f, PI2);
							instance.spawn_position.X += (float)(sin(dir) * radius);
							instance.spawn_position.Y += (float)(cos(dir) * radius);
							break;
						}
					case ParticleFX_SpawnShape::Rect:
						{
							instance.spawn_position.X += (float)Random::Static_Next_Double(-emitter.type->spawn_outer_radius, emitter.type->spawn_outer_radius);
							instance.spawn_position.Y += (float)Random::Static_Next_Double(-emitter.type->spawn_outer_radius, emitter.type->spawn_outer_radius);
							break;
						}
					}

					/*DBG_LOG("<spawn %s> effect_pos=%f,%f effect_dir=%f pos=%f,%f dir=%f spawn_pos=%f,%f spawn_dir=%f", 
						emitter.type->name.c_str(),
						effect->position.X, effect->position.Y, effect->direction,
						instance.position.X, instance.position.Y, instance.direction, 
						instance.spawn_position.X, instance.spawn_position.Y, instance.spawn_direction);
					*/	

					// Increase spawn delta.
					spawn_delta += spawn_delta_offset;
				}

				if (has_spawned)
				{
					emitter.last_spawn_position = effect->position;
					emitter.last_spawn_direction = effect->direction;
				}
			}

			// Add last step to accumulator/life-elapsed.
			emitter.accumulated -= step;
			emitter.life_elapsed += step;

			// Break out if we have no accumulated time left.
			// If step < step_size then we have lapsed a cycle and need to run
			// again so we get a 1.0f delta update.
			if (step < step_size && force_delta == 0.0f)
			{
				force_delta = 1.0f;
			}
			else if (emitter.accumulated <= 0.0f || force_delta != 0.0f)
			{
				break;
			}
		}
	}
}

void ParticleManager::Simulate(int worker_index)
{		
	{
		PROFILE_SCOPE("Simulate Emitters");

		// Simulate our batch of emitters.
		int emitters_per_worker = m_simulate_effect_count / m_worker_count;
		int start_index			= emitters_per_worker * worker_index;
		int end_index			= start_index + emitters_per_worker;

		if (worker_index == m_worker_count - 1)
		{
			end_index += m_simulate_effect_count % m_worker_count;
		}

		for (int i = start_index; i < end_index; i++)
		{
			ParticleEffect* effect = m_effects[i];
			Simulate_Effect(m_simulate_elapsed_time, effect);
		}
	}
	
	{
		PROFILE_SCOPE("Simulate Particles");

		// Simulate our batch of particles.
		int particles_per_worker = m_simulate_particle_count / m_worker_count;
		int start_index			 = particles_per_worker * worker_index;
		int end_index			 = start_index + particles_per_worker;

		if (worker_index == m_worker_count - 1)
		{
			end_index += m_simulate_particle_count % m_worker_count;
		}

		//DBG_LOG("Worker[%i] START:%i END:%i", worker_index, start_index, end_index);

		for (int i = start_index; i < end_index; i++)
		{
			ParticleInstance* particle = &m_particles[i];
			Simulate_Particle(m_simulate_elapsed_time, m_simulate_delta_t, particle, i, worker_index);
		}
	}

	// Wait for all workers to finish.
//	DBG_LOG("%i Simulated", worker_index);
	if (m_simulate_blocker->Wait())
	{
		// Setup for sort if we are last worker to finish.
		m_render_particle_count = Min(m_render_particle_count, max_render_particles);
		for (int i = 0; i < m_render_particle_count; i++)
		{
			m_render_particle_buffer_sorted[i] = &m_render_particle_buffer[i];
		}

		m_simulate_blocker->Continue();
	} 

	// Sort our segment of the render particles.
	{
		PROFILE_SCOPE("Partial Sort");

		int particles_per_worker = m_render_particle_count / m_worker_count;
		int start_index			 = particles_per_worker * worker_index;
		int end_index			 = start_index + particles_per_worker;

		if (worker_index == m_worker_count - 1)
		{
			end_index += m_render_particle_count % m_worker_count;
		}

		//DBG_LOG("Sorting from %i to %i", start_index, end_index);
		std::sort(m_render_particle_buffer_sorted + start_index, m_render_particle_buffer_sorted + end_index, &RenderParticle::Sort_Particle_Predicate);
	}

	// Wait for all workers to finish.
	//DBG_LOG("Worker %i waiting", worker_index);
	if (m_sort_blocker->Wait())
	{
		//DBG_LOG("Final sort starting.");

		PROFILE_SCOPE("Final Sort");
		{
			// Merge all the sorted lists together plz.

			int worker_read_indexes[max_workers];
			int worker_end_indexes[max_workers];

			int write_ptr = 0;

			for (int i = 0; i < m_worker_count; i++)
			{
				int particles_per_worker = m_render_particle_count / m_worker_count;
				int start_index			 = particles_per_worker * i;
				int end_index			 = start_index + particles_per_worker;

				if (i == m_worker_count - 1)
				{
					end_index += m_render_particle_count % m_worker_count;
				}

				worker_read_indexes[i] = start_index;
				worker_end_indexes[i]  = end_index;
			}

			for (int i = 0; i < m_render_particle_count; i++)
			{
				RenderParticle* particle_ptr = NULL;
				int worker_idx = -1;

				// Find furthest particle.
				for (int j = 0; j < m_worker_count; j++)
				{
					int read_index = worker_read_indexes[j];
					int end_index = worker_end_indexes[j];
					if (read_index < end_index)
					{
						RenderParticle* worker_particle = m_render_particle_buffer_sorted[read_index];
						if (particle_ptr == NULL || RenderParticle::Sort_Particle_Predicate(worker_particle, particle_ptr))
						{
							particle_ptr = worker_particle;
							worker_idx = j;
						}
					}
				}

				// Calculate render buffer index.
				// No need to do this atomically we should be the only worker atm.
				//particle_ptr->render_index = Atomic::Increment32(&particle_ptr->buffer->Particle_Count) - 1;
				particle_ptr->render_index = particle_ptr->buffer->Particle_Count++;

				// Write to final array.
				DBG_ASSERT(particle_ptr != NULL);
				m_render_particle_buffer_final_sorted[i] = particle_ptr;
				worker_read_indexes[worker_idx]++;			
			}
		}

	//	DBG_LOG("Final sort finished.");

		m_final_sort_complete = true;
		m_sort_blocker->Continue();
	}

	//DBG_LOG("Worker %i finished waiting", worker_index);
	DBG_ASSERT_STR(m_final_sort_complete == true, "Worker %i somehow got ahead of final sort!", worker_index);

	// TODO:
	//	Run Emitter Event - Runs a disabled emitter at the particles position. Better than a billion emitters.
	//	Limit max workers to worker task count.

	//static int last_values[max_particles];

	// Write out render particles.
	{
		PROFILE_SCOPE("Write Render Particles");
		
		// Simulate our batch of particles.
		int particles_per_worker = m_render_particle_count / m_worker_count;
		int start_index			 = particles_per_worker * worker_index;
		int end_index			 = start_index + particles_per_worker;

		if (worker_index == m_worker_count - 1)
		{
			end_index += m_render_particle_count % m_worker_count;
		}

		DBG_ASSERT_STR(m_render_particle_count == 0 || end_index <= m_render_particle_count, "Invalid end_index %f/%f", end_index, m_render_particle_count);

		for (int i = start_index; i < end_index; i++)
		{
			RenderParticle* particle = m_render_particle_buffer_final_sorted[i];
			//Atomic::Increment32(&m_particles_rendered);

			particle->buffer->Write(
				particle->render_index,
				particle->frame, 
				particle->x, 
				particle->y, 
				particle->z, 
				particle->angle, 
				particle->red,
				particle->green,
				particle->blue,
				particle->alpha,
				particle->scale_x,
				particle->scale_y,
				particle->flip_h,
  				particle->flip_v);
		}
	}		

//	Atomic::Increment32(&m_workers_run);
//	DBG_LOG("%i Done", worker_index);		
}

void ParticleManager::Reset_Particle_Sounds()
{
	for (int i = 0; i < m_sound_count; i++)
	{
		if (m_sounds[i]->channel != NULL)
		{
			SAFE_DELETE(m_sounds[i]->channel);
		}
	}

	m_sound_count = 0;

	for (int i = 0; i < max_sounds; i++)
	{
		m_sounds[i] = &m_sounds_mem[i];
		Atomic::Increment32(&m_sounds[i]->incarnation);

		m_sounds[i]->kill_flag = false;
	}
}

ParticleInstanceHandle* ParticleManager::Alloc_Particle_Instance_Handle(ParticleInstance* instance)
{
	if (m_instance_handle_count >= max_instance_handles)
	{
		return NULL;
	}

	ParticleInstanceHandle* handle = m_instance_handles[m_instance_handle_count++];
	handle->incarnation = instance->incarnation;
	handle->m_reset_count = m_reset_count;
	handle->instance = instance;
	handle->index = m_instance_handle_count - 1;

	instance->is_owner = true;

	return handle;
}

void ParticleManager::Free_Particle_Instance_Handle(ParticleInstanceHandle* handle)
{
	// If we have reset since alloc, this handle is not valid.
	if (handle == NULL || m_reset_count != handle->m_reset_count)
	{
		return;
	}

	DBG_ASSERT(m_instance_handle_count > 0);

	// TODO: FIX, can go to -1 if we reset then call this D:
	ParticleInstanceHandle* tmp = m_instance_handles[m_instance_handle_count - 1];
	m_instance_handles[--m_instance_handle_count] = m_instance_handles[handle->index];
	m_instance_handles[handle->index] = tmp;
	tmp->index = handle->index;
}

void ParticleManager::Update_Particle_Instance_Handle(ParticleInstance* old_location, ParticleInstance* new_location)
{
	for (int i = 0; i < m_instance_handle_count; i++)
	{
		ParticleInstanceHandle* handle = m_instance_handles[i];
		if (handle->instance == old_location)
		{
			handle->instance = new_location;
		}
	}
}

ParticleInstigatorHandle* ParticleManager::Alloc_Instigator_Handle(void* instigator)
{
	if (m_instance_handle_count >= max_instigator_handles)
	{
		return NULL;
	}

	ParticleInstigatorHandle* handle = m_instigator_handles[m_instigator_handle_count++];
	handle->m_incarnation++;
	handle->m_reset_count = m_reset_count;
	handle->m_instigator = instigator;
	handle->m_index = m_instigator_handle_count - 1;

	return handle;
}

void ParticleManager::Free_Instigator_Handle(ParticleInstigatorHandle* handle)
{
	// If we have reset since alloc, this handle is not valid.
	if (handle == NULL || m_reset_count != handle->m_reset_count)
	{
		return;
	}

	handle->m_incarnation++;

	DBG_ASSERT(m_instigator_handle_count > 0);

	// TODO: FIX, can go to -1 if we reset then call this D:
	ParticleInstigatorHandle* tmp = m_instigator_handles[m_instigator_handle_count - 1];
	m_instigator_handles[--m_instigator_handle_count] = m_instigator_handles[handle->m_index];
	m_instigator_handles[handle->m_index] = tmp;
	tmp->m_index = handle->m_index;
}

ParticleSoundHandle ParticleManager::Alloc_Particle_Sound(SoundHandle* handle)
{
	ParticleSoundHandle result;
	result.manager = this;
	result.incarnation = -1;
	result.sound = NULL;

	SoundChannel* channel = handle->Get()->Allocate_Channel();
	if (channel != NULL)
	{
		while (true)
		{
 			int count = m_sound_count;
			if (count >= max_sounds)
			{
				break;
			}

			int r = Atomic::CompareExchange32(&m_sound_count, count, count + 1);
			if (r == count)
			{
				ParticleSound* sound = m_sounds[r];
				Atomic::Increment32(&sound->incarnation);

				sound->kill_flag = false;
				sound->channel = channel;
				handle->Get()->Play(sound->channel, Vector3(0.0f, 0.0f, 0.0f), false);

				result.incarnation = sound->incarnation;
				result.sound = sound;

				//DBG_LOG("Allocated sound 0x%08x (now %i sounds)", sound, m_sound_count);

				break;
			}
		}
	}

	// Couldn't allocate space :(
	if (result.sound == NULL)
	{
		SAFE_DELETE(channel);
	}

	return result;
}

ParticleSimulationTask::ParticleSimulationTask(int index)
	: m_worker_index(index)
{
}

void ParticleSimulationTask::Run()
{
	ParticleManager* manager = ParticleManager::Get();
	manager->Simulate(m_worker_index);
}	

INLINE bool ParticleSoundHandle::Is_Valid()
{
	if (manager == NULL || sound == NULL)
	{
		return false;
	}

	return (sound->incarnation == incarnation);
}

INLINE void ParticleSoundHandle::Dispose()
{
	sound->kill_flag = true;
	//DBG_LOG("Marking sound 0x%08x for delete", sound);
}

INLINE void ParticleSoundHandle::Update(const Vector3& pos)
{
	sound->channel->Set_Spatial_Position(pos);
}

void ParticleManager::Update_Demo_Properties(ParticleEffect* effect)
{
	if (effect->instigator)
	{
		void* instigator = effect->instigator->Get_Instigator();
		if (instigator && effect->instigator_incarnation == effect->instigator->Get_Incarnation())
		{
			int team_index = GameEngine::Get()->Get_Scene()->Get_Particle_Instigator_Team_Index(instigator);
			effect->demo_instigator_team = team_index;

			for (int i = 0; i < effect->emitter_count; i++)
			{
				effect->emitters[i]->demo_instigator_team = team_index;
			}
		}
	}
}