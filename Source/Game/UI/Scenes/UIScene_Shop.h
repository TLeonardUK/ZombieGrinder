// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_SHOP_
#define _GAME_UI_SCENES_UISCENE_SHOP_

#include "Engine/UI/UIScene.h"
#include "Engine/UI/Elements/UIGrid.h"

#include "Game/Profile/ItemManager.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Generic/Events/Event.h"
#include "Generic/Events/Delegate.h"

struct Profile;

class UIScene_Shop : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Shop, "UI");

private:
	Delegate<UIScene_Shop, UIGridDrawItemData>* m_draw_item_delegate;
	
	Profile* m_profile;

	std::vector<ItemArchetype*> m_item_list;
	std::vector<std::string> m_item_categories;

	std::vector<ItemArchetype*> m_category_items;

	ItemArchetype* m_purchase_item;

	float m_display_coins;
	float m_coin_sound_timer;

	float m_direction_change_timer;
	int m_direction;

	int m_last_selected_item_index;

	float m_info_alpha;

	float m_custom_color_delta;

	enum
	{
		DIRECTION_CHANGE_INTERVAL = 300,
		COIN_SOUND_INTERVAL = 50,
		INFO_FADE_IN_DELAY = 500
	};

protected:
	void Refresh_Item_List();
	void Category_Changed(const char* name);

public:
	UIScene_Shop(Profile* profile);
	~UIScene_Shop();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Is_Focusable();
	bool Should_Display_Cursor();
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
		
	void Recieve_Event(UIManager* manager, UIEvent e);
	
	void On_Draw_Item(UIGridDrawItemData* data);

};

#endif

