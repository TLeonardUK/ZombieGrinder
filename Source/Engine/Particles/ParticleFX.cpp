// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Particles/ParticleFXFactory.h"
#include "Engine/Particles/ParticleFX.h"
#include "Engine/Particles/ParticleFXHandle.h"

#include "Engine/Renderer/Atlases/Atlas.h"

#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIElement.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Config/ConfigFile.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/Math/Random.h"

#include <typeinfo>

ParticleFX::ParticleFX()
{
}

ParticleFX::~ParticleFX()
{
}

void ParticleFX_Curve_Range::Randomise(ParticleFX_Curve* curve)
{
	start = (float)Random::Static_Next_Double(curve->start_min, curve->start_max);
	end   = (float)Random::Static_Next_Double(curve->end_min, curve->end_max);
}

void ParticleFX_Curve::Precalculate_Curve(std::vector<ParticleFX_CurvePoint> points, float* output, int output_size)
{
	is_static = true;
	float last_value = 0.0f;

	for (int i = 0; i <= output_size; i++)
	{
		float time = (1.0f / (output_size)) * i;

		ParticleFX_CurvePoint* before = NULL;
		ParticleFX_CurvePoint* after = NULL;

		// Find point below/at us.
		for (std::vector<ParticleFX_CurvePoint>::iterator iter = points.begin(); iter != points.end(); iter++)
		{
			ParticleFX_CurvePoint& point = *iter;
			if (point.time <= time)
			{
				before = &point;
			}
		}

		// Find point infront of us/at us.
		for (std::vector<ParticleFX_CurvePoint>::iterator iter = points.begin(); iter != points.end(); iter++)
		{
			ParticleFX_CurvePoint& point = *iter;
			if (point.time > time)
			{
				after = &point;
				break;
			}
		}

		if (after == NULL)
			after = before;

		if (after == NULL || before == NULL)
		{
			output[i] = 0.0f;
			continue;
		}

		// Find delta.
		float time_range = (after->time - before->time);
		float time_delta = time_range == 0.0f ? 0 : (time - before->time) / time_range;

		float value_range = (after->value - before->value);
		float value = before->value + (value_range * time_delta);
		
		output[i] = value;

		if (value != last_value && i != 0)
		{
			is_static = false;
		}
		last_value = value;
	}

	static_value = last_value;
}

bool ParticleFX::Load_Elements(ConfigFile* config)
{
	// Load settings.
	m_name = config->Get<std::string>("settings/name");
	m_name_hash = StringHelper::Hash(m_name.c_str());

#define GET_SETTING(type, name, def_value, node)	(config->Get_Node(name, false, false, node, false) == NULL	? def_value : config->Get<type>(name, node))
#define GET_ATTRIBUTE(type, name, def_value, node)	(config->Contains(name, node, true) == false				? def_value : config->Get<type>(name, node, true))
	
	// Load each emitter.
	std::vector<ConfigFileNode> emitter_nodes = config->Get<std::vector<ConfigFileNode> >("emitters/emitter");
	for (std::vector<ConfigFileNode>::iterator iter = emitter_nodes.begin(); iter != emitter_nodes.end(); iter++)
	{
		if (m_emitters.size() >= max_emitters)
			continue;

		ConfigFileNode emitter_node = *iter;

		ParticleFX_EmitterConfig emitter;
		emitter.name						= GET_SETTING(std::string,	"settings/name",						"Untitled FX",		emitter_node);
		emitter.render_pass					= GET_SETTING(std::string,	"settings/render_pass",					"geometry",			emitter_node);
		emitter.keep_effect_alive			= GET_SETTING(bool,			"settings/keep_effect_alive",			true,				emitter_node);		
		emitter.lock_rotation_to_direction	= GET_SETTING(bool,			"settings/lock_rotation_to_direction",	false,				emitter_node);
		emitter.global_max_particles		= GET_SETTING(int,			"settings/global_max_particles",		0,					emitter_node);
		emitter.draw_method					= ParticleFX_EmitterDrawMethod::Parse(GET_SETTING(const char*, "settings/draw_method", "Sprite", emitter_node));		
		emitter.blend_mode					= ParticleFX_EmitterBlendMode::Parse(GET_SETTING(const char*, "settings/blend_mode", "Alpha", emitter_node));		
		emitter.cycle_length				= GET_SETTING(float,		"settings/cycle_length",				1.0f,				emitter_node);
		emitter.cycle_count					= GET_SETTING(float,		"settings/cycle_count",					1.0f,				emitter_node);
		emitter.max_particles_per_cycle		= GET_SETTING(int,			"settings/max_particles_per_cycle",		0,					emitter_node);	
		const char* sprite_name				= GET_SETTING(const char*,	"settings/sprite",						"",					emitter_node);
		emitter.sprite_anim					= ResourceFactory::Get()->Get_Atlas_Animation(sprite_name);
		emitter.sprite_frame				= ResourceFactory::Get()->Get_Atlas_Frame(sprite_name);
		emitter.affected_by_attractors		= GET_SETTING(bool,			"settings/affected_by_attractors",		false,				emitter_node);
		emitter.move_based_on_direction		= GET_SETTING(bool,			"settings/move_based_on_direction",		false,				emitter_node);			
		emitter.enable_collision			= GET_SETTING(bool,			"settings/enable_collision",			false,				emitter_node);
		emitter.collision_response			= ParticleFX_CollisionResponse::Parse(GET_SETTING(const char*, "settings/collision_reponse", "Bounce", emitter_node));		
		emitter.collision_scale_x			= GET_SETTING(float,		"settings/collision_scale_x",			1.0f,				emitter_node);		
		emitter.collision_scale_y			= GET_SETTING(float,		"settings/collision_scale_y",			1.0f,				emitter_node);		
		emitter.collision_offset_x			= GET_SETTING(float,		"settings/collision_offset_x",			0.0f,				emitter_node);		
		emitter.collision_offset_y			= GET_SETTING(float,		"settings/collision_offset_y",			0.0f,				emitter_node);		
		emitter.collision_dir_offset		= GET_SETTING(float,		"settings/collision_dir_offset",		0.0f,				emitter_node);

		emitter.draw_offset_x				= GET_SETTING(float,		"settings/draw_offset_x",				0.0f,				emitter_node);
		emitter.draw_offset_y				= GET_SETTING(float,		"settings/draw_offset_y",				0.0f,				emitter_node);

		emitter.owner_trigger_delay_min		= GET_SETTING(float,		"settings/owner_trigger_delay_min",		0.0f,				emitter_node);
		emitter.owner_trigger_delay_max		= GET_SETTING(float,		"settings/owner_trigger_delay_max",		0.0f,				emitter_node);
		emitter.destroy_on_spawn_collision	= GET_SETTING(bool,			"settings/destroy_on_spawn_collision",	true,				emitter_node);

		emitter.allow_bouncing				= GET_SETTING(bool,			"settings/allow_bouncing",				true,				emitter_node);
		emitter.is_projectile				= GET_SETTING(bool,			"settings/is_projectile",				false,				emitter_node);
		
		const char* loop_sound				= GET_SETTING(const char*,	"settings/loop_sound",					"",					emitter_node);
		emitter.loop_sound					= ResourceFactory::Get()->Get_Sound(loop_sound);
		const char* penetration_sound		= GET_SETTING(const char*,	"settings/penetration_sound",			"",					emitter_node);
		emitter.penetration_sound			= ResourceFactory::Get()->Get_Sound(penetration_sound);

		emitter.beam_use_seperate_tail_head	= GET_SETTING(bool,			"settings/beam_use_seperate_tail_head",				false,				emitter_node);	
		
		emitter.penetration_speed_scalar	= GET_SETTING(float,		"settings/penetration_speed_scalar",				1.0f,				emitter_node);	

		emitter.enable_damage				= GET_SETTING(bool,			"settings/enable_damage",				false,				emitter_node);	
		emitter.damage_offset_x				= GET_SETTING(float,		"settings/damage_offset_x",				0.0f,				emitter_node);		
		emitter.damage_offset_y				= GET_SETTING(float,		"settings/damage_offset_y",				0.0f,				emitter_node);	
		emitter.damage_radius				= GET_SETTING(float,		"settings/damage_radius",				0.0f,				emitter_node);	
		emitter.damage_type					= ParticleFX_DamageType::Parse(GET_SETTING(const char*, "settings/damage_type", "Fire", emitter_node));
		emitter.constant_damage				= GET_SETTING(bool,			"settings/constant_damage",				false,				emitter_node);	

		emitter.spawn_shape					= ParticleFX_SpawnShape::Parse(GET_SETTING(const char*, "settings/spawn_shape", "Point", emitter_node));
		emitter.spawn_inner_radius			= GET_SETTING(float,		"settings/spawn_inner_radius",			0.0f,				emitter_node);
		emitter.spawn_outer_radius			= GET_SETTING(float,		"settings/spawn_outer_radius",			0.0f,				emitter_node);
		
		emitter.collection_enabled			= GET_SETTING(bool,			"settings/collection_enabled",				false,				emitter_node);	
		emitter.collection_type				= ParticleFX_EmitterCollectionType::Parse(GET_SETTING(const char*, "settings/collection_type", "Coin", emitter_node));
		emitter.collection_value			= GET_SETTING(int,			"settings/collection_value",				0,				emitter_node);	

		emitter.track_effect				= GET_SETTING(bool,			"settings/track_effect",				false,				emitter_node);
		emitter.draw_shadow					= GET_SETTING(bool,			"settings/draw_shadow",					false,				emitter_node);
		emitter.offset_by_height			= GET_SETTING(bool,			"settings/offset_by_height",			false,				emitter_node);
		emitter.spawn_offset_x				= GET_SETTING(float,		"settings/spawn_offset_x",				0.0f,				emitter_node);
		emitter.spawn_offset_y				= GET_SETTING(float,		"settings/spawn_offset_y",				0.0f,				emitter_node);
		emitter.scale_proportional			= GET_SETTING(bool,			"settings/scale_proportional",			false,				emitter_node);		
		emitter.invert_scale_on_bounce		= GET_SETTING(bool,			"settings/invert_scale_on_bounce",		false,				emitter_node);	

		emitter.important					= GET_SETTING(bool,			"settings/important",					false,				emitter_node);	
		
		emitter.add_emitter_direction		= GET_SETTING(bool,			"settings/add_emitter_direction",		false,				emitter_node);		
		emitter.layer_offset				= GET_SETTING(int,			"settings/layer_offset",				0,					emitter_node);		
		emitter.depth_bias					= GET_SETTING(float,		"settings/depth_bias",					0.0f,				emitter_node);		

		emitter.event_count					= 0;

		emitter.current_particle_count		= 0;

		//DBG_LOG("\tEmitter - %s", emitter.name.c_str());

		// Parse curves.
		std::vector<ConfigFileNode> curve_nodes = config->Get_Children(config->Get<std::vector<ConfigFileNode> >("curves", emitter_node)[0]);
		for (std::vector<ConfigFileNode>::iterator iter2 = curve_nodes.begin(); iter2 != curve_nodes.end(); iter2++)
		{
			ConfigFileNode curve_node = *iter2;

			int curve_type = (int)ParticleFX_CurveType::Parse(config->Get_Name(curve_node));

			ParticleFX_Curve& curve = emitter.curves[curve_type];
			curve.points.clear();
			curve.start_min = GET_ATTRIBUTE(float, "start_min", 0.0f, curve_node);
			curve.start_max = GET_ATTRIBUTE(float, "start_max", 0.0f, curve_node);
			curve.end_min	= GET_ATTRIBUTE(float, "end_min", 0.0f, curve_node);
			curve.end_max	= GET_ATTRIBUTE(float, "end_max", 0.0f, curve_node);

			//DBG_LOG("\t\tCurve - %i", curve_type);

			// Parse points.
			std::vector<ConfigFileNode> point_nodes = config->Get<std::vector<ConfigFileNode> >("point", curve_node);
			for (std::vector<ConfigFileNode>::iterator iter3 = point_nodes.begin(); iter3 != point_nodes.end(); iter3++)
			{
				ConfigFileNode point_node = *iter3;

				ParticleFX_CurvePoint point;
				point.time	= GET_ATTRIBUTE(float, "x", 0.0f, point_node);
				point.value = GET_ATTRIBUTE(float, "y", 0.0f, point_node);
				curve.points.push_back(point);

				//DBG_LOG("\t\t\tPoint %f,%f", point.time, point.value);
			}

			curve.Precalculate();
		}

		// Parse events.
		std::vector<ConfigFileNode> event_nodes = config->Get<std::vector<ConfigFileNode> >("events/event", emitter_node);
		for (std::vector<ConfigFileNode>::iterator iter2 = event_nodes.begin(); iter2 != event_nodes.end(); iter2++)
		{
			ConfigFileNode event_node = *iter2;
			
			DBG_ASSERT(emitter.event_count < emitter.max_events);

			ParticleFX_Event point;
			point.time			= GET_ATTRIBUTE(float, "x", 0.0f, event_node);
			point.probability	= GET_ATTRIBUTE(float, "probability", 0.0f, event_node);
			point.type			= ParticleFX_EventType::Parse(GET_ATTRIBUTE(const char*, "type", "Spawn", event_node));
			point.trigger		= ParticleFX_EventTrigger::Parse(GET_ATTRIBUTE(const char*, "trigger", "Collision", event_node));			
			point.param			= GET_ATTRIBUTE(const char*, "param", "", event_node);	
			point.modifier		= GET_ATTRIBUTE(const char*, "modifier", "", event_node);
			point.modifier_hash	= point.modifier == "" ? 0 : StringHelper::Hash(point.modifier.c_str());
			point.param_float   = point.param == "" ? 0.0f : (float)atof(point.param.c_str());
			emitter.events[emitter.event_count++] = point;

			//DBG_LOG("\t\tEvent - %i", point.type);
		}

		m_emitters.push_back(emitter);
	}

	// Load each attractor.
	std::vector<ConfigFileNode> attractor_nodes = config->Get<std::vector<ConfigFileNode> >("attractors/attractor");
	for (std::vector<ConfigFileNode>::iterator iter = attractor_nodes.begin(); iter != attractor_nodes.end(); iter++)
	{
		if (m_attractors.size() >= max_attractors)
			continue;

		ConfigFileNode attractor_node = *iter;

		ParticleFX_AttractorConfig attractor;
		attractor.name			= GET_ATTRIBUTE(const char*, "name", "", attractor_node);
		attractor.strength		= GET_ATTRIBUTE(float, "strength", 0.0f, attractor_node);
		attractor.radius		= GET_ATTRIBUTE(float, "radius", 0.0f, attractor_node);
		attractor.x_offset		= GET_ATTRIBUTE(float, "x_offset", 0.0f, attractor_node);
		attractor.y_offset		= GET_ATTRIBUTE(float, "y_offset", 0.0f, attractor_node);
		attractor.track_effect	= GET_ATTRIBUTE(bool, "track_effect", false, attractor_node);

		//DBG_LOG("\nAttractor - %s", attractor.name.c_str());

		m_attractors.push_back(attractor);
	}

#undef GET_SETTING
#undef GET_ATTRIBUTE

	return true;
}

void ParticleFX::Save(const char* url)
{
	ConfigFile config;

	// Settings	
	ConfigFileNode settings_node = config.New_Node("settings");
	config.New_Node("name", m_name, settings_node);

	// Emitters
	ConfigFileNode emitters_node = config.New_Node("emitters");
	for (std::vector<ParticleFX_EmitterConfig>::iterator iter = m_emitters.begin(); iter != m_emitters.end(); iter++)
	{
		ParticleFX_EmitterConfig& emitter = *iter;

		ConfigFileNode emitter_node = config.New_Node("emitter", emitters_node);

			ConfigFileNode emitter_settings_node = config.New_Node("settings", emitter_node);
				config.New_Node("name",							emitter.name,															emitter_settings_node);
				config.New_Node("render_pass",					emitter.render_pass,													emitter_settings_node);
				config.New_Node("lock_rotation_to_direction",	emitter.lock_rotation_to_direction,										emitter_settings_node);
				config.New_Node("keep_effect_alive",			emitter.keep_effect_alive,												emitter_settings_node);
				config.New_Node("global_max_particles",			emitter.global_max_particles,											emitter_settings_node);
				config.New_Node("draw_method",					ParticleFX_EmitterDrawMethod::To_String(emitter.draw_method),			emitter_settings_node);
				config.New_Node("blend_mode",					ParticleFX_EmitterBlendMode::To_String(emitter.blend_mode),				emitter_settings_node);
				config.New_Node("cycle_length",					emitter.cycle_length,													emitter_settings_node);
				config.New_Node("cycle_count",					emitter.cycle_count,													emitter_settings_node);
				config.New_Node("max_particles_per_cycle",		emitter.max_particles_per_cycle,										emitter_settings_node);
				config.New_Node("allow_bouncing",				emitter.allow_bouncing,													emitter_settings_node);
				config.New_Node("is_projectile",				emitter.is_projectile,													emitter_settings_node);
				config.New_Node("beam_use_seperate_tail_head",	emitter.beam_use_seperate_tail_head,									emitter_settings_node);
				config.New_Node("draw_offset_x",				emitter.draw_offset_x,													emitter_settings_node);
				config.New_Node("draw_offset_y",				emitter.draw_offset_y,													emitter_settings_node);

				config.New_Node("owner_trigger_delay_min",		emitter.owner_trigger_delay_min,										emitter_settings_node);
				config.New_Node("owner_trigger_delay_max",		emitter.owner_trigger_delay_max,										 emitter_settings_node);
				config.New_Node("destroy_on_spawn_collision",	emitter.destroy_on_spawn_collision,										 emitter_settings_node);

				if (emitter.sprite_anim != NULL)
					config.New_Node("sprite",					emitter.sprite_anim->Name,												emitter_settings_node);
				else if (emitter.sprite_frame != NULL)
					config.New_Node("sprite",					emitter.sprite_frame->Name,												emitter_settings_node);
				else
					config.New_Node("sprite",					"",																		emitter_settings_node);

				if (emitter.loop_sound != NULL)
					config.New_Node("loop_sound",				emitter.loop_sound->Get()->Get_Name(),									emitter_settings_node);

				if (emitter.penetration_sound != NULL)
					config.New_Node("penetration_sound",		emitter.penetration_sound->Get()->Get_Name(),							emitter_settings_node);

				config.New_Node("affected_by_attractors",		emitter.affected_by_attractors,											emitter_settings_node);
				config.New_Node("move_based_on_direction",		emitter.move_based_on_direction,										emitter_settings_node);
				config.New_Node("enable_collision",				emitter.enable_collision,												emitter_settings_node);
				config.New_Node("collision_reponse",			ParticleFX_CollisionResponse::To_String(emitter.collision_response),	emitter_settings_node);
				config.New_Node("spawn_shape",					ParticleFX_SpawnShape::To_String(emitter.spawn_shape),					emitter_settings_node);
				config.New_Node("collision_scale_x",			emitter.collision_scale_x,												emitter_settings_node);
				config.New_Node("collision_scale_y",			emitter.collision_scale_y,												emitter_settings_node);
				config.New_Node("collision_offset_x",			emitter.collision_offset_x,												emitter_settings_node);
				config.New_Node("collision_offset_y",			emitter.collision_offset_y,												emitter_settings_node);
				config.New_Node("collision_dir_offset",			emitter.collision_dir_offset,											emitter_settings_node);
				config.New_Node("spawn_inner_radius",			emitter.spawn_inner_radius,												emitter_settings_node);
				config.New_Node("spawn_outer_radius",			emitter.spawn_outer_radius,												emitter_settings_node);
				config.New_Node("track_effect",					emitter.track_effect,													emitter_settings_node);
				config.New_Node("draw_shadow",					emitter.draw_shadow,													emitter_settings_node);
				config.New_Node("offset_by_height",				emitter.offset_by_height,												emitter_settings_node);
				config.New_Node("important",					emitter.important,														emitter_settings_node);
				config.New_Node("spawn_offset_x",				emitter.spawn_offset_x,													emitter_settings_node);
				config.New_Node("spawn_offset_y",				emitter.spawn_offset_y,													emitter_settings_node);
				config.New_Node("scale_proportional",			emitter.scale_proportional,												emitter_settings_node);
				config.New_Node("invert_scale_on_bounce",		emitter.invert_scale_on_bounce,											emitter_settings_node);
				config.New_Node("add_emitter_direction",		emitter.add_emitter_direction,											emitter_settings_node);
				config.New_Node("layer_offset",					emitter.layer_offset,													emitter_settings_node);
				config.New_Node("depth_bias",					emitter.depth_bias,														emitter_settings_node);
				config.New_Node("penetration_speed_scalar",		emitter.penetration_speed_scalar,										emitter_settings_node);
				config.New_Node("enable_damage",				emitter.enable_damage,													emitter_settings_node);
				config.New_Node("constant_damage",				emitter.constant_damage,												emitter_settings_node);
				config.New_Node("damage_offset_x",				emitter.damage_offset_x,												emitter_settings_node);
				config.New_Node("damage_offset_y",				emitter.damage_offset_y,												emitter_settings_node);			
				config.New_Node("damage_radius",				emitter.damage_radius,													emitter_settings_node);
				config.New_Node("damage_type",					ParticleFX_DamageType::To_String(emitter.damage_type),					emitter_settings_node);
				config.New_Node("collection_enabled",			emitter.collection_enabled,												emitter_settings_node);
				config.New_Node("collection_value",				emitter.collection_value,												emitter_settings_node);
				config.New_Node("collection_type",				ParticleFX_EmitterCollectionType::To_String(emitter.collection_type),	emitter_settings_node);

			ConfigFileNode emitter_curves_node = config.New_Node("curves", emitter_node);
			for (int i = 0; i < ParticleFX_CurveType::COUNT; i++)
			{
				ParticleFX_Curve& curve = emitter.curves[i];

				ConfigFileNode curve_node = config.New_Node(ParticleFX_CurveType::To_String((ParticleFX_CurveType::Type)i), emitter_curves_node);
				config.Set("start_min",		curve.start_min,	curve_node, true);
				config.Set("start_max",		curve.start_max,	curve_node, true);
				config.Set("end_min",		curve.end_min,		curve_node, true);
				config.Set("end_max",		curve.end_max,		curve_node, true);

				// Curve points plz.
				for (std::vector<ParticleFX_CurvePoint>::iterator iter2 = curve.points.begin(); iter2 != curve.points.end(); iter2++)
				{
					ParticleFX_CurvePoint& point = *iter2;
					
					ConfigFileNode point_node = config.New_Node("point", curve_node);
					config.Set("x",	point.time,	point_node, true);
					config.Set("y",	point.value,point_node, true);
				}
			}

			ConfigFileNode emitter_events_node = config.New_Node("events", emitter_node);
			for (int i = 0; i < emitter.event_count; i++)
			{
				ParticleFX_Event& evt = emitter.events[i];

				ConfigFileNode event_node = config.New_Node("event", emitter_events_node);
				config.Set("x",				evt.time,											event_node, true);
				config.Set("probability",	evt.probability,									event_node, true);
				config.Set("type",			ParticleFX_EventType::To_String(evt.type),			event_node, true);
				config.Set("trigger",		ParticleFX_EventTrigger::To_String(evt.trigger),	event_node, true);
				config.Set("param",			evt.param,											event_node, true);
				config.Set("modifier",		evt.modifier,										event_node, true);
			}
	}

	// Attractors.
	ConfigFileNode attractors_node = config.New_Node("attractors");
	for (std::vector<ParticleFX_AttractorConfig>::iterator iter = m_attractors.begin(); iter != m_attractors.end(); iter++)
	{
		ParticleFX_AttractorConfig& attractor = *iter;

		ConfigFileNode attractor_node = config.New_Node("attractor", attractors_node);
		config.New_Node("name",			attractor.name,			attractor_node);
		config.New_Node("strength",		attractor.strength,		attractor_node);
		config.New_Node("radius",		attractor.radius,		attractor_node);
		config.New_Node("x_offset",		attractor.x_offset,		attractor_node);
		config.New_Node("y_offset",		attractor.y_offset,		attractor_node);
		config.New_Node("track_effect",	attractor.track_effect, attractor_node);
	}

	config.Save(url);
}

ParticleFX* ParticleFX::Load(const char* url)
{
	Platform* platform = Platform::Get();

	// Compile time.
	DBG_LOG("Loading particlefx resource '%s'.", url);
	
	// Load configuration settings.
	ConfigFile config;
	if (!config.Load(url))
	{
		DBG_LOG("Failed to load particlefx, config file could not be found: '%s'", url);
		return NULL;
	}

	// Create the pfx.
	ParticleFX* pfx = new ParticleFX();
	
	// Load layout.
	if (!pfx->Load_Elements(&config))
	{
		DBG_LOG("Failed to load particlefx, element configuration is invalid: '%s'", url);
		SAFE_DELETE(pfx);
		return NULL;
	}

	return pfx;
}

std::string ParticleFX::Get_Name()
{
	return m_name;
}

int ParticleFX::Get_Name_Hash()
{
	return m_name_hash;
}