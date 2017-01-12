// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_EQUIPMENT_
#define _GAME_UI_SCENES_UISCENE_EQUIPMENT_

#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIFrame.h"
#include "Generic/Types/Color.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

#include "Game/Profile/Profile.h"

struct Profile;
struct UIGridDrawItemData;
struct UIGridCanDragData;
struct Item;
class UICharacterPreviewRenderer;

class UIScene_Equipment : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Equipment, "UI");

private:
	Delegate<UIScene_Equipment, UIGridDrawItemData>* m_draw_item_delegate;
	Delegate<UIScene_Equipment, UIGridCanDragData>* m_can_drag_delegate;

	std::vector<ProfileItemStack> m_item_list;

	UIFrame m_background_frame;
	UIFrame m_active_background_frame;
	UIFrame m_equipped_background_frame;
	
	FontHandle*		m_font;
	FontRenderer	m_font_renderer;

	Profile* m_profile;

	float m_display_coins;
	float m_display_skills;

	float m_coin_sound_timer;
	float m_skill_sound_timer;

	int m_direction;

	UICharacterPreviewRenderer* m_char_renderer;

	enum
	{
		COIN_SOUND_INTERVAL = 50,
		INFO_FADE_IN_DELAY = 500
	};

	int m_last_selected_item_index;
	float m_info_alpha;

	Item* m_sell_item;
	Item* m_combine_item;

	Color m_slot_colors[4];

	Color m_custom_tint_color;
	Item* m_color_selector_item_a;
	Item* m_color_selector_item_b;
	bool m_color_selector_pending;

	float m_custom_color_delta;

protected:
	void Update_Info_Box(const FrameTime& time, UIManager* manager);
	void Update_Gold_Counters(const FrameTime& time, UIManager* manager);
	void Update_Item_Boxes(UIManager* manager);
	void Update_Equip_Items();

	void Perform_Combine(Item* item_a, Item* item_b);

public:
	void Refresh_Item_List();

public:
	UIScene_Equipment(Profile* profile);
	~UIScene_Equipment();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);
	
	void On_Can_Drag_Item(UIGridCanDragData* data);
	void On_Draw_Item(UIGridDrawItemData* data);
	void Draw_Slot(const FrameTime& time, const char* element_name, Item* item);
	void Draw_Item(Item* item, Vector2 position, Vector2 ui_scale, const FrameTime& time, bool multiple_in_stack, bool dragging);

};

#endif

