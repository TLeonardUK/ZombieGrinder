// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UITransition.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/GameRunner.h"

#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/UI/Layouts/UILayout.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Layouts/UILayoutHandle.h"

#include "Engine/UI/Transitions/UIFadeInTransition.h"

#include "Engine/Online/OnlineMatching.h"

#include "Engine/Display/GfxDisplay.h"

#include "Engine/Platform/Platform.h"

#include "Generic/Math/Math.h"

#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasFactory.h"
#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Online/OnlineMatching.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include "Engine/Input/Input.h"

#include "Engine/Profiling/ProfilingManager.h"

#include <math.h>
#include <algorithm>

UIManager::UIManager()
	: m_scene_index(-1)
	, m_scene_change_time(0.0f)
	, m_last_atlas_version(-1)
	, m_transition_in_progress(false)
	, m_stack_stored(false)
	, m_cursor_type(UICursorType::Pointer)
	, m_camera(NULL)
	, m_selection_cursor_position(0, 0)
	, m_selection_cursor_target_position(0, 0)
	, m_focused_element(NULL)
	, m_last_mouse_position(0, 0)
	, m_mouse_fade_timer(0.0f)
	, m_navigation_disabled(false)
	, m_mouse_selection_element(NULL)
	, m_cursor_offset_timer(0.0f)
	, m_cursor_offset_direction(false)
{
	// Load the UI atlas.
	m_ui_atlas = ResourceFactory::Get()->Get_Atlas("UI");
	DBG_ASSERT(m_ui_atlas != NULL);

	m_atlas_renderer = new AtlasRenderer(m_ui_atlas);

	// Load the UI font.
	m_ui_font = ResourceFactory::Get()->Get_Font("UI");
	DBG_ASSERT(m_ui_font != NULL);

	// Clear out scene list.
	memset(m_scenes, 0, sizeof(UIScene*) * UI_MAX_SCENES);

	// Refresh resources.
	Refresh();
}

UIManager::~UIManager()
{
	SAFE_DELETE(m_atlas_renderer);

	while (m_scene_index-- >= 0)
	{
		SAFE_DELETE(m_scenes[m_scene_index + 1]);
	}

	if (m_transition_in_progress == true)
	{
		SAFE_DELETE(m_transition_action.Scene);
		SAFE_DELETE(m_transition_action.Transition);

		m_transition_in_progress = false;
	}

	for (HashTable<UIScene*, unsigned int>::Iterator iter = m_persistent_scenes.Begin(); iter != m_persistent_scenes.End(); iter++)
	{
		UIScene* scene = *iter;
		SAFE_DELETE(scene);
	}
	m_persistent_scenes.Clear();
}

void UIManager::Delete_Scene(UIScene* scene)
{
	for (std::vector<Queued_Event>::iterator iter = m_queued_events.begin(); iter != m_queued_events.end(); )
	{
		Queued_Event qe = *iter;
		if (qe.scene == scene || qe.evt.Scene == scene)
		{ 
			DBG_LOG("Purging deleted scene event.");
			iter = m_queued_events.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	SAFE_DELETE(scene);
}

bool UIManager::Transition_In_Progress()
{
	return m_transition_in_progress;
}

bool UIManager::Pending_Actions()
{
	return m_queued_events.size() > 0 || m_defered_go_actions.size() > 0;
}

void UIManager::Limit_Mouse_Selection_To_Element(UIElement* element)
{
	m_mouse_selection_element = element;
}

void UIManager::Queue_Event(UIEvent e, UIScene* scene)
{
	Queued_Event evt;
	evt.scene = scene;
	evt.evt = e;
	m_queued_events.push_back(evt);
}

void UIManager::Store_Focused_Element(UIScene* scene, UIElement* element)
{
	if (element == NULL)
	{
		return;
	}

	int index = std::find(scene->Get_Elements().begin(), scene->Get_Elements().end(), element) - scene->Get_Elements().begin();
	int hash = StringHelper::Hash(scene->Get_Name());
	
	if (m_last_focused_element_index_per_scene.Contains(hash))
	{
		m_last_focused_element_index_per_scene.Remove(hash);
	}
	m_last_focused_element_index_per_scene.Set(hash, index);
}

void UIManager::Restore_Focused_Element(UIScene* scene)
{
	int last_index = -1;
	if (m_last_focused_element_index_per_scene.Get(StringHelper::Hash(scene->Get_Name()), last_index))
	{
		if (last_index >= 0 && last_index < (int)scene->Get_Elements().size())
		{
			UIElement* element = scene->Get_Elements().at(last_index);
			if (element != NULL)
			{
				Focus(element);
			}
		}
	}
}

void UIManager::Store_Persistent_Scene(UIScene* scene)
{
	m_persistent_scenes.Set(StringHelper::Hash(scene->Get_Name()), scene);
}

UIScene* UIManager::Retrieve_Persistent_Scene(const char* name)
{
	return m_persistent_scenes.Get(StringHelper::Hash(name));
}

UIScene* UIManager::Get_Top_Focusable_Screen(const char* stopAtName)
{
	for (int i = m_scene_index; i >= 0; i--)
	{
		if (m_scenes[i]->Is_Focusable() || m_scenes[i]->Get_Name() == stopAtName)
		{
			return m_scenes[i];
		}
	}
	return NULL;
}

UIScene* UIManager::Get_Scene(int offset)
{
	if (m_scene_index < 0)
		return NULL;
	return m_scenes[offset];//m_scenes[m_scene_index - (offset + 1)];
}

int	UIManager::Get_Top_Scene_Index()
{
	return m_scene_index;
}

UIScene* UIManager::Get_Scene_From_Top(int offset)
{
	return m_scenes[m_scene_index - offset];
}

AtlasHandle* UIManager::Get_Atlas()
{
	return m_ui_atlas;
}

FontHandle* UIManager::Get_Font()
{
	return m_ui_font;
}

Camera*	UIManager::Get_Camera()
{
	return m_camera;
}

void UIManager::Set_Camera(Camera* camera)
{
	m_camera = camera;
}

void UIManager::Set_Cursor(UICursorType::Type cursor)
{
	m_cursor_type = cursor;
}

Vector2 UIManager::Get_UI_Scale()
{
	float scale = GfxDisplay::Get()->Get_Height() / 240.0f;
	return Vector2(scale, scale);
}

UIElement* UIManager::Get_Focused_Element()
{
	return m_focused_element;
}

void UIManager::Disable_Navigation()
{
	m_navigation_disabled = true;
}

void UIManager::Enable_Navigation()
{	
	m_navigation_disabled = false;
}

UIElement* UIManager::Find_Element_By_Position(Vector2 position, bool only_mouse_hot, bool focusable_only)
{
	UIScene* curr_scene = m_scenes[m_scene_index];

	UIElement* element = curr_scene->Find_Element_By_Position(this, position, m_scene_index, only_mouse_hot, focusable_only);
	if (element != NULL)
	{
		return element;
	}

	return NULL;
}

void UIManager::Refresh()
{
	// Get cursors.
	m_cursor_frames[(int)UICursorType::Pointer]		= m_ui_atlas->Get()->Get_Frame("cursor_pointer");
	m_cursor_frames[(int)UICursorType::SplitH]		= m_ui_atlas->Get()->Get_Frame("cursor_split_horizontal");
	m_cursor_frames[(int)UICursorType::SplitV]		= m_ui_atlas->Get()->Get_Frame("cursor_split_vertical");
	m_cursor_frames[(int)UICursorType::HandPointer] = m_ui_atlas->Get()->Get_Frame("screen_main_cursor");

	// Get audio.
	m_sounds[(int)UISoundType::Click]			= ResourceFactory::Get()->Get_Sound("sfx_screens_main_click");
	m_sounds[(int)UISoundType::Small_Click]		= ResourceFactory::Get()->Get_Sound("sfx_screens_main_small_click");
	m_sounds[(int)UISoundType::Character]		= ResourceFactory::Get()->Get_Sound("sfx_screens_main_char");
	m_sounds[(int)UISoundType::Back]			= ResourceFactory::Get()->Get_Sound("sfx_screens_main_back");
	m_sounds[(int)UISoundType::Close_Popup]		= ResourceFactory::Get()->Get_Sound("sfx_screens_main_close_popup");
	m_sounds[(int)UISoundType::Hover]			= ResourceFactory::Get()->Get_Sound("sfx_screens_main_hover");
	m_sounds[(int)UISoundType::Paint]			= ResourceFactory::Get()->Get_Sound("sfx_screens_main_paint");
	m_sounds[(int)UISoundType::Popup]			= ResourceFactory::Get()->Get_Sound("sfx_screens_main_popup");
	m_sounds[(int)UISoundType::Select]			= ResourceFactory::Get()->Get_Sound("sfx_screens_main_select");
	m_sounds[(int)UISoundType::Enter_SubMenu]	= ResourceFactory::Get()->Get_Sound("sfx_screens_sub_enter");
	m_sounds[(int)UISoundType::Exit_SubMenu]	= ResourceFactory::Get()->Get_Sound("sfx_screens_sub_exit");
	m_sounds[(int)UISoundType::Pop]				= ResourceFactory::Get()->Get_Sound("sfx_screens_main_pop");
	m_sounds[(int)UISoundType::Thud]			= ResourceFactory::Get()->Get_Sound("sfx_screens_main_thud");
	m_sounds[(int)UISoundType::Coin_Decrease]	= ResourceFactory::Get()->Get_Sound("sfx_hud_score_decrease");
	m_sounds[(int)UISoundType::Coin_Increase]	= ResourceFactory::Get()->Get_Sound("sfx_hud_score_increase");

	// Refresh scenes.
	for (int i = 0; i <= m_scene_index; i++)
	{
		m_scenes[i]->Refresh(this);
	}

	// Refresh persistent backgrounds.
	for (HashTable<UIScene*, unsigned int>::Iterator iter = m_persistent_scenes.Begin(); iter != m_persistent_scenes.End(); iter++)
	{
		(*iter)->Refresh(this);
	}

	// Stores last version.
	m_last_atlas_version = m_ui_atlas->Get_Version();
}

void UIManager::Tick(const FrameTime& time)
{
	// Defered go-action?
	while (m_defered_go_actions.size() > 0)
	{
		UIAction action = *m_defered_go_actions.begin();
		m_defered_go_actions.erase(m_defered_go_actions.begin());

		Perform_Go(action);
	}
	
	// Grab current scene.
	UIScene* curr_scene = m_scenes[m_scene_index];

	// Refresh states.	
	if (m_last_atlas_version != m_ui_atlas->Get_Version())
	{
		Refresh();
	}

	// Pending scene delete?
	for (std::vector<UIScene*>::iterator iter = m_to_delete.begin(); iter != m_to_delete.end(); iter++)
	{
		DBG_LOG("[D] Deleting scene '%s'.", (*iter)->Get_Name());	
		Delete_Scene(*iter);
	}

	// Reset cursor to default.
	m_cursor_type = UICursorType::Pointer;
	m_hotkey_used_this_frame = false;

	if (m_transition_in_progress == true)
	{
		Tick_Background(time);	
		//Tick_Always(time);

		if (m_transition_action.Transition->Is_Complete())
		{
			Complete_Transition();
		}
		else
		{
			m_transition_action.Transition->Tick(time, this, m_scenes[m_scene_index], m_scene_index, true);

			Store_Stack();
			Apply_Action_To_Stack(m_transition_action);
			m_transition_action.Transition->Tick(time, this, m_scenes[m_scene_index], m_scene_index, false);
			Restore_Stack();
		}
	}
	else
	{
		// Tick the scene.
		Tick_Background(time);
		curr_scene->Tick(time, this, m_scene_index);
	}

	// Change selection.
	Update_Selection_Cursor(time);

	// Dispatch queued events.	
	std::vector<Queued_Event> actions = m_queued_events; // We create this copy as events can produce more events ;_;
	m_queued_events.clear();

	for (std::vector<Queued_Event>::iterator iter = actions.begin(); iter != actions.end(); iter++)
	{
		Queued_Event qe = *iter;
		qe.scene->Internal_Dispatch_Event(this, qe.evt);
	}
}

void UIManager::Complete_Transition()
{
	// Reset offset for both scenes, as offset should only be used in transitions!
	m_scenes[m_scene_index]->Reset_Element_Offsets();

	Store_Stack();
	Apply_Action_To_Stack(m_transition_action);
	m_scenes[m_scene_index]->Reset_Element_Offsets();
	Restore_Stack();

	Apply_Action_To_Stack(m_transition_action);
	m_transition_in_progress = false;

	SAFE_DELETE(m_transition_action.Transition);
}

bool UIManager::Draw_Game_BG(const FrameTime& time)
{
	PROFILE_SCOPE_BUDGETED("UI Background Render", 5.0f);

	// If in game/editor, we need to render the game background, we do
	// this here rather than in a "Background" so we don't end up rendering it twice (expensive!)
	// during transitions.
	UIScene* scene = Get_Scene_By_Name("UIScene_Game");
	if (scene != NULL)
	{
		Retrieve_Persistent_Scene("UIScene_GameBackground")->Draw(time, this, 0);
		return true;
	}

	return false;
}

void UIManager::Draw(const FrameTime& time)
{
	UIScene* curr_scene = m_scenes[m_scene_index];
	UIScene* next_scene = NULL;
	UIScene* scene = NULL;

	RenderPipeline* pipeline = RenderPipeline::Get();

	bool game_bg_drawn = false;

	{
		{
			PROFILE_SCOPE_BUDGETED("Render UI Setup", 5.0f);

			Renderer::Get()->Set_Depth_Test(false);
			Renderer::Get()->Set_Depth_Write(true);
		}

		if (!game_bg_drawn)
		{
			game_bg_drawn = Draw_Game_BG(time);
		}
	}

	if (m_transition_in_progress == true)
	{
		UITransitionDrawOrder::Type order = m_transition_action.Transition->Get_Draw_Order();

		bool new_first = true;

		switch (order)
		{
			case UITransitionDrawOrder::NewFirstOnPopElseOld:	new_first = (m_transition_action.Action == UIAction::Action::Pop); break;
			case UITransitionDrawOrder::OldFirstOnPopElseNew:	new_first = (m_transition_action.Action != UIAction::Action::Pop); break;
			case UITransitionDrawOrder::NewFirst:				new_first = true;
			case UITransitionDrawOrder::OldFirst:				new_first = false;
		}

		if (new_first)
		{
			Store_Stack();
			Apply_Action_To_Stack(m_transition_action);

			if (!game_bg_drawn)
			{
				game_bg_drawn = Draw_Game_BG(time);
			}
			m_transition_action.Transition->Draw(time, this, m_scenes[m_scene_index], m_scene_index, false);
			pipeline->Flush_Batches();
			next_scene = m_scenes[m_scene_index];

			Restore_Stack();

			m_transition_action.Transition->Draw(time, this, m_scenes[m_scene_index], m_scene_index, true);
		}
		else
		{
			m_transition_action.Transition->Draw(time, this, m_scenes[m_scene_index], m_scene_index, true);
			pipeline->Flush_Batches();

			Store_Stack();
			Apply_Action_To_Stack(m_transition_action);

			if (!game_bg_drawn)
			{
				game_bg_drawn = Draw_Game_BG(time);
			}

			m_transition_action.Transition->Draw(time, this, m_scenes[m_scene_index], m_scene_index, false);
			next_scene = m_scenes[m_scene_index];

			Restore_Stack();
		}
	}
	else if (curr_scene != NULL)
	{
		PROFILE_SCOPE_BUDGETED("Render Stack Base", 5.0f);

		Draw_Stack(time);
	}

	{
		PROFILE_SCOPE_BUDGETED("Render Cursor", 5.0f);
	
		// Draw the selection cursor.
		Draw_Selection_Cursor(time, curr_scene, next_scene);
	}

	{
		PROFILE_SCOPE_BUDGETED("Render Overlays", 5.0f);

		GameEngine::Get()->Get_Runner()->Draw_Overlays(time);
	}

	{
		PROFILE_SCOPE_BUDGETED("Render OnScreen Logs", 5.0f);

		GameEngine::Get()->Get_Runner()->Draw_Onscreen_Logs(time);
	}
	{
		PROFILE_SCOPE_BUDGETED("Render Watermark", 5.0f);

		GameEngine::Get()->Get_Runner()->Draw_Watermark(time);
	}
}

bool UIManager::Get_Hot_Keys_Enabled(UIElement* element)
{
	return Is_Element_Focusable(element);
}

bool UIManager::Is_Element_Focusable(UIElement* element)
{
	UIScene* curr_scene = m_scenes[m_scene_index];

	std::vector<UIElement*> elements = curr_scene->Get_Elements();
			
	if (m_mouse_selection_element != NULL)
	{
		elements = m_mouse_selection_element->Get_Elements();
	}

	std::vector<UIElement*> children;

	for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
	{
		UIElement* sub_element = *iter;

		for (std::vector<UIElement*>::iterator child_iter = sub_element->m_children.begin(); child_iter != sub_element->m_children.end(); child_iter++)
		{
			UIElement* child_element = *child_iter;
			children.push_back(child_element);
		}
	}

	return (std::find(elements.begin(), elements.end(), element) != elements.end()) || 
		   (std::find(children.begin(), children.end(), element) != children.end());
}

bool UIManager::Check_Hotkey_Valid(OutputBindings::Type key)
{
	bool bUsed = m_hotkey_used_this_frame;
	m_hotkey_used_this_frame = true;
	return !bUsed;
}

bool UIManager::Mouse_Selection_Restricted()
{
	return (m_mouse_selection_element != NULL);
}

bool UIManager::Was_Pressed(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		Input* input = Input::Get(i);
		if (input->Was_Pressed(type))
		{
			return true;
		}
	}
	return false;
}

bool UIManager::Was_Down(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		Input* input = Input::Get(i);
		if (input->Was_Down(type))
		{
			return true;
		}
	}
	return false;
}

bool UIManager::Is_Down(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		Input* input = Input::Get(i);
		if (input->Is_Down(type))
		{
			return true;
		}
	}
	return false;
}

void UIManager::Update_Selection_Cursor(const FrameTime& time)
{
	UIScene* curr_scene = m_scenes[m_scene_index];

	if (m_transition_in_progress == false && curr_scene->Is_Focusable() && m_navigation_disabled == false)
	{
		// Up / Down will always focus on the next item.
		if (Was_Pressed(OutputBindings::GUI_Up))
		{
			// Let focused element attempt to override the focus selection
			// before we do the default.
			if (m_focused_element == NULL || m_focused_element->Focus_Up(this) == false)
			{
				Focus_Previous();
			}
		}
		else if (Was_Pressed(OutputBindings::GUI_Down))
		{
			if (m_focused_element == NULL || m_focused_element->Focus_Down(this) == false)
			{
				Focus_Next();
			}
		}

		// Left / Right will change settings in an item.
		if (Was_Pressed(OutputBindings::GUI_Left))
		{
			if (m_focused_element == NULL || m_focused_element->Focus_Left(this) == false)
			{
				Focus_Previous();
			}
		}
		else if (Was_Pressed(OutputBindings::GUI_Right))
		{
			if (m_focused_element == NULL || m_focused_element->Focus_Right(this) == false)
			{
				Focus_Next();
			}
		}

		// Move cursor to mouse if we have moved recently.
		Vector2 mouse_position = Input::Get()->Get_Mouse_State()->Get_Position();
		Vector2 mouse_delta = mouse_position - m_last_mouse_position;
		if (fabs(mouse_delta.X) > 3 || fabs(mouse_delta.Y) > 3)
		{
			/*
			std::vector<UIElement*> elements = curr_scene->Get_Elements();
			
			if (m_mouse_selection_element != NULL)
			{
				elements = m_mouse_selection_element->Get_Elements();
			}

			for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
			{
				UIElement* e = *iter;
				if (e->Is_Focusable() == true && e->Get_Screen_Box().Intersects(mouse_position))
				{
					if (e != m_focused_element)
					{				
						Play_UI_Sound(UISoundType::Click);
					}
					Focus(e);				
				}
			}*/

			UIElement* elm = Find_Element_By_Position(mouse_position, false, true);
			if (elm != m_focused_element && elm != NULL && elm->Get_Visible())
			{		
				Play_UI_Sound(UISoundType::Click);
				Focus(elm);
			}

			m_last_mouse_position = mouse_position;
			m_mouse_fade_timer = 0.0f;		
		}
		else
		{
			m_mouse_fade_timer += time.Get_Frame_Time();
		}

	}

	// Update focal point.
	if (m_focused_element != NULL && m_transition_in_progress == false)
	{
		// Work out the target focus position.
		m_selection_cursor_target_position = m_focused_element->Get_Focus_Point();
	}

	// Update cursor position.
	float diff_x = Math::SmoothStep(m_selection_cursor_position.X, m_selection_cursor_target_position.X, 0.1f) - m_selection_cursor_position.X;
	float diff_y = Math::SmoothStep(m_selection_cursor_position.Y, m_selection_cursor_target_position.Y, 0.1f) - m_selection_cursor_position.Y;
	m_selection_cursor_position.X += (diff_x * time.Get_Delta()) * 200.0f;
	m_selection_cursor_position.Y += (diff_y * time.Get_Delta()) * 200.0f;
}

void UIManager::Play_UI_Sound(UISoundType::Type type)
{
	m_sounds[(int)type]->Get()->Play();
}

void UIManager::Clear_Focus()
{
	m_focused_element = NULL;
}

void UIManager::Focus_Next(bool disable_audio)
{
	// Play audio.
	if (disable_audio == false)
	{
		Play_UI_Sound(UISoundType::Click);
	}

	// Find and focus on next element.
	std::vector<UIElement*>& elements = m_scenes[m_scene_index]->Get_Elements();
	if (elements.size() > 0)
	{
		bool passed_current = false;

		for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
		{
			UIElement* e = *iter;
			if (e->Is_Focusable() && 
				e->Get_Visible() &&
				passed_current == true)
			{
				Focus(e);
				return;
			}
			if (e == m_focused_element)
			{
				passed_current = true;
			}
		}

		// Focus on first element.		
		for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
		{
			UIElement* e = *iter;
			if (e->Is_Focusable() && e->Get_Visible())
			{
				Focus(e);
				return;
			}
		}

		Clear_Focus();
	}
	else
	{
		Clear_Focus();
	}
}

void UIManager::Focus_Previous(bool disable_audio)
{
	// Play audio.
	if (disable_audio == false)
	{
		Play_UI_Sound(UISoundType::Click);
	}

	// Find and focus on next element.
	std::vector<UIElement*>& elements = m_scenes[m_scene_index]->Get_Elements();
	if (elements.size() > 0)
	{
		UIElement* last_focusable = NULL;

		for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
		{
			UIElement* e = *iter;
			if (e == m_focused_element)
			{
				if (last_focusable != NULL)
				{
					Focus(last_focusable);
					return;
				}
				else
				{
					break;
				}
			}
			if (e->Is_Focusable() && e->Get_Visible())
			{
				last_focusable = e;
			}
		}

		// Focus on last element.		
		last_focusable = NULL;
		for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
		{
			UIElement* e = *iter;
			if (e->Is_Focusable() && e->Get_Visible())
			{
				last_focusable = e;
			}
		}

		if (last_focusable != NULL)
		{
			Focus(last_focusable);
			return;
		}

		Clear_Focus();
	}
	else
	{
		Clear_Focus();
	}
}

void UIManager::Focus(UIElement* element)
{
	if (m_focused_element == element)
	{
		return;
	}

	// Work out the target focus position.
//	m_selection_cursor_target_position = element->Get_Focus_Point();
	
	// If first focus on a screen, go directly to the cursor position.
	if (m_focused_element == NULL)
	{
//		m_selection_cursor_position = m_selection_cursor_target_position;
	}

	m_focused_element = element;

	// Tell element they have been focused on.
	element->Focus();

	// Throw out the focus event.
	UIEvent evnt(UIEventType::Element_Focus, element);
	element->Get_Scene()->Dispatch_Event(this, evnt);
}

void UIManager::Draw_Selection_Cursor(const FrameTime& time, UIScene* curr_scene, UIScene* next_scene)
{
	PROFILE_SCOPE("Draw Selection Cursor");

	// Perform a little in-out offset animation for the selection cursor.
	m_cursor_offset_timer += time.Get_Frame_Time();
	if (m_cursor_offset_timer >= CURSOR_OFFSET_INTERVAL)
	{
		m_cursor_offset_timer = 0.0f;
		m_cursor_offset_direction = !m_cursor_offset_direction;
	}

	float delta = m_cursor_offset_timer / CURSOR_OFFSET_INTERVAL;
	if (m_cursor_offset_direction == true)
	{
		delta = 1.0f - delta;
	}

	// Draw the selection cursor.
	AtlasFrame* cursor_frame = m_cursor_frames[UICursorType::HandPointer];
	Vector2 cursor_offset(m_selection_cursor_position.X + Math::SmoothStep(CURSOR_OFFSET_MAX, 0, delta), m_selection_cursor_position.Y);	
	Vector2 cursor_scale = Get_UI_Scale();
	float cursor_alpha = 1.0f;

	// Fade cursor out if current scene is not focusable.
	if (m_transition_in_progress == true)
	{
		if (curr_scene->Is_Focusable() == true && next_scene->Is_Focusable() == true)
		{
			cursor_alpha = 1.0f;
		}
		else if (curr_scene->Is_Focusable() == false && next_scene->Is_Focusable() == true)
		{
			cursor_alpha = m_transition_action.Transition->Get_Delta();
		}
		else if (curr_scene->Is_Focusable() == true && next_scene->Is_Focusable() == false)
		{
			cursor_alpha = 1.0f - m_transition_action.Transition->Get_Delta();
		}
		else
		{
			cursor_alpha = 0.0f;
		}
	}
	else if (curr_scene->Is_Focusable() == false)
	{
		cursor_alpha = 0.0f;
	}

	// Draw selection cursor!
	if (curr_scene != NULL && curr_scene->Should_Display_Focus_Cursor() && curr_scene->Is_Focusable() == true)
	{
		m_atlas_renderer->Draw_Frame
		(
			cursor_frame, 
			Rect2D(cursor_offset.X, cursor_offset.Y, cursor_frame->Rect.Width, cursor_frame->Rect.Height), 
			0.0f, 
			Color(255.0f, 255.0f, 255.0f, 255.0f * cursor_alpha), 
			false, 
			false,
			RendererOption::E_Src_Alpha_One_Minus_Src_Alpha,
			cursor_scale
		);
	}

	// Draw mouse cursor (only used for editor etc).
	if (curr_scene != NULL && curr_scene->Should_Display_Cursor() && curr_scene->Is_Focusable() == true)
	{
		float mouse_fade = 1.0f - (Max(0.0f, m_mouse_fade_timer - MOUSE_FADE_DELAY) / MOUSE_FADE_INTERVAL);

		if (curr_scene->Should_Fade_Cursor())
		{
			mouse_fade = 1.0f;
		}

		if (mouse_fade > 0.0f)
		{
			m_atlas_renderer->Draw_Frame
			(
				m_cursor_frames[m_cursor_type], 
				Input::Get(0)->Get_Mouse_State()->Get_Position().Round(),
				0.0f,
				Color(255.0f, 255.0f, 255.0f, 255.0f * cursor_alpha * mouse_fade)
			);
		}
	}
}

void UIManager::Draw_Stack(const FrameTime& time, bool draw_background)
{	
	PROFILE_SCOPE_BUDGETED("Render UI Stack", 8.0f);

	if (draw_background == true)
	{
		Draw_Background(time);
	}

	{
		m_scenes[m_scene_index]->Draw(time, this, m_scene_index);
	}
}

UIScene* UIManager::Get_Scene_By_Name(const char* name)
{
	for (int i = m_scene_index; i >= 0; i--)
	{
		UIScene* scene = m_scenes[i];
		if (strcmp(scene->Get_Name(), name) == 0)
		{
			return scene;
		}
	}
	return NULL;
}

UIScene* UIManager::Get_Scene_Below(UIScene* find)
{
	for (int i = m_scene_index; i >= 0; i--)
	{
		UIScene* scene = m_scenes[i];
		if (scene == find && i > 0)
		{
			return m_scenes[i - 1];
		}
	}
	return NULL;
}

void UIManager::Tick_Background(const FrameTime& time)
{
	UIScene* background = NULL;
	for (int i = m_scene_index; i >= 0; i--)
	{
		UIScene* scene = m_scenes[i];
		UIScene* scene_bg = scene->Get_Background(this);
		if (scene_bg != NULL)
		{
			background = scene_bg;
			break;
		}
		if (!scene->Get_Should_Render_Lower_Scenes_Background())
		{
			break;
		}
	}

	if (background != NULL)
	{
		background->Tick(time, this, m_scene_index);
	}
}

void UIManager::Tick_Always(const FrameTime& time)
{
	UIScene* background = NULL;
	for (int i = m_scene_index; i >= 0; i--)
	{
		UIScene* scene = m_scenes[i];
		if (scene->Should_Tick_When_Not_Top())
		{
			scene->Tick(time, this, i);
		}
	}
}

void UIManager::Draw_Background(const FrameTime& time)
{
	PROFILE_SCOPE("Draw Background");

	UIScene* background = NULL;
	for (int i = m_scene_index; i >= 0; i--)
	{
		UIScene* scene = m_scenes[i];
		UIScene* scene_bg = scene->Get_Background(this);
		if (scene_bg != NULL)
		{
			background = scene_bg;
			break;
		}
		if (!scene->Get_Should_Render_Lower_Scenes_Background())
		{
			break;
		}
	}

//	DBG_ONSCREEN_LOG_STRID("Background", Color::Blue, 1.0f, "UI Background = %s", background != NULL ? background->Get_Name() : "NULL");

	if (background != NULL)
	{
		background->Draw(time, this, m_scene_index);
	}
}

void UIManager::Go(UIAction action)
{
	m_defered_go_actions.push_back(action);	
}

void UIManager::Perform_Go(UIAction action)
{
	// Need to force finish any active transitions
	// or our action is going to totally balls up.
	if (m_transition_in_progress == true)
	{
		Complete_Transition();
	}

	DBG_LOG("[UI Action] Performing action. Transition_In_Progress=%i Current_Scene=%s", m_transition_in_progress, Get_Scene(m_scene_index) == NULL ? "None" : Get_Scene(m_scene_index)->Get_Name());
	switch (action.Action)
	{
	case UIAction::Action::Pop:
		{
			if (action.Until_Scene_Name == NULL)
			{
				DBG_LOG("[UI Action] Poping UI Scene.");	
				action.Scene = m_scenes[m_scene_index - 1];
			}
			else
			{
				DBG_LOG("[UI Action] Poping UI Scenes until '%s'.", action.Until_Scene_Name);	
				
				int scene_index = m_scene_index;

				while (scene_index >= 0)
				{
					UIScene* scene = m_scenes[scene_index];
					if (stricmp(scene->Get_Name(), action.Until_Scene_Name) == 0)
					{
						action.Scene = scene;
						break;
					}
		
					scene_index--;
				}

				DBG_ASSERT_STR(action.Scene != NULL, "Failed to find scene '%s' in UI stack.", action.Until_Scene_Name);
			}
		}
		break;
	case UIAction::Action::Push:
		{
			DBG_LOG("[UI Action] Pushing UI Scene '%s'.", action.Scene->Get_Name());	
		}
		break;		
	case UIAction::Action::Replace:
		{
			DBG_LOG("[UI Action] Replacing UI Scene '%s'.", action.Scene->Get_Name());	
		}
		break;		
	}

	// Always force-enable navigation when changing screen so old screens cannot accidently leave navigation
	// disabled while its in the middle of something.
	Enable_Navigation();

	// Exit old scene.
	if (m_scene_index >= 0 && m_scenes[m_scene_index] != NULL)
	{
		DBG_LOG("Exiting UI Scene '%s'.", m_scenes[m_scene_index]->Get_Name());	
		m_scenes[m_scene_index]->Exit(this);

		Store_Focused_Element(m_scenes[m_scene_index], m_focused_element);
	}

	// Perform transition!
	if (action.Transition != NULL)
	{
		//DBG_ASSERT_STR(m_transition_in_progress == false, "Attempt to perform UI transition when one is already in progress.");

		m_transition_in_progress = true;
		m_transition_action = action;
		
		if (action.Scene != NULL && action.Scene->Is_Focusable())
		{
			Store_Stack();
			Apply_Action_To_Stack(action);

			Clear_Focus();
			Focus_Next(true);

			Restore_Stack();
		}
		else
		{
			Clear_Focus();
		}
	}
	else
	{
		Apply_Action_To_Stack(action);
		
		if (action.Scene->Is_Focusable())
		{
			Focus_Next();
		}
		else
		{
			Clear_Focus();		
		}
	}

	// Enter new sceen.
	if (action.Scene != NULL)
	{
		DBG_LOG("Entering UI Scene '%s'.", action.Scene->Get_Name());	
		action.Scene->Enter(this);

		Restore_Focused_Element(action.Scene);
	}
}

void UIManager::Store_Stack()
{	
	PROFILE_SCOPE("Store Stack");

	DBG_ASSERT_STR(m_stack_stored == false, "Attempt to store stack multiple times.");
	m_stack_stored = true;

	m_stored_scene_index = m_scene_index;

	for (int i = 0; i <= m_scene_index; i++)
	{
		m_stored_scenes[i] = m_scenes[i];
	}
}

void UIManager::Restore_Stack()
{
	PROFILE_SCOPE("Restore Stack");

	DBG_ASSERT_STR(m_stack_stored == true, "Attempt to restore stack without having a stored stack.");
	m_stack_stored = false;

	m_scene_index = m_stored_scene_index;

	for (int i = 0; i <= m_stored_scene_index; i++)
	{
		m_scenes[i] = m_stored_scenes[i];
	}
}

bool UIManager::Can_Accept_Invite()
{
	if (m_scene_index >= 0)
	{
		UIScene* curr_scene = m_scenes[m_scene_index];
		return curr_scene->Can_Accept_Invite();
	}
	return false;
}

void UIManager::Apply_Action_To_Stack(UIAction action)
{
	PROFILE_SCOPE("Apply Action To Stack");

	switch (action.Action)
	{
	case UIAction::Action::Pop:
		{
			if (action.Until_Scene_Name != NULL)
			{
				while (true)
				{
					UIScene* scene = m_scenes[m_scene_index];
					if (stricmp(scene->Get_Name(), action.Until_Scene_Name) == 0)
					{
						//m_scene_index++;
						break;
					}
					else
					{
						if (m_stack_stored == false)
						{
							//scene->Exit(this);
							DBG_LOG("[A] Deleting scene '%s'.", scene->Get_Name());
							Delete_Scene(scene);
						}
						m_scene_index--;
					}
				}
			}
			else
			{
				if (m_stack_stored == false)
				{
					//m_scenes[m_scene_index]->Exit(this);
					DBG_LOG("[B] Deleting scene '%s'.", m_scenes[m_scene_index]->Get_Name());
					Delete_Scene(m_scenes[m_scene_index]);
				}
				m_scene_index--;
			}
		}
		break;
	case UIAction::Action::Push:
		{
			m_scenes[++m_scene_index] = action.Scene;
			if (m_stack_stored == false)
			{
				//m_scenes[m_scene_index]->Enter(this);
			}
		}
		break;		
	case UIAction::Action::Replace:
		{			
			if (m_stack_stored == false)
			{
				//m_scenes[m_scene_index]->Exit(this);		
				DBG_LOG("[C] Deleting scene '%s' (replacing with %s).", m_scenes[m_scene_index]->Get_Name(), action.Scene->Get_Name());	
				Delete_Scene(m_scenes[m_scene_index]);
			}
			m_scenes[m_scene_index] = action.Scene;
			if (m_stack_stored == false)
			{
				//m_scenes[m_scene_index]->Enter(this);
			}
		}
		break;		
	}
}
