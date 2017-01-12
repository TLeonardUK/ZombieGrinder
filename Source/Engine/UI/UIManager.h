// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIMANAGER_
#define _ENGINE_UI_UIMANAGER_

#include "Engine/Engine/FrameTime.h"
#include "Engine/UI/UIEvent.h"

#include "Generic/Types/HashTable.h"
#include "Engine/Input/Input.h"

#include "Generic/Types/Vector2.h"

class UIScene;
class UITransition;
class UIElement;

class FontHandle;
class FontRenderer;
class AtlasHandle;
class AtlasRenderer;
struct AtlasFrame;
class Camera;
class SoundHandle;

#include <vector>

#define UI_MAX_SCENES 16

struct UICursorType
{
	enum Type
	{
		Pointer		= 0,
		SplitH		= 1,
		SplitV		= 2,
		HandPointer = 3,

		COUNT,
	};
};

struct UISoundType
{
	enum Type
	{
		Click			= 0,
		Character		= 1,
		Back			= 2,
		Close_Popup		= 3,
		Hover			= 4,
		Paint			= 5,
		Popup			= 6,
		Select			= 7,
		Enter_SubMenu	= 8,
		Exit_SubMenu	= 9,
		Small_Click		= 10,
		Pop				= 11,
		Thud			= 12,
		Coin_Decrease	= 13,
		Coin_Increase	= 14,

		COUNT,
	};
};

struct UIAction
{
	MEMORY_ALLOCATOR(UIAction, "UI");

private:
	friend class UIManager;

	struct Action
	{
		enum Type
		{
			Push,
			Pop,
			Replace
		};
	};

	Action::Type	Action;
	UIScene*		Scene;
	UITransition*	Transition;
	const char*		Until_Scene_Name;

public:
	static UIAction Push	(UIScene* scene, UITransition* transition = NULL)
	{
		UIAction action;
		action.Action		= Action::Push;
		action.Scene		= scene;
		action.Transition	= transition;
		return action;
	}

	static UIAction Pop		(UITransition* transition = NULL, const char* until_scene_name = NULL)
	{
		UIAction action;
		action.Action			= Action::Pop;
		action.Transition		= transition;
		action.Scene			= NULL;
		action.Until_Scene_Name	= until_scene_name;
		return action;
	}

	static UIAction Replace (UIScene* scene, UITransition* transition = NULL)
	{
		UIAction action;
		action.Action		= Action::Replace;
		action.Scene		= scene;
		action.Transition	= transition;
		return action;
	}
};

class UIManager
{
	MEMORY_ALLOCATOR(UIManager, "UI");

protected:
	std::vector<UIScene*>	m_to_delete;

	UIScene*				m_scenes[UI_MAX_SCENES];
	int						m_scene_index;

	UIScene*				m_stored_scenes[UI_MAX_SCENES];
	int						m_stored_scene_index;
	bool					m_stack_stored;

	float					m_scene_change_time;
	
	AtlasHandle*			m_ui_atlas;
	FontHandle*				m_ui_font;
	AtlasRenderer*			m_atlas_renderer;

	AtlasFrame*				m_cursor_frames[UICursorType::COUNT];
	UICursorType::Type		m_cursor_type;
	
	SoundHandle*			m_sounds[UISoundType::COUNT];

	bool					m_transition_in_progress;
	UIAction				m_transition_action;

	int						m_last_atlas_version;

	Camera*					m_camera;

	HashTable<UIScene*, unsigned int>	m_persistent_scenes;

	Vector2					m_selection_cursor_position;
	Vector2					m_selection_cursor_target_position;
	bool					m_cursor_offset_direction;
	float					m_cursor_offset_timer;

	Vector2					m_last_mouse_position;
	float					m_mouse_fade_timer;

	UIElement*				m_focused_element;

	bool					m_navigation_disabled;

	HashTable<int, int>		m_last_focused_element_index_per_scene;

	std::vector<UIAction>	m_defered_go_actions;

	UIElement*				m_mouse_selection_element;

	bool					m_hot_keys_enabled;
	bool					m_hotkey_used_this_frame;
	
	enum 
	{
		CURSOR_OFFSET_MAX		= -20,
		CURSOR_OFFSET_INTERVAL	= 700,

		MOUSE_FADE_DELAY		= 1000,
		MOUSE_FADE_INTERVAL		= 700
	};

	struct Queued_Event
	{
		UIScene* scene;
		UIEvent evt;
	};

	std::vector<Queued_Event> m_queued_events;

private:
	void Apply_Action_To_Stack(UIAction action);
	void Store_Stack();
	void Restore_Stack();

	void Store_Focused_Element(UIScene* scene, UIElement* element);
	void Restore_Focused_Element(UIScene* scene);

	void Perform_Go(UIAction action);

	void Tick_Always(const FrameTime& time);

	void Complete_Transition();

	void Delete_Scene(UIScene* scene);

public:
	template<typename T>
	T Get_Scene_By_Type()
	{
		for (int i = 0; i <= m_scene_index; i++)
		{
			T scene = dynamic_cast<T>(m_scenes[i]);
			if (scene != NULL)
			{
				return scene;
			}
		}
		return NULL;
	}

public:
	UIManager();
	~UIManager();

	int			Get_Top_Scene_Index();

	UIScene*	Get_Top_Focusable_Screen(const char* stopAtName = "");
	
	UIScene*	 Get_Scene(int offset = 0);
	UIScene*	 Get_Scene_From_Top(int offset = 0);
	UIScene*	 Get_Scene_Below(UIScene* scene);
	AtlasHandle* Get_Atlas();
	FontHandle*	 Get_Font();

	UIScene*	Get_Scene_By_Name(const char* name);

	bool Can_Accept_Invite();

	bool Pending_Actions();

	bool Transition_In_Progress();

	Camera*		 Get_Camera();
	void		 Set_Camera(Camera* camera);

	Vector2		 Get_UI_Scale();

	void		 Play_UI_Sound(UISoundType::Type type);

	bool		Get_Hot_Keys_Enabled(UIElement* element);
	bool		Is_Element_Focusable(UIElement* element);	

	bool Was_Pressed(OutputBindings::Type type);
	bool Was_Down(OutputBindings::Type type);
	bool Is_Down(OutputBindings::Type type);

	void Queue_Event(UIEvent e, UIScene* scene);

	UIElement*	Get_Focused_Element();

	void Set_Cursor(UICursorType::Type cursor);

	bool Check_Hotkey_Valid(OutputBindings::Type key);
	
	bool Mouse_Selection_Restricted();

	void Disable_Navigation();
	void Enable_Navigation();

	void Refresh();

	void Tick(const FrameTime& time);
	void Tick_Background(const FrameTime& time);
	void Update_Selection_Cursor(const FrameTime& time);

	bool Draw_Game_BG(const FrameTime& time);

	void Draw(const FrameTime& time);	
	void Draw_Stack(const FrameTime& time, bool draw_background = true);
	void Draw_Background(const FrameTime& time);
	void Draw_Selection_Cursor(const FrameTime& time, UIScene* curr_scene, UIScene* next_scene);

	void Clear_Focus();
	void Focus_Next(bool disable_audio = false);
	void Focus_Previous(bool disable_audio = false);
	void Focus(UIElement* element);

	void Limit_Mouse_Selection_To_Element(UIElement* element);

	void Go(UIAction action);

	void Store_Persistent_Scene(UIScene* scene);
	UIScene* Retrieve_Persistent_Scene(const char* name);

	UIElement* Find_Element_By_Position(Vector2 position, bool only_mouse_hot = false, bool focusable_only = false);

};

#endif

