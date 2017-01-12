// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/PfxEditor/UIScene_PfxEditor.h"
#include "Game/UI/Scenes/Editor/UIScene_FileBrowser.h"
#include "Game/UI/Scenes/Editor/UIScene_ResourceBrowser.h"
#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/Editor/EditorTileSelection.h"
#include "Game/UI/Scenes/UIScene_Chat.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"

#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Display/GfxDisplay.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIMenuBar.h"
#include "Engine/UI/Elements/UISplitContainerItem.h"

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIToolbarItem.h"
#include "Engine/UI/Elements/UIMenuItem.h"
#include "Engine/UI/Elements/UIPropertyGrid.h"
#include "Engine/UI/Elements/UICurveEditor.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Engine/GameEngine.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Game/Runner/Game.h"
#include "Game/Game/EditorManager.h"

#include "Generic/Math/Math.h"
#include "Generic/Helper/StringHelper.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Camera.h"
#include "Game/Scene/Cameras/EditorCamera.h"
#include "Game/Scene/Cameras/GameCamera.h"
#include "Game/Scene/Editor/EditorBackground.h"
#include "Game/Scene/Editor/EditorForeground.h"

#include "Game/Game/VoteManager.h"
#include "Game/Game/Votes/EditModeVote.h"

#include "Engine/Particles/ParticleFX.h"
#include "Engine/Particles/ParticleFXHandle.h"
#include "Engine/Particles/ParticleManager.h"

#include "Game/Version.h"

#define DELETE_EMITTER_EVENT_ID			1
#define DELETE_ATTRACTOR_EVENT_ID		2 
#define CURVE_CLICK_EVENT_ID			3
#define ADD_EVENT_EVENT_ID				4
#define DELETE_EVENT_EVENT_ID			5
#define SPRITE_CLICK_EVENT_ID			6
#define ANIMATION_CLICK_EVENT_ID		7
#define SOUND_CLICK_EVENT_ID			8

UIScene_PfxEditor::UIScene_PfxEditor()
	: m_needs_refresh(false)
	, m_fx(NULL)
	, m_save_path("")
	, m_effect_pos(0.0f, 0.0f, 3000.0f)
	, m_effect_dir(0)
	, m_collision_show(false)
	, m_edit_curve(NULL)
	, m_edit_resource_name(NULL)
	, m_show_lighting(false)
{
	Set_Layout("particle_editor");

	// Grab frames we need.
	m_background_frame = ResourceFactory::Get()->Get_Atlas_Frame("editor_background");
	
	m_manager		= GameEngine::Get()->Get_UIManager();
	m_font			= m_manager->Get_Font();
	m_font_renderer	= FontRenderer(m_font, false, false);

	// Copy game camera state to editor and begin using it.
	RenderPipeline*		pipeline			= RenderPipeline::Get();
	Game*				game				= Game::Get();
	Scene*				scene				= GameEngine::Get()->Get_Scene();
	Map*				map					= game->Get_Map();
	Atlas*				tileset				= map->Get_Tileset()->Get();
	Vector2				tile_size			= map->Get_Tile_Size();
	
	AtlasFrame*			tileset_frame		= tileset->Get_Frame_By_Index(0);	
	Vector2				tileset_expanse		= Vector2(tileset_frame->GridOrigin.Width, tileset_frame->GridOrigin.Height);
	
	UIPanel*			camera_panel		= Find_Element<UIPanel*>("camera_panel");
	Rect2D				camera_panel_box	= camera_panel->Get_Screen_Box();

	Camera*				game_camera			= Game::Get()->Get_Camera(CameraID::Game1); // SPLITSCREEN_TODO
	EditorCamera*		editor_camera		= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Main));
	EditorCamera*		tileset_camera		= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Tileset));
	EditorCamera*		sub_camera			= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Sub));

	Reset_Camera();

	editor_camera->Set_Enabled(false);
	tileset_camera->Set_Enabled(false);
	sub_camera->Set_Enabled(true);
	
	pipeline->Set_Active_Camera(sub_camera);
	pipeline->Set_Disable_Drawables(true);

	// New effect!
	New();

	// Refresr property.
	Refresh_Properties();
	Refresh_State();

	// Add some collision around main area.

	// If we are viewing debug stuff, add a bit of collision around the origin.
	CollisionManager::Get()->Create_Handle(CollisionType::Solid, CollisionShape::Rectangle, CollisionGroup::Environment, CollisionGroup::NONE, Rect2D(-128, (-128) - 16, 256, 16), Vector3(0,0,0), Vector3(0,0,0), true);
	CollisionManager::Get()->Create_Handle(CollisionType::Solid, CollisionShape::Rectangle, CollisionGroup::Environment, CollisionGroup::NONE, Rect2D(-128, 128, 256, 16), Vector3(0,0,0), Vector3(0,0,0), true);
	CollisionManager::Get()->Create_Handle(CollisionType::Solid, CollisionShape::Rectangle, CollisionGroup::Environment, CollisionGroup::NONE, Rect2D((-128) - 16, (-128) - 16, 16, 256 + 32), Vector3(0,0,0), Vector3(0,0,0), true);
	CollisionManager::Get()->Create_Handle(CollisionType::Solid, CollisionShape::Rectangle, CollisionGroup::Environment, CollisionGroup::NONE, Rect2D(128, (-128) - 16, 16, 256 + 32), Vector3(0,0,0), Vector3(0,0,0), true);
}

UIScene_PfxEditor::~UIScene_PfxEditor()
{
	RenderPipeline*	pipeline = RenderPipeline::Get();
	pipeline->Set_Disable_Drawables(false);

	ParticleManager::Get()->Reset();	
	ParticleManager::Get()->Set_Draw_Debug(false);
	CollisionManager::Get()->Set_Show_Collision(false);

	CollisionManager::Get()->Reset();
	Game::Get()->Get_Map()->Create_Collision();

	SAFE_DELETE(m_fx);
}

void UIScene_PfxEditor::Reset_Camera()
{
	UIPanel*		camera_panel		= Find_Element<UIPanel*>("camera_panel");
	Rect2D			camera_panel_box	= camera_panel->Get_Screen_Box();
	EditorCamera*	sub_camera			= static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Sub));

	sub_camera->Set_Zoom(1.0f);
	sub_camera->Set_Position(Vector3(-(camera_panel_box.Width * 0.5f), -(camera_panel_box.Height * 0.5f), 0));

	m_effect_pos = Vector3(0.0f, 0.0f, 2.0f);
	m_effect_dir = 0;
}

void UIScene_PfxEditor::New(const char* path)
{
	ParticleManager::Get()->Reset();

	Update_Curves();
	m_edit_curve = NULL;

	// Delete old effect.
	SAFE_DELETE(m_fx);

	// Create a blank effect
	if (path != NULL)
	{
		m_fx = ParticleFX::Load(path);
	}
	if (m_fx == NULL)
	{
		m_fx = new ParticleFX();
		m_fx->m_name = "Untitled Effect";
	}

	// Spawn initial effect.
	m_effect = ParticleManager::Get()->Spawn(m_fx, m_effect_pos, m_effect_dir, rand() % 100, 0);

	m_save_path = "";

	Reset_Camera();	
	Refresh_Properties();
}

void UIScene_PfxEditor::Begin_Open()
{
	m_saving = false;
	m_manager->Go(UIAction::Push(new UIScene_FileBrowser(false, "../Data/Base/Effects/", "xml"), new UIFadeInTransition()));	
}

void UIScene_PfxEditor::Begin_Save()
{
	if (m_save_path != "")
	{
		Save(m_save_path);
	}
	else
	{
		Begin_SaveAs();
	}
}

void UIScene_PfxEditor::Begin_SaveAs()
{
	m_save_path = "";
	m_saving = true;
	m_manager->Go(UIAction::Push(new UIScene_FileBrowser(true, "../Data/Base/Effects/", "xml"), new UIFadeInTransition()));	
}

void UIScene_PfxEditor::Save(std::string path)
{
	DBG_LOG("Saving '%s'.", path.c_str());

	m_fx->Save(path.c_str());
	m_save_path = path;
}

void UIScene_PfxEditor::Open(std::string path)
{
	DBG_LOG("Opening '%s'.", path.c_str());
	New(path.c_str());
	m_save_path = path;
}

void UIScene_PfxEditor::Add_Effect()
{
	if (m_fx->m_emitters.size() >= m_fx->max_emitters)
		return;

	Update_Curves();
	m_edit_curve = NULL;
	
	ParticleManager::Get()->Reset();

	ParticleFX_EmitterConfig emitter;
	emitter.name						= "Untitled Effect";
	emitter.global_max_particles		= 0;
	emitter.draw_method					= ParticleFX_EmitterDrawMethod::Sprite;
	emitter.blend_mode					= ParticleFX_EmitterBlendMode::Alpha;
	emitter.lock_rotation_to_direction	= true;
	emitter.beam_use_seperate_tail_head	= false;
	emitter.render_pass					= "geometry";
	emitter.cycle_length				= 1.0f;
	emitter.cycle_count					= 1;
	emitter.max_particles_per_cycle		= 0;
	emitter.editor_sprite_name			= "";
	emitter.sprite_anim					= NULL;
	emitter.sprite_frame				= ResourceFactory::Get()->Get_Atlas_Frame("particle_smoke");
	emitter.affected_by_attractors		= false;
	emitter.move_based_on_direction		= true;
	emitter.enable_collision			= false;
	emitter.collision_response			= ParticleFX_CollisionResponse::Stop;
	emitter.spawn_shape					= ParticleFX_SpawnShape::Point;
	emitter.collision_scale_x			= 1.0f;
	emitter.collision_scale_y			= 1.0f;
	emitter.collision_offset_y			= 0.0f;
	emitter.collision_offset_y			= 0.0f;
	emitter.spawn_inner_radius			= 10.0f;
	emitter.spawn_outer_radius			= 10.0f;
	emitter.track_effect				= false;
	emitter.current_particle_count		= 0;
	emitter.event_count					= 0;
	emitter.draw_shadow					= false;
	emitter.offset_by_height			= false;
	emitter.important					= false;
	emitter.spawn_offset_x				= 0;
	emitter.spawn_offset_y				= 0;
	emitter.scale_proportional			= false;
	emitter.add_emitter_direction		= true;
	emitter.depth_bias					= 0.0f;
	emitter.layer_offset				= 0;
	emitter.keep_effect_alive			= true;
	emitter.invert_scale_on_bounce		= false;
	emitter.enable_damage				= false;
	emitter.damage_offset_x				= 0;
	emitter.damage_offset_y				= 0;
	emitter.damage_radius				= 0.0f;
	emitter.damage_type					= ParticleFX_DamageType::Projectile;
	emitter.constant_damage				= false;
	emitter.collection_enabled			= false;
	emitter.collection_value			= 0;
	emitter.collection_type				= ParticleFX_EmitterCollectionType::Coin;
	emitter.penetration_sound			= NULL;
	emitter.loop_sound					= NULL;
	emitter.penetration_speed_scalar	= 1.0f;
	emitter.allow_bouncing				= false;

	emitter.curves[ParticleFX_CurveType::Sprite_Frame]				= ParticleFX_Curve(0, 0, 0, 0);
	emitter.curves[ParticleFX_CurveType::Direction]					= ParticleFX_Curve(0, 360, 0, 360);
	emitter.curves[ParticleFX_CurveType::Spawn_Rate]				= ParticleFX_Curve(100, 100, 1000, 1000);
	emitter.curves[ParticleFX_CurveType::Velocity_X]				= ParticleFX_Curve(10, 10, 20, 20);
	emitter.curves[ParticleFX_CurveType::Velocity_Y]				= ParticleFX_Curve(10, 10, 20, 20);
	emitter.curves[ParticleFX_CurveType::Velocity_Height]			= ParticleFX_Curve(0, 0, 0, 0);
	emitter.curves[ParticleFX_CurveType::Velocity_Rotation]			= ParticleFX_Curve(0, 0, 0, 0);
	emitter.curves[ParticleFX_CurveType::Velocity_Direction]		= ParticleFX_Curve(0, 0, 0, 0);
	emitter.curves[ParticleFX_CurveType::Color_R]					= ParticleFX_Curve(0, 0, 1, 1);
	emitter.curves[ParticleFX_CurveType::Color_G]					= ParticleFX_Curve(0, 0, 1, 1);
	emitter.curves[ParticleFX_CurveType::Color_B]					= ParticleFX_Curve(0, 0, 1, 1);
	emitter.curves[ParticleFX_CurveType::Color_A]					= ParticleFX_Curve(0, 0, 1, 1);
	emitter.curves[ParticleFX_CurveType::Scale_X]					= ParticleFX_Curve(0, 0, 1, 1);
	emitter.curves[ParticleFX_CurveType::Scale_Y]					= ParticleFX_Curve(0, 0, 1, 1);
	emitter.curves[ParticleFX_CurveType::Lifetime]					= ParticleFX_Curve(0, 0, 10, 10);
	emitter.curves[ParticleFX_CurveType::Rotation]					= ParticleFX_Curve(0, 0, 0, 0);
	emitter.curves[ParticleFX_CurveType::Height]					= ParticleFX_Curve(0, 0, 0, 0);
	emitter.curves[ParticleFX_CurveType::Damage]					= ParticleFX_Curve(0, 0, 0, 0);

	m_fx->m_emitters.push_back(emitter);

	m_needs_refresh = true;

	Refresh_Curve();
}

void UIScene_PfxEditor::Add_Attractor()
{
	if (m_fx->m_attractors.size() >= m_fx->max_attractors)
		return;

	Update_Curves();
	m_edit_curve = NULL;
	
	ParticleManager::Get()->Reset();

	ParticleFX_AttractorConfig emitter;
	emitter.name			= "Untitled Attractor"; 
	emitter.x_offset		= 0.0f;
	emitter.y_offset		= 0.0f;
	emitter.strength		= 10.0f;
	emitter.radius			= 100.0f;
	emitter.track_effect	= true;

	m_fx->m_attractors.push_back(emitter);
	
	m_needs_refresh = true;

	Refresh_Curve();
}

void UIScene_PfxEditor::Delete_Effect(ParticleFX_EmitterConfig* emitter)
{
	Update_Curves();
	m_edit_curve = NULL;
	
	ParticleManager::Get()->Reset();

	for (std::vector<ParticleFX_EmitterConfig>::iterator iter = m_fx->m_emitters.begin(); iter != m_fx->m_emitters.end(); iter++)
	{
		if (&(*iter) == emitter)
		{
			m_fx->m_emitters.erase(iter);
			break;
		}
	}
	
	m_needs_refresh = true;

	Refresh_Curve();
}

void UIScene_PfxEditor::Delete_Attractor(ParticleFX_AttractorConfig* attractor)
{
	Update_Curves();
	m_edit_curve = NULL;

	ParticleManager::Get()->Reset();

	for (std::vector<ParticleFX_AttractorConfig>::iterator iter = m_fx->m_attractors.begin(); iter != m_fx->m_attractors.end(); iter++)
	{
		if (&(*iter) == attractor)
		{
			m_fx->m_attractors.erase(iter);
			break;
		}
	}
	
	m_needs_refresh = true;

	Refresh_Curve();
}

void UIScene_PfxEditor::Add_Event_Node(int index, UIPropertyGrid* grid, UIPropertyGridItem* parent, ParticleFX_Event* evt)
{
	std::vector<std::string> event_types;
	for (int i = 0; i < ParticleFX_EventType::COUNT; i++)
	{
		event_types.push_back(ParticleFX_EventType::To_Name((ParticleFX_EventType::Type)i));
	}

	std::vector<std::string> trigger_types;
	for (int i = 0; i < ParticleFX_EventTrigger::COUNT; i++)
	{
		trigger_types.push_back(ParticleFX_EventTrigger::To_Name((ParticleFX_EventTrigger::Type)i));
	}

	UIPropertyGridItem* evt_node = grid->Add_Item(StringHelper::Format("Event %i", index).c_str(), parent, UIPropertyGridDataType::None, NULL, std::vector<std::string>(), evt);				
	grid->Add_Item_Button(evt_node, "icon_subtract", DELETE_EVENT_EVENT_ID);

	grid->Add_Item("Type", evt_node, UIPropertyGridDataType::Combo, &evt->type, event_types);
	grid->Add_Item("Trigger", evt_node, UIPropertyGridDataType::Combo, &evt->trigger, trigger_types);
	grid->Add_Item("Time", evt_node, UIPropertyGridDataType::Float, &evt->time);
	grid->Add_Item("Probability", evt_node, UIPropertyGridDataType::Float, &evt->probability);
	grid->Add_Item("Parameter", evt_node, UIPropertyGridDataType::String, &evt->param);
	grid->Add_Item("Sound", evt_node, UIPropertyGridDataType::External, &evt->param, std::vector<std::string>(), &evt->param, SOUND_CLICK_EVENT_ID);
}

void UIScene_PfxEditor::Add_Event(UIPropertyGrid* grid, UIPropertyGridItem* item, ParticleFX_EmitterConfig* emt)
{
	if (emt->event_count >= ParticleFX_EmitterConfig::max_events)
		return;

	ParticleFX_Event evt;
	evt.trigger = ParticleFX_EventTrigger::Time;
	evt.type	= ParticleFX_EventType::Play_Sound;
	evt.time	= 0.0f;
	evt.param	= "";
	evt.probability	= 1.0f;
	emt->events[emt->event_count++] = evt;

	Add_Event_Node(emt->event_count, grid, item, &emt->events[emt->event_count - 1]);
}

void UIScene_PfxEditor::Remove_Event(UIPropertyGrid* grid, UIPropertyGridItem* item, ParticleFX_Event* evt)
{
	ParticleFX_EmitterConfig* emt = NULL;

	for (std::vector<ParticleFX_EmitterConfig>::iterator iter = m_fx->m_emitters.begin(); iter != m_fx->m_emitters.end(); iter++)
	{
		ParticleFX_EmitterConfig& emitter = *iter;
		for (int i = 0; i < emitter.event_count; i++)
		{
			if (&emitter.events[i] == evt)
			{
				// Copy last event over new event.
				emitter.events[i] = emitter.events[--emitter.event_count];
				emt = &emitter;
				break;
			}
		}
	}
	
	UIPropertyGridItem* parent = item->Parent;
	int index = 0;

	DBG_ASSERT(emt != NULL);			
	grid->Remove_Item(item);				

	// Update pointer references for all nodes still existing.
	for (std::vector<UIPropertyGridItem*>::iterator iter = parent->Children.begin(); iter != parent->Children.end(); iter++)
	{
		UIPropertyGridItem* node = *iter;
		node->Name = StringHelper::Format("Event %i", index + 1);
		node->Meta_Data = &emt->events[index++];
	}
}

void UIScene_PfxEditor::Update_Curves()
{
	UICurveEditor* grid = Find_Element<UICurveEditor*>("curve_panel");

	// We don't need to do this when updating curves?
	//ParticleManager::Get()->Reset();

	if (m_edit_curve != NULL)
	{
		std::vector<UICurvePoint> points = grid->Get_Points();
		
		m_edit_curve->points.clear();

		for (std::vector<UICurvePoint>::iterator iter = points.begin(); iter != points.end(); iter++)
		{
			UICurvePoint ui_p = *iter;

			ParticleFX_CurvePoint p;
			p.time = ui_p.x;
			p.value = ui_p.y;

			m_edit_curve->points.push_back(p);
		}

		m_edit_curve->Precalculate();
	}
}

void UIScene_PfxEditor::Precalculate_Curve()
{
	for (std::vector<ParticleFX_EmitterConfig>::iterator iter = m_fx->m_emitters.begin(); iter != m_fx->m_emitters.end(); iter++)
	{
		ParticleFX_EmitterConfig& config = *iter;
		for (int i = 0; i < ParticleFX_CurveType::COUNT; i++)
			config.curves[i].Precalculate();
	}
}

void UIScene_PfxEditor::Refresh_Curve()
{
	UICurveEditor* grid = Find_Element<UICurveEditor*>("curve_panel");
	grid->Clear_Points();

	if (m_edit_curve != NULL)
	{
		for (std::vector<ParticleFX_CurvePoint>::iterator iter = m_edit_curve->points.begin(); iter != m_edit_curve->points.end(); iter++)
		{
			ParticleFX_CurvePoint point = *iter;
			grid->Add_Point(point.time, point.value, point.time == 1.0f || point.time == 0.0f);
		}
	}
}

void UIScene_PfxEditor::Update_Properties()
{
	for (std::vector<ParticleFX_EmitterConfig>::iterator iter = m_fx->m_emitters.begin(); iter != m_fx->m_emitters.end(); iter++)
	{
		ParticleFX_EmitterConfig& config = *iter;
		config.sprite_anim = ResourceFactory::Get()->Get_Atlas_Animation(config.editor_sprite_name.c_str());
		config.sprite_frame = ResourceFactory::Get()->Get_Atlas_Frame(config.editor_sprite_name.c_str());
		config.loop_sound = ResourceFactory::Get()->Get_Sound(config.editor_loop_sound.c_str());
		config.penetration_sound = ResourceFactory::Get()->Get_Sound(config.editor_penetration_sound.c_str());
	}
}

void UIScene_PfxEditor::Refresh_Properties()
{
	UIPropertyGrid* grid = Find_Element<UIPropertyGrid*>("properties_panel");
	grid->Clear_Items();

	// Settings
	UIPropertyGridItem* settings = grid->Add_Item("Settings");
		 grid->Add_Item("Name", settings, UIPropertyGridDataType::String, &m_fx->m_name);
		
	// Emitters
	for (int i = 0; i < m_fx->Get_Emitter_Count(); i++)
	{
		ParticleFX_EmitterConfig* emitter = m_fx->Get_Emitter(i);

		UIPropertyGridItem* emitter_node = grid->Add_Item(StringHelper::Format("Emitter %i", i + 1).c_str(), NULL, UIPropertyGridDataType::None, NULL, std::vector<std::string>(), emitter);
		grid->Add_Item_Button(emitter_node, "icon_subtract", DELETE_EMITTER_EVENT_ID);

			grid->Add_Item("Name", emitter_node, UIPropertyGridDataType::String, &emitter->name);
			grid->Add_Item("Global Max Particles", emitter_node, UIPropertyGridDataType::Int, &emitter->global_max_particles);
			grid->Add_Item("Keep Effect Alive", emitter_node, UIPropertyGridDataType::Bool, &emitter->keep_effect_alive);

			std::vector<std::string> draw_modes;
			for (int j = 0; j < ParticleFX_EmitterDrawMethod::COUNT; j++)
			{
				draw_modes.push_back(ParticleFX_EmitterDrawMethod::To_String((ParticleFX_EmitterDrawMethod::Type)j));
			}
			grid->Add_Item("Draw Mode", emitter_node, UIPropertyGridDataType::Combo, &emitter->draw_method, draw_modes);

			std::vector<std::string> blend_modes;
			for (int j = 0; j < ParticleFX_EmitterBlendMode::COUNT; j++)
			{
				blend_modes.push_back(ParticleFX_EmitterBlendMode::To_Name((ParticleFX_EmitterBlendMode::Type)j));
			}
			grid->Add_Item("Blend Mode", emitter_node, UIPropertyGridDataType::Combo, &emitter->blend_mode, blend_modes);
			
			grid->Add_Item("Beam Uses Separate Tail And Head", emitter_node, UIPropertyGridDataType::Bool, &emitter->beam_use_seperate_tail_head);
			
			grid->Add_Item("Lock Rotation To Direction", emitter_node, UIPropertyGridDataType::Bool, &emitter->lock_rotation_to_direction);
			grid->Add_Item("Render Pass", emitter_node, UIPropertyGridDataType::String, &emitter->render_pass);
			grid->Add_Item("Cycle Length", emitter_node, UIPropertyGridDataType::Float, &emitter->cycle_length);
			grid->Add_Item("Cycle Count", emitter_node, UIPropertyGridDataType::Float, &emitter->cycle_count);
			grid->Add_Item("Cycle Max Particles", emitter_node, UIPropertyGridDataType::Int, &emitter->max_particles_per_cycle);			
			
			emitter->editor_sprite_name = (emitter->sprite_anim != NULL ? emitter->sprite_anim->Name.c_str() : (emitter->sprite_frame != NULL ? emitter->sprite_frame->Name.c_str() : ""));
			grid->Add_Item("Sprite", emitter_node, UIPropertyGridDataType::External, &emitter->editor_sprite_name, std::vector<std::string>(), &emitter->editor_sprite_name, SPRITE_CLICK_EVENT_ID);
			grid->Add_Item("Animation", emitter_node, UIPropertyGridDataType::External, &emitter->editor_sprite_name, std::vector<std::string>(), &emitter->editor_sprite_name, ANIMATION_CLICK_EVENT_ID);

			grid->Add_Item("Affected By Attractors", emitter_node, UIPropertyGridDataType::Bool, &emitter->affected_by_attractors);
			grid->Add_Item("Move Based On Direction", emitter_node, UIPropertyGridDataType::Bool, &emitter->move_based_on_direction);
			grid->Add_Item("Enable Collision", emitter_node, UIPropertyGridDataType::Bool, &emitter->enable_collision);
		
			grid->Add_Item("Allow Bouncing", emitter_node, UIPropertyGridDataType::Bool, &emitter->allow_bouncing);

			grid->Add_Item("Loop Sound", emitter_node, UIPropertyGridDataType::External, &emitter->editor_loop_sound, std::vector<std::string>(), &emitter->editor_loop_sound, SOUND_CLICK_EVENT_ID);
			grid->Add_Item("Penetration Sound", emitter_node, UIPropertyGridDataType::External, &emitter->editor_penetration_sound, std::vector<std::string>(), &emitter->editor_penetration_sound, SOUND_CLICK_EVENT_ID);
			
			grid->Add_Item("Penetration Velocity Scalar", emitter_node, UIPropertyGridDataType::Float, &emitter->penetration_speed_scalar);

			// collision response.
			std::vector<std::string> collision_responses;
			for (int j = 0; j < ParticleFX_CollisionResponse::COUNT; j++)
			{
				collision_responses.push_back(ParticleFX_CollisionResponse::To_Name((ParticleFX_CollisionResponse::Type)j));
			}
			grid->Add_Item("Collision Response", emitter_node, UIPropertyGridDataType::Combo, &emitter->collision_response, collision_responses);
			grid->Add_Item("Collision Scale X", emitter_node, UIPropertyGridDataType::Float, &emitter->collision_scale_x);
			grid->Add_Item("Collision Scale Y", emitter_node, UIPropertyGridDataType::Float, &emitter->collision_scale_y);
			grid->Add_Item("Collision Offset X", emitter_node, UIPropertyGridDataType::Float, &emitter->collision_offset_x);
			grid->Add_Item("Collision Offset Y", emitter_node, UIPropertyGridDataType::Float, &emitter->collision_offset_y);

			grid->Add_Item("Enable Damage", emitter_node, UIPropertyGridDataType::Bool, &emitter->enable_damage);
			std::vector<std::string> damage_types;
			for (int j = 0; j < ParticleFX_DamageType::COUNT; j++)
			{
				damage_types.push_back(ParticleFX_DamageType::To_String((ParticleFX_DamageType::Type)j));
			}
			grid->Add_Item("Damage Response", emitter_node, UIPropertyGridDataType::Combo, &emitter->damage_type, damage_types);
			grid->Add_Item("Damage Radius", emitter_node, UIPropertyGridDataType::Float, &emitter->damage_radius);
			grid->Add_Item("Damage Offset X", emitter_node, UIPropertyGridDataType::Float, &emitter->damage_offset_x);
			grid->Add_Item("Damage Offset Y", emitter_node, UIPropertyGridDataType::Float, &emitter->damage_offset_y);
			grid->Add_Item("Constant Damage", emitter_node, UIPropertyGridDataType::Bool, &emitter->constant_damage);
		
			std::vector<std::string> collection_types;
			for (int j = 0; j < ParticleFX_EmitterCollectionType::COUNT; j++)
			{
				collection_types.push_back(ParticleFX_EmitterCollectionType::To_String((ParticleFX_EmitterCollectionType::Type)j));
			}
			grid->Add_Item("Collection Enabled", emitter_node, UIPropertyGridDataType::Bool, &emitter->collection_enabled);
			grid->Add_Item("Collection Type", emitter_node, UIPropertyGridDataType::Combo, &emitter->collection_type, collection_types);
			grid->Add_Item("Collection Value", emitter_node, UIPropertyGridDataType::Float, &emitter->collection_value);

			// spawn shape.
			std::vector<std::string> spawn_shapes;
			for (int j = 0; j < ParticleFX_SpawnShape::COUNT; j++)
			{
				spawn_shapes.push_back(ParticleFX_SpawnShape::To_Name((ParticleFX_SpawnShape::Type)j));
			}
			grid->Add_Item("Spawn Shape", emitter_node, UIPropertyGridDataType::Combo, &emitter->spawn_shape, spawn_shapes);
			
			grid->Add_Item("Spawn Inner Radius", emitter_node, UIPropertyGridDataType::Float, &emitter->spawn_inner_radius);
			grid->Add_Item("Spawn Outer Radius", emitter_node, UIPropertyGridDataType::Float, &emitter->spawn_outer_radius);
			grid->Add_Item("Spawn Offset X", emitter_node, UIPropertyGridDataType::Float, &emitter->spawn_offset_x);
			grid->Add_Item("Spawn Offset Y", emitter_node, UIPropertyGridDataType::Float, &emitter->spawn_offset_y);

			grid->Add_Item("Track Effect", emitter_node, UIPropertyGridDataType::Bool, &emitter->track_effect);
			grid->Add_Item("Draw Shadow", emitter_node, UIPropertyGridDataType::Bool, &emitter->draw_shadow);
			grid->Add_Item("Offset By Height", emitter_node, UIPropertyGridDataType::Bool, &emitter->offset_by_height);
			grid->Add_Item("Important", emitter_node, UIPropertyGridDataType::Bool, &emitter->important);
			
			grid->Add_Item("Scale Proportional", emitter_node, UIPropertyGridDataType::Bool, &emitter->scale_proportional);
			grid->Add_Item("Invert Scale On Bounce", emitter_node, UIPropertyGridDataType::Bool, &emitter->invert_scale_on_bounce);
			grid->Add_Item("Add Emitter Rotation", emitter_node, UIPropertyGridDataType::Bool, &emitter->add_emitter_direction);	

			grid->Add_Item("Depth Bias", emitter_node, UIPropertyGridDataType::Float, &emitter->depth_bias);
			grid->Add_Item("Layer Offset", emitter_node, UIPropertyGridDataType::Int, &emitter->layer_offset);			

			UIPropertyGridItem* curves = grid->Add_Item("Curves", emitter_node);		
			for (int j = 0; j < ParticleFX_CurveType::COUNT; j++)
			{
				UIPropertyGridItem* curve = grid->Add_Item(ParticleFX_CurveType::To_Name((ParticleFX_CurveType::Type)j), curves, UIPropertyGridDataType::External, NULL, std::vector<std::string>(), &emitter->curves[i], CURVE_CLICK_EVENT_ID);
				grid->Add_Item("Upper Min", curve, UIPropertyGridDataType::Float, &emitter->curves[i].end_min);
				grid->Add_Item("Upper Max", curve, UIPropertyGridDataType::Float, &emitter->curves[i].end_max);
				grid->Add_Item("Lower Min", curve, UIPropertyGridDataType::Float, &emitter->curves[i].start_min);
				grid->Add_Item("Lower Max", curve, UIPropertyGridDataType::Float, &emitter->curves[i].start_max);
			}

			UIPropertyGridItem* events = grid->Add_Item("Events", emitter_node, UIPropertyGridDataType::None, NULL, std::vector<std::string>(), emitter);	
			grid->Add_Item_Button(events, "icon_add", ADD_EVENT_EVENT_ID);
			
			for (int j = 0; j < emitter->event_count; j++)
			{
				ParticleFX_Event* evt = &emitter->events[j];
				Add_Event_Node(j + 1, grid, events, evt);
			}
	}

	// Attractors
	for (int i = 0; i < m_fx->Get_Attractor_Count(); i++)
	{
		ParticleFX_AttractorConfig* attractor = m_fx->Get_Attractor(i);

		UIPropertyGridItem* attractor_node = grid->Add_Item(StringHelper::Format("Attractor %i", i + 1).c_str(), NULL, UIPropertyGridDataType::None, NULL, std::vector<std::string>(), attractor);
		grid->Add_Item_Button(attractor_node, "icon_subtract", DELETE_ATTRACTOR_EVENT_ID);

			grid->Add_Item("Name", attractor_node, UIPropertyGridDataType::String, &attractor->name);
			grid->Add_Item("X Offset", attractor_node, UIPropertyGridDataType::Float, &attractor->x_offset);
			grid->Add_Item("Y Offset", attractor_node, UIPropertyGridDataType::Float, &attractor->y_offset);
			grid->Add_Item("Strength", attractor_node, UIPropertyGridDataType::Float, &attractor->strength);
			grid->Add_Item("Radius", attractor_node, UIPropertyGridDataType::Float, &attractor->radius);
			grid->Add_Item("Track Effect", attractor_node, UIPropertyGridDataType::Bool, &attractor->track_effect);
	}
}

UIScene* UIScene_PfxEditor::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_PFXBackground");
}

const char* UIScene_PfxEditor::Get_Name()
{
	return "UIScene_PfxEditor";
}

bool UIScene_PfxEditor::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_PfxEditor::Should_Display_Cursor()
{
	return true;
}
	
bool UIScene_PfxEditor::Should_Display_Focus_Cursor()
{
	return false;
}

bool UIScene_PfxEditor::Is_Focusable()
{
	return true;
}

bool UIScene_PfxEditor::Should_Fade_Cursor()
{
	return false;
}

void UIScene_PfxEditor::Enter(UIManager* manager)
{
}	

void UIScene_PfxEditor::Exit(UIManager* manager)
{
}

void UIScene_PfxEditor::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
	Refresh_State();
}

void UIScene_PfxEditor::Refresh_State()
{
	Game* game = Game::Get();
	Map* map = game->Get_Map();

	Find_Element<UIMenuItem*>("show_collision_menu_item")->Set_Selected(m_collision_show);
	Find_Element<UIMenuItem*>("show_lighting_menu_item")->Set_Selected(m_show_lighting);

	ParticleManager::Get()->Set_Draw_Debug(m_collision_show);
	CollisionManager::Get()->Set_Show_Collision(m_collision_show);

	if (m_show_lighting)
	{
		RenderPipeline::Get()->Set_Ambient_Lighting(Color::Black.To_Vector4());
	}
	else
	{
		RenderPipeline::Get()->Set_Ambient_Lighting(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
	}
}

void UIScene_PfxEditor::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{		
	GfxDisplay* display = GfxDisplay::Get();
	Game* game = Game::Get();
	EditorCamera* sub_camera = static_cast<EditorCamera*>(game->Get_Camera(CameraID::Editor_Main));

	MouseState* mouse = Input::Get()->Get_Mouse_State();
	Vector2 mouse_pos = mouse->Get_Position();

	bool is_topmost = (manager->Get_Top_Scene_Index() == scene_index);
	UIScene_Game* game_scene = reinterpret_cast<UIScene_Game*>(manager->Get_Scene_By_Name("UIScene_Game"));
	
	UIPanel* camera_panel = Find_Element<UIPanel*>("camera_panel");
	Rect2D camera_panel_box	= camera_panel->Get_Screen_Box();

	// Spawn effect again if its finished.
	if (m_effect.Is_Finished())
	{
		m_effect = ParticleManager::Get()->Spawn(m_fx, m_effect_pos, m_effect_dir, rand() % 100, 0);
	//	DBG_LOG("Respawning effect!");
	}
	
	// Refresh property grid.
	if (m_needs_refresh == true)
	{		
		Refresh_Properties();
		m_needs_refresh = false;
	}

	// Moving effect?
	if (camera_panel_box.Intersects(mouse_pos))
	{
		if (mouse->Is_Button_Down(InputBindings::Mouse_Left))
		{
			Vector2 d_pos = sub_camera->Unproject(mouse_pos);
			m_effect_pos = Vector3(d_pos.X, d_pos.Y, m_effect_pos.Z);

		}
		else if (mouse->Is_Button_Down(InputBindings::Mouse_Right))
		{
			m_effect_dir += HALFPI * time.Get_Delta(); // 90 degrees per second?
		}
	}

	// Update effect position.
	m_effect.Set_Position(m_effect_pos);
	m_effect.Set_Target(Vector3(0.0f, 0.0f, 0.0f));
	m_effect.Set_Direction(m_effect_dir);

	m_collector_handle.Set_Position(m_effect_pos + Vector3(32.0f, 32.0f, 0.0f));

	if (is_topmost)
	{
		/*
		// Open the chat menu.
		if (Input::Get()->Was_Pressed(OutputBindings::Chat))
		{
			manager->Play_UI_Sound(UISoundType::Small_Click);
			manager->Go(UIAction::Push(new UIScene_Chat(game_scene, false), new UIFadeInTransition()));
			return;
		}*/

		if (Input::Get()->Was_Pressed(InputBindings::Keyboard_Space))
		{
			ParticleManager::Get()->Reset();		
			m_collector_handle =  ParticleManager::Get()->Create_Collector(Vector3(16.0f, 16.0f, 16.0f), 48.0f, 40.0f);
		}
	}


	UIScene::Tick(time, manager, scene_index);
}

void UIScene_PfxEditor::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Get some junk we need.
	GfxDisplay*		display		= GfxDisplay::Get();
	Camera*			last_camera = RenderPipeline::Get()->Get_Active_Camera();
	RenderPipeline* pipeline	= RenderPipeline::Get();
	Renderer*		renderer	= Renderer::Get();
	Game*			game		= Game::Get();
	Map*			map			= game->Get_Map();
	Atlas*			tileset		= map->Get_Tileset()->Get();
	Vector2			tile_size	= map->Get_Tile_Size();

	UIScene*		game_scene		= manager->Get_Scene_By_Name("UIScene_Game");
	EditorCamera*	sub_camera		= static_cast<EditorCamera*>(game->Get_Camera(CameraID::Editor_Sub));
	EditorCamera*	ui_camera		= static_cast<EditorCamera*>(game->Get_Camera(CameraID::UI));
	
	UIPanel*		camera_panel		= Find_Element<UIPanel*>("camera_panel");
	Rect2D			camera_panel_box	= camera_panel->Get_Screen_Box();

	PrimitiveRenderer pr;

	// Update input-active areas of cameras.	
	sub_camera->Set_Input_Viewport(camera_panel_box);

	// Draw all post scene elements.
	UIScene::Draw(time, manager, scene_index);

	// Draw some debug info.
	m_font_renderer.Draw_String(StringHelper::Format("Total Particles: %i", ParticleManager::Get()->Get_Particle_Count()).c_str(), Rect2D(10.0f, 34.0f, display->Get_Width() - 20.0f, 16.0f), 8.0f, Color::White, TextAlignment::Right, TextAlignment::Top);
	m_font_renderer.Draw_String(StringHelper::Format("Total Effects: %i", ParticleManager::Get()->Get_Effect_Count()).c_str(), Rect2D(10.0f, 44.0f, display->Get_Width() - 20.0f, 16.0f), 8.0f, Color::White, TextAlignment::Right, TextAlignment::Top);
	m_font_renderer.Draw_String(StringHelper::Format("Total Emitters: %i", ParticleManager::Get()->Get_Emitter_Count()).c_str(), Rect2D(10.0f, 54.0f, display->Get_Width() - 20.0f, 16.0f), 8.0f, Color::White, TextAlignment::Right, TextAlignment::Top);
	m_font_renderer.Draw_String(StringHelper::Format("Total Attractors: %i", ParticleManager::Get()->Get_Attractor_Count()).c_str(), Rect2D(10.0f, 64.0f, display->Get_Width() - 20.0f, 16.0f), 8.0f, Color::White, TextAlignment::Right, TextAlignment::Top);

	// Draw game overlays.
	static_cast<UIScene_Game*>(manager->Get_Scene(scene_index - 2))->Draw(time, manager, scene_index - 2);
}

void UIScene_PfxEditor::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::MenuItem_Click:
		{
			if (e.Source->Get_Name() == "new_menu_item")
				New();
			if (e.Source->Get_Name() == "open_menu_item")
				Begin_Open();
			if (e.Source->Get_Name() == "save_menu_item")
				Begin_Save();
			if (e.Source->Get_Name() == "save_as_menu_item")
				Begin_SaveAs();
			if (e.Source->Get_Name() == "exit_menu_item")
				manager->Go(UIAction::Pop());		
			if (e.Source->Get_Name() == "reset_camera_menu_item")
				Reset_Camera();	
			if (e.Source->Get_Name() == "show_collision_menu_item")
			{
				m_collision_show = !m_collision_show;
				Refresh_State();
			}
			if (e.Source->Get_Name() == "show_lighting_menu_item")
			{
				m_show_lighting = !m_show_lighting;
				Refresh_State();
			}
			break;
		}
	case UIEventType::ToolbarItem_Click:
		{
			if (e.Source->Get_Name() == "add_effect_toolbar_item")
				Add_Effect();
			else if (e.Source->Get_Name() == "add_attractor_toolbar_item")
				Add_Attractor();

			break;
		}
	case UIEventType::PropertyGrid_Changed:
		{
			//m_needs_refresh = true;
			Update_Properties();
			Precalculate_Curve();
			break;
		}
	case UIEventType::PropertyGrid_ButtonClicked:
		{
			UIPropertyGrid* grid = reinterpret_cast<UIPropertyGrid*>(e.Source);

			// 0 = delete button.
			if (e.Param == DELETE_EMITTER_EVENT_ID)
			{
				UIPropertyGridItem* item = reinterpret_cast<UIPropertyGridItem*>(e.Sub_Source);
				Delete_Effect(reinterpret_cast<ParticleFX_EmitterConfig*>(item->Meta_Data));
			}
			else if (e.Param == DELETE_ATTRACTOR_EVENT_ID)
			{
				UIPropertyGridItem* item = reinterpret_cast<UIPropertyGridItem*>(e.Sub_Source);
				Delete_Attractor(reinterpret_cast<ParticleFX_AttractorConfig*>(item->Meta_Data));
			}
			else if (e.Param == ADD_EVENT_EVENT_ID)
			{
				UIPropertyGridItem* item = reinterpret_cast<UIPropertyGridItem*>(e.Sub_Source);
				Add_Event(grid, item, reinterpret_cast<ParticleFX_EmitterConfig*>(item->Meta_Data));
			}
			else if (e.Param == DELETE_EVENT_EVENT_ID)
			{
				UIPropertyGridItem* item = reinterpret_cast<UIPropertyGridItem*>(e.Sub_Source);
				Remove_Event(grid, item, reinterpret_cast<ParticleFX_Event*>(item->Meta_Data));
			}
			break;
		}
	case UIEventType::PropertyGrid_ExternalClicked:
		{
			UIPropertyGridItem* item = reinterpret_cast<UIPropertyGridItem*>(e.Sub_Source);
			if (e.Param == CURVE_CLICK_EVENT_ID)
			{
				Update_Curves();
				m_edit_curve = reinterpret_cast<ParticleFX_Curve*>(item->Meta_Data);
				Refresh_Curve();
			}
			else if (e.Param == SPRITE_CLICK_EVENT_ID)
			{
				m_edit_resource_name = reinterpret_cast<std::string*>(item->Meta_Data);
				m_manager->Go(UIAction::Push(new UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Image, *m_edit_resource_name), new UIFadeInTransition()));	
			}
			else if (e.Param == ANIMATION_CLICK_EVENT_ID)
			{
				m_edit_resource_name = reinterpret_cast<std::string*>(item->Meta_Data);
				m_manager->Go(UIAction::Push(new UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Animation, *m_edit_resource_name), new UIFadeInTransition()));	
			}
			else if (e.Param == SOUND_CLICK_EVENT_ID)
			{
				m_edit_resource_name = reinterpret_cast<std::string*>(item->Meta_Data);
				m_manager->Go(UIAction::Push(new UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Audio, *m_edit_resource_name), new UIFadeInTransition()));	
			}
			break;
		}
	case UIEventType::CurveEditor_Modified:
		{
			Update_Curves();
			break;
		}
	case UIEventType::Dialog_Close:
		{
			UIScene_FileBrowser* browser = dynamic_cast<UIScene_FileBrowser*>(e.Scene);
			if (browser != NULL)
			{
				std::string path = browser->Get_Selected_Path();
				if (path != "")
				{
					if (m_saving == true)
					{
						Save(path);
					}
					else
					{
						Open(path);
					}
				}
			}

			UIScene_ResourceBrowser* res_browser = dynamic_cast<UIScene_ResourceBrowser*>(e.Scene);
			if (res_browser != NULL)
			{
				std::string path = res_browser->Get_Selected_Resource();
				if (path != "")
				{
					*m_edit_resource_name = path;
					Update_Properties();
				}
			}
			break;
		}
	}
}
