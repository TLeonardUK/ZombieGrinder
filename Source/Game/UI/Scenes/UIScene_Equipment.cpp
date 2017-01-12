// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Equipment.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Game/UI/Scenes/UIScene_InventoryDropDialog.h"
#include "Game/UI/Scenes/UIScene_PaintItemColorSelector.h"
#include "Game/UI/Scenes/UIScene_UnlockDialog.h"
#include "Game/UI/Scenes/UIScene_UpgradeWeapon.h"
#include "Game/UI/Scenes/Helper/UICharacterPreviewRenderer.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/UI/Elements/UIGrid.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UIDropDownMenu.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Online/OnlineInventory.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Generic/Math/Random.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"

#include "Generic/Helper/PersistentLogHelper.h"

//#define OPT_RANDOMISE_EQUIPMENT_CONTINUALLY

UIScene_Equipment::UIScene_Equipment(Profile* profile)
	: m_profile(profile)
	, m_coin_sound_timer(0.0f)
	, m_skill_sound_timer(0.0f)
	, m_direction(0)
	, m_last_selected_item_index(-1)
	, m_info_alpha(0.0f)
	, m_sell_item(NULL)
	, m_combine_item(NULL)
	, m_color_selector_pending(false)
	, m_custom_color_delta(0.0f)
{
	DBG_ASSERT(profile != NULL);
	DBG_LOG("Modifying equipment for: %s", profile->Name.c_str());

	Set_Layout("equipment");	

	m_slot_colors[0] = Color(200, 100, 100, 200);
	m_slot_colors[1] = Color(100, 200, 100, 200);
	m_slot_colors[2] = Color(100, 100, 200, 200);
	m_slot_colors[3] = Color(255, 255, 255, 200);

	m_display_coins = (float)profile->Coins;
	m_display_skills = (float)profile->Skill_Points;
	
	Refresh_Item_List();

	m_draw_item_delegate = new Delegate<UIScene_Equipment, UIGridDrawItemData>(this, &UIScene_Equipment::On_Draw_Item);
	m_can_drag_delegate = new Delegate<UIScene_Equipment, UIGridCanDragData>(this, &UIScene_Equipment::On_Can_Drag_Item);
	
	UIGrid* grid = Find_Element<UIGrid*>("grid_view");	
	grid->Set_Draw_Item_Background(false);
	grid->On_Draw_Item += m_draw_item_delegate;
	grid->On_Can_Drag += m_can_drag_delegate;

	// Grab all atlas frames.
	m_background_frame			= UIFrame("screen_main_box_active_#");
	m_active_background_frame	= UIFrame("screen_main_box_inactive_#");
	m_equipped_background_frame = UIFrame("screen_main_box_equipped_#");

	m_font = GameEngine::Get()->Get_UIManager()->Get_Font();
	m_font_renderer	= FontRenderer(m_font);

	m_char_renderer = new UICharacterPreviewRenderer();

#ifndef MASTER_BUILD

	/*
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_HMG"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_SMG5"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_SMG5SD"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Briefcase"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_DualPistols"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_PistolSD"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_SG12"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_EPR"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_PartyPopper"));
	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Shuriken"));
	*/

	for (int i = 0; i < 99 + 8; i++)
	{
	//	m_profile->Add_Item(ItemManager::Get()->Find_Archetype("Item_Spanner"));
	}

	/*
	// DBEUG DEBUG DEBUG DEBUG DEBUG DEBUG
	for (int i = 0; i < 70; i++)
	{
		m_profile->Level_Up();
	}
	// DBEUG DEBUG DEBUG DEBUG DEBUG DEBUG
	*/
#endif

	Update_Equip_Items();
}

UIScene_Equipment::~UIScene_Equipment()
{
	SAFE_DELETE(m_char_renderer);
	SAFE_DELETE(m_draw_item_delegate);
	SAFE_DELETE(m_can_drag_delegate);
}

const char* UIScene_Equipment::Get_Name()
{
	return "UIScene_Equipment";
}

bool UIScene_Equipment::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Equipment::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Equipment::Is_Focusable()
{
	return true;
}

void UIScene_Equipment::Refresh_Item_List()
{
	UIGrid* grid = Find_Element<UIGrid*>("grid_view");

	m_item_list = m_profile->Get_Item_Stacks();
	
	grid->Set_Total_Items(m_item_list.size());
}

void UIScene_Equipment::Enter(UIManager* manager)
{
	ProfileManager* profile_manager = ProfileManager::Get();

	FrameTime time;
	Update_Gold_Counters(time, manager);

	UILabel* text_box = Find_Element<UILabel*>("name_box");
	text_box->Set_Value(m_profile->Name);

	UILabel* level_label = Find_Element<UILabel*>("level_label");
	UILabel* skill_label = Find_Element<UILabel*>("skill_label");
	UILabel* gold_label = Find_Element<UILabel*>("gold_label");
	UIProgressBar* level_progress = Find_Element<UIProgressBar*>("level_progress");
	
	int xp_for_next_level = profile_manager->Get_Level_XP(m_profile->Level + 1);
	int xp = m_profile->Level_XP;
	float xp_progress = m_profile->Get_Level_Progress();

	level_label->Set_Value(SF("#menu_select_character_level", m_profile->Level));
	if (xp_for_next_level == 0)
	{
		level_progress->Set_Progress(1.0f);
		level_progress->Set_Value(S("#menu_inventory_max_level"));
	}
	else
	{
		level_progress->Set_Progress(xp_progress);
		level_progress->Set_Value(SF("#menu_inventory_xp",
			StringHelper::Format_Number((float)xp).c_str(),
			StringHelper::Format_Number((float)xp_for_next_level).c_str()
			));
	}

	skill_label->Set_Value(SF("#menu_inventory_skill_points", StringHelper::Format_Number((float)m_profile->Skill_Points).c_str()));

	if (m_profile->Coins >= m_profile->Wallet_Size)
	{
		gold_label->Set_Value(SF("#menu_inventory_gold_full", StringHelper::Format_Number((float)m_profile->Coins).c_str()));
	}
	else
	{
		gold_label->Set_Value(SF("#menu_inventory_gold", StringHelper::Format_Number((float)m_profile->Coins).c_str()));
	}
}	

void UIScene_Equipment::Exit(UIManager* manager)
{
}

void UIScene_Equipment::Update_Equip_Items()
{
//	UIManager* manager = GameEngine::Get()->Get_UIManager();
//	UIPanel* equip_slot_1 = Find_Element<UIPanel*>("equip_slot_1");
//	UIPanel* equip_slot_2 = Find_Element<UIPanel*>("equip_slot_2");
//	UIPanel* equip_slot_3 = Find_Element<UIPanel*>("equip_slot_3");
	
//	Item* skill_1 = m_profile->Get_Item_Slot(0);
//	Item* skill_2 = m_profile->Get_Item_Slot(1);
//	Item* skill_3 = m_profile->Get_Item_Slot(2);

}

void UIScene_Equipment::Update_Info_Box(const FrameTime& time, UIManager* manager)
{
	Vector2 ui_scale = manager->Get_UI_Scale();

	UIGrid* grid = Find_Element<UIGrid*>("grid_view");
	UIPanel* info_panel = Find_Element<UIPanel*>("info_panel");
	UILabel* info_name_label = Find_Element<UILabel*>("info_name_label");
	UILabel* info_price_label = Find_Element<UILabel*>("info_price_label");
	UILabel* info_description_label = Find_Element<UILabel*>("info_description_label");
	UILabel* info_durability_label = Find_Element<UILabel*>("info_durability_label");
	UIProgressBar* info_durability_bar = Find_Element<UIProgressBar*>("info_durability_bar");


	ProfileManager* profile_manager = ProfileManager::Get();
		
	int item_index = grid->Get_Selected_Index();

	// Show info dialog if we hover long enough.
	if (grid->Is_Focused(manager) && grid->Get_Selection_Time() >= INFO_FADE_IN_DELAY && item_index >= 0 && item_index < (int)m_item_list.size() && m_combine_item == NULL)
	{
		m_info_alpha = Math::Lerp(m_info_alpha, 1.0f, 0.05f);
	}
	else
	{
		m_info_alpha = 0.0f;
	}

	// Position info next to the info box.
	float screen_width  = (float)GfxDisplay::Get()->Get_Width();
	float screen_height = (float)GfxDisplay::Get()->Get_Height();
		
	Rect2D item_box = grid->Get_Selected_Item_Box();
	Rect2D screen_box = grid->Get_Screen_Box();
	Rect2D info_box = info_panel->Get_Screen_Box();

	bool left = (item_box.X + (item_box.Width * 0.5f)) <= (screen_box.X + (screen_box.Width * 0.5f));
	bool top  = (item_box.Y + (item_box.Height * 0.5f)) <= (screen_box.Y + (screen_box.Height * 0.5f));

	float spacing = 5.0f * ui_scale.Y;

	// Top-Left
	if (left == true && top == true)
	{
	//	info_panel->Reposition(Rect2D(item_box.X, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height));
		info_panel->Reposition(Rect2D(screen_box.X, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height));
	}
	// Top-Right
	else if (left == false && top == true)
	{
//		info_panel->Reposition(Rect2D(item_box.X + item_box.Width - info_box.Width, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height));
		info_panel->Reposition(Rect2D(screen_box.X, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height));
	}
	// Bottom-Left
	else if (left == true && top == false)
	{
//		info_panel->Reposition(Rect2D(item_box.X, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height));
		info_panel->Reposition(Rect2D(screen_box.X, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height));
	}
	// Bottom-Right
	else if (left == false && top == false)
	{
//		info_panel->Reposition(Rect2D(item_box.X + item_box.Width - info_box.Width, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height));
		info_panel->Reposition(Rect2D(screen_box.X, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height));
	}

	float durability_alpha = 0.0f;

	// Show info.
	if (item_index >= 0 && item_index < (int)m_item_list.size())// && m_last_selected_item_index != item_index)
	{
		ProfileItemStack& item = m_item_list.at(item_index);

		std::string name = item.Items[0]->Get_Display_Name();
		if (item.Archetype->item_slot != ItemSlot::All || (item.Archetype->combine_group == "Paints" && item.Items[0]->indestructable))
		{
			if (!item.Items[0]->indestructable)
			{
				if (item.Archetype->max_durability >= 0.0f)
				{
					durability_alpha = 1.0f;
					info_durability_bar->Set_Progress(1.0f - (item.Items[0]->equip_duration / item.Archetype->max_durability));
				}
			}
		}

		info_name_label->Set_Value(name);

		if (item.Items[0]->Is_Broken())
		{
			info_price_label->Set_Value(S("#menu_inventory_broken"));
		}
		else if (item.Archetype->is_sellable == false)
		{
			info_price_label->Set_Value("");
		}
		else
		{
			float multiplier = m_profile->Get_Stat_Multiplier(Profile_Stat::Sell_Price);
			if (item.Items[0]->was_inventory_drop)
			{
				multiplier *= INVENTORY_ITEM_SELL_PRICE_MULTIPLIER;
			}

			info_price_label->Set_Value(SF("#menu_inventory_gold", StringHelper::Format_Number((float)(item.Archetype->cost * multiplier)).c_str()));
		}

		std::string description = item.Archetype->Get_Description(item.Items[0]->indestructable);

		ItemUpgradeTree* tree = item.Archetype->Get_Upgrade_Tree();
		if (tree)
		{
			std::vector<ItemUpgradeTreeNodeModifier> modifiers;
			std::vector<int> upgrade_ids;

			item.Items[0]->Get_Upgrade_Ids(upgrade_ids);
			tree->Get_Active_Compound_Modifiers(modifiers, upgrade_ids);

			if (modifiers.size() > 0 || item.Items[0]->indestructable)
			{
				description += "\n\n";
			}
			for (std::vector<ItemUpgradeTreeNodeModifier>::iterator iter = modifiers.begin(); iter != modifiers.end(); iter++)
			{
				ItemUpgradeTreeNodeModifier& mod = *iter;
				description += mod.To_Description_String() + "\n";
			}

			if (item.Items[0]->indestructable)
			{
				description += S("#weapon_upgrade_mod_infinite_durability_description");
				description += "\n";
			}
		}

		if (item.Items[0]->was_inventory_drop)
		{
			description += "\n\n";
			description += UniqueItemStats(item.Items[0]).ToString();
		}
		info_description_label->Set_Value(description);
	}

	info_panel->Set_Frame_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_name_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_price_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_description_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_durability_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha * durability_alpha));
	info_durability_bar->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha * durability_alpha));

	m_last_selected_item_index = item_index;
}

void UIScene_Equipment::Update_Gold_Counters(const FrameTime& time, UIManager* manager)
{
	UILabel* skill_label = Find_Element<UILabel*>("skill_label");
	UILabel* gold_label = Find_Element<UILabel*>("gold_label");

	// Lerp the display coins if we have bought/sold anything.	
	if (abs(m_display_coins - m_profile->Coins) >= 1.0f)
	{
		m_display_coins = Math::Lerp((float)m_display_coins, (float)m_profile->Coins, 0.1f);

		m_coin_sound_timer += time.Get_Frame_Time();
		if (m_coin_sound_timer >= COIN_SOUND_INTERVAL)
		{
			manager->Play_UI_Sound(UISoundType::Coin_Increase);
			m_coin_sound_timer = 0.0f;
		}
	}
	else
	{
		m_display_coins = (float)m_profile->Coins;
	}
	
	// Lerp the display skills if we have bought/sold anything.	
	if (abs(m_display_skills - m_profile->Skill_Points) >= 1.0f)
	{
		m_display_skills = Math::Lerp((float)m_display_skills, (float)m_profile->Skill_Points, 0.1f);

		m_skill_sound_timer += time.Get_Frame_Time();
		if (m_skill_sound_timer >= COIN_SOUND_INTERVAL)
		{
			manager->Play_UI_Sound(UISoundType::Coin_Increase);
			m_skill_sound_timer = 0.0f;
		}
	}
	else
	{
		m_display_skills = (float)m_profile->Skill_Points;
	}


	if (m_display_coins>= m_profile->Wallet_Size)
	{
		gold_label->Set_Value(SF("#menu_inventory_gold_full", StringHelper::Format_Number((float)ceilf(m_display_coins)).c_str()));
	}
	else
	{
		gold_label->Set_Value(SF("#menu_inventory_gold", StringHelper::Format_Number((float)ceilf(m_display_coins)).c_str()));
	}

	skill_label->Set_Value(SF("#menu_inventory_skill_points", StringHelper::Format_Number((float)ceilf((float)m_profile->Skill_Points)).c_str()));

	Update_Item_Boxes(manager);
}

void UIScene_Equipment::Update_Item_Boxes(UIManager* manager)
{
	UIPanel* equip_slot_1 = Find_Element<UIPanel*>("equip_slot_1");
	UIPanel* equip_slot_2 = Find_Element<UIPanel*>("equip_slot_2");
	UIPanel* equip_slot_3 = Find_Element<UIPanel*>("equip_slot_3");

	equip_slot_1->Set_Frame_Color(m_slot_colors[0]);
	equip_slot_2->Set_Frame_Color(m_slot_colors[1]);
	equip_slot_3->Set_Frame_Color(m_slot_colors[2]);
}

void UIScene_Equipment::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Update_Gold_Counters(time, manager);
	Update_Info_Box(time, manager);
	
	Find_Element<UIButton*>("back_button")->Set_Value(m_combine_item != NULL ? "#menu_cancel_combine" : "#menu_back");

	m_direction = m_char_renderer->Get_Direction();

#ifdef OPT_RANDOMISE_EQUIPMENT_CONTINUALLY
	static int old_direction = 0;
	if (m_direction != old_direction)
	{
		std::vector<ItemArchetype*> archetypes = ItemManager::Get()->Get_Archetypes();

		// Remove all items.
		for (int i = 0; i < (int)m_profile->Get_Items().size(); i++)
		{
			m_profile->Remove_Item(m_profile->Get_Items()[i]);
		}

		// Figure out some heads to add.
		std::vector<ItemArchetype*> heads;
		std::vector<ItemArchetype*> accessories;
		std::vector<ItemArchetype*> weapons;
		std::vector<ItemArchetype*> paints;
		for (std::vector<ItemArchetype*>::iterator iter = archetypes.begin(); iter != archetypes.end(); iter++)
		{
			ItemArchetype* type = *iter;
			if (type->item_slot == ItemSlot::Head)
			{
				heads.push_back(type);
			}
			if (type->item_slot == ItemSlot::Accessory)
			{
				accessories.push_back(type);
			}
			if (type->item_slot == ItemSlot::Weapon)
			{
				weapons.push_back(type);
			}
			if (type->item_slot == ItemSlot::All && type->combine_group == "Paints")
			{
				paints.push_back(type);
			}
		}

		// Add random items.
		int head_count = Random::Static_Next(1, 3);
		int head_equipped = Random::Static_Next(0, head_count - 1);
		for (int i = 0; i < head_count; i++)
		{
			ItemArchetype* head  = heads[Random::Static_Next(0, heads.size() - 1)];
			ItemArchetype* paint = paints[Random::Static_Next(0, paints.size() - 1)];
				
			Item* head_item = m_profile->Add_Item(head);
			Item* paint_item = m_profile->Add_Item(paint);

			if (m_profile->Can_Combine_Item(head_item, paint_item))
			{
				m_profile->Combine_Item(head_item, paint_item);
			}

			if (i == head_equipped)
			{
				m_profile->Equip_Item(head_item);
			}
		}
		
		int accessory_count = Random::Static_Next(1, 3);
		int accessory_equipped = Random::Static_Next(0, accessory_count - 1);
		for (int i = 0; i < accessory_count; i++)
		{
			ItemArchetype* accessory = accessories[Random::Static_Next(0, accessories.size() - 1)];
			ItemArchetype* paint = paints[Random::Static_Next(0, paints.size() - 1)];

			Item* accessory_item = m_profile->Add_Item(accessory);
			Item* paint_item = m_profile->Add_Item(paint);

			if (m_profile->Can_Combine_Item(accessory_item, paint_item))
			{
				m_profile->Combine_Item(accessory_item, paint_item);
			}

			if (i == accessory_equipped)
			{
				m_profile->Equip_Item(accessory_item);
			}
		}

		int weapon_count = Random::Static_Next(1, 3);
		int weapon_equipped = Random::Static_Next(0, weapon_count - 1);
		for (int i = 0; i < accessory_count; i++)
		{
			ItemArchetype* weapon = weapons[Random::Static_Next(0, weapons.size() - 1)];
			ItemArchetype* paint = paints[Random::Static_Next(0, paints.size() - 1)];

			Item* weapon_item = m_profile->Add_Item(weapon);
			Item* paint_item = m_profile->Add_Item(paint);

			if (m_profile->Can_Combine_Item(weapon_item, paint_item))
			{
				m_profile->Combine_Item(weapon_item, paint_item);
			}

			if (i == weapon_equipped)
			{
				m_profile->Equip_Item(weapon_item);
			}
		}
		
		old_direction = m_direction;
		Refresh_Item_List();
	}
#endif

	/*
	if (manager->Get_Scene_By_Type<UIScene_InventoryDropDialog*>() == NULL)
	{
		OnlineInventoryItem item;
		if (m_profile->Unpack_Events.size() > 0)
		{
			ProfileUnpackEvent evt = m_profile->Unpack_Events.back();
			m_profile->Unpack_Events.pop_back();

			manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
			manager->Go(UIAction::Push(new UIScene_InventoryDropDialog(evt), new UIFadeInTransition()));

			return;
		}
	}
	*/

	if (manager->Get_Scene_By_Type<UIScene_UnlockDialog*>() == NULL)
	{
		OnlineInventoryItem item;
		if (m_profile->Unpack_Events.size() > 0)
		{
			std::vector<UnlockDialogEntry> entries;

			for (unsigned int i = 0; i < m_profile->Unpack_Events.size(); i++)
			{
				ProfileUnpackEvent evt = m_profile->Unpack_Events.back();
				m_profile->Unpack_Events.pop_back();

				UnlockDialogEntry entry;
				entry.Type = UnlockDialogEntryType::Item_Unpacked;
				entry.Unpack_Event = evt;
				entry.User_Profile = m_profile;
				entries.push_back(entry);
			}

			manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
			manager->Go(UIAction::Push(new UIScene_UnlockDialog(entries), new UIFadeInTransition()));

			return;
		}
	}

	if (Input::Get()->Was_Pressed(InputBindings::Mouse_Left))
	{
		UIGrid* grid = Find_Element<UIGrid*>("grid_view");

		Vector2 pos = Input::Get()->Get_Mouse_State()->Get_Position();

		UIPanel* equip_slot_1 = Find_Element<UIPanel*>("equip_slot_1");
		UIPanel* equip_slot_2 = Find_Element<UIPanel*>("equip_slot_2");
		UIPanel* equip_slot_3 = Find_Element<UIPanel*>("equip_slot_3");

		Item* target_item = NULL;

		if (equip_slot_1->Get_Screen_Box().Intersects(pos))
		{
			target_item = m_profile->Get_Item_Slot(0);
		}
		else if (equip_slot_2->Get_Screen_Box().Intersects(pos))
		{
			target_item = m_profile->Get_Item_Slot(1);
		}
		else if (equip_slot_3->Get_Screen_Box().Intersects(pos))
		{
			target_item = m_profile->Get_Item_Slot(2);
		}

		if (target_item != NULL)
		{
			int item_index = 0;
			for (std::vector<ProfileItemStack>::iterator iter = m_item_list.begin(); iter != m_item_list.end() && target_item != NULL; iter++, item_index++)
			{
				ProfileItemStack& item = *iter;
				for (u32 i = 0; i < item.Items.size() && target_item != NULL; i++)
				{
					if (target_item == item.Items[i])
					{
						grid->Drag_Item_Index(item_index);
						m_combine_item = target_item;
						target_item = NULL;
						break;
					}
				}
			}
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Equipment::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);

	// Draw icon overlays.
	Draw_Slot(time, "equip_slot_1", m_profile->Get_Item_Slot(0));
	Draw_Slot(time, "equip_slot_2", m_profile->Get_Item_Slot(1));
	Draw_Slot(time, "equip_slot_3", m_profile->Get_Item_Slot(2));
	
	// Draw character.
	UIPanel* info_panel = Find_Element<UIPanel*>("description_label");
	m_char_renderer->Draw(time, manager, m_profile, info_panel->Get_Screen_Box());
}

void UIScene_Equipment::Draw_Slot(const FrameTime& time, const char* element_name, Item* item)
{
	if (item == NULL)
	{
		return;
	}
	
	Vector2 ui_scale = GameEngine::Get()->Get_UIManager()->Get_UI_Scale();
	Vector2 original_ui_scale = ui_scale;
	UIPanel* panel = Find_Element<UIPanel*>(element_name);
	Rect2D panel_bounds = panel->Get_Screen_Box();
	
	ui_scale.X *= 0.60f;
	ui_scale.Y *= 0.60f;

	Vector2 position = Vector2
	(
		panel_bounds.X + (panel_bounds.Width * 0.5f) - (item->archetype->icon_offset.X * ui_scale.X),
		panel_bounds.Y + (panel_bounds.Height * 0.5f) - (item->archetype->icon_offset.Y * ui_scale.Y)
	);

	Draw_Item(item, position, ui_scale, time, false, false);

	// Draw attached icons.
	std::vector<Item*> attached_items = m_profile->Get_Attached_Items(item);
	for (std::vector<Item*>::iterator iter = attached_items.begin(); iter != attached_items.end(); iter++)
	{
		Item* attached_item = *iter;	
		Vector2 attached_position = Vector2
		(
			panel_bounds.X + (panel_bounds.Width * 0.525f),
			panel_bounds.Y + (panel_bounds.Height * 0.525f)
		);
	
		Draw_Item(attached_item, attached_position, ui_scale * 0.5f, time, false, false);	
	}
}

void UIScene_Equipment::Draw_Item(Item* item, Vector2 position, Vector2 ui_scale, const FrameTime& time, bool multiple_in_stack, bool dragging)
{
	AtlasRenderer atlas_renderer(NULL);

	Color tint_color = item->primary_color;
	Color base_color = Color::White;

	if (item->archetype->use_custom_color)
	{
		m_custom_color_delta += time.Get_Delta_Seconds();
		int ms = (int)(m_custom_color_delta * 10000);

		int r = (ms / 100);
		int g = (ms / 33);
		int b = (ms / 70);

		// Make RGB do a ping-pong effect to stop sudden color changes when the values wrap.
		int dir = (r / 255) % 2;
		tint_color.R = dir ? (r % 255) : (255 - (r % 255));

		dir = (g / 255) % 2;
		tint_color.G = dir ? (g % 255) : (255 - (g % 255));
		
		dir = (b / 255) % 2;
		tint_color.B = dir ? (b % 255) : (255 - (b % 255));
	
		tint_color.A = 255;
	}

	bool grey_out = item->Is_Broken();

	if (m_combine_item != NULL && dragging == false)
	{
		if ((m_combine_item == item && multiple_in_stack == false) || !m_profile->Can_Combine_Item(m_combine_item, item))
		{
			grey_out = true;
		}
	}

	if (grey_out)
	{
		base_color.R = (u8)(base_color.R * 0.4f);
		base_color.G = (u8)(base_color.G * 0.4f);
		base_color.B = (u8)(base_color.B * 0.4f);
		base_color.A = (u8)(base_color.A * 0.4f);

		tint_color.R = (u8)(tint_color.R * 0.4f);
		tint_color.G = (u8)(tint_color.G * 0.4f);
		tint_color.B = (u8)(tint_color.B * 0.4f);
		tint_color.A = (u8)(tint_color.A * 0.4f);
	}

	if (item->archetype->is_icon_direction_based && item->archetype->item_slot != 1)
	{
		if (item->archetype->is_tintable)
		{
			atlas_renderer.Draw_Frame(item->archetype->icon_animations[m_direction]->Frames[0], position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);	
			atlas_renderer.Draw_Frame(item->archetype->icon_tint_animations[m_direction]->Frames[0], position, 0.0f, tint_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);	
		}
		else
		{
			atlas_renderer.Draw_Frame(item->archetype->icon_animations[m_direction]->Frames[0], position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);	
		}
	}
	else
	{
		if (item->archetype->is_tintable)
		{
			atlas_renderer.Draw_Frame(item->archetype->icon_animations[0]->Frames[0], position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);	
			atlas_renderer.Draw_Frame(item->archetype->icon_tint_animations[0]->Frames[0], position, 0.0f, tint_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);			
		}
		else
		{
			atlas_renderer.Draw_Frame(item->archetype->icon_animations[0]->Frames[0], position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);	
		}
	}
}

void UIScene_Equipment::On_Can_Drag_Item(UIGridCanDragData* data)
{
	if (m_combine_item != NULL)
	{
		data->allow_drag = false;
		return;
	}

	data->allow_drag = true;
}

void UIScene_Equipment::On_Draw_Item(UIGridDrawItemData* data)
{
	Vector2 ui_scale = data->ui_scale;
	ProfileItemStack& stack = m_item_list.at(data->item_index);

	AtlasRenderer renderer;

	Item* item = stack.Items[0];
	bool multiple_in_stack = stack.Items.size() > 1;

	Color tint_color = item->primary_color;
	Color base_color = Color::White;

	bool grey_out = item->Is_Broken();

	if (m_combine_item != NULL && data->dragging == false)
	{
		if ((m_combine_item == item && multiple_in_stack == false) || !m_profile->Can_Combine_Item(m_combine_item, item))
		{
			grey_out = true;
		}
	}

	if (grey_out)
	{
		base_color.R = (u8)(base_color.R * 0.4f);
		base_color.G = (u8)(base_color.G * 0.4f);
		base_color.B = (u8)(base_color.B * 0.4f);
		base_color.A = (u8)(base_color.A * 0.4f);

		tint_color.R = (u8)(tint_color.R * 0.4f);
		tint_color.G = (u8)(tint_color.G * 0.4f);
		tint_color.B = (u8)(tint_color.B * 0.4f);
		tint_color.A = (u8)(tint_color.A * 0.4f);
	}

	if (stack.Items[0]->equip_slot != -1)
	{
		m_equipped_background_frame.Draw_Frame(renderer, data->item_bounds, data->ui_scale, m_slot_colors[stack.Archetype->item_slot]);
	}
	else
	{
		if (data->selected)
		{
			m_active_background_frame.Draw_Frame(renderer, data->item_bounds, data->ui_scale, m_slot_colors[stack.Archetype->item_slot]);
		}
		else
		{
			m_background_frame.Draw_Frame(renderer, data->item_bounds, data->ui_scale, m_slot_colors[stack.Archetype->item_slot]);
		}
	}
	
	ui_scale.X *= 0.75f;
	ui_scale.Y *= 0.75f;

	Vector2 position = Vector2
	(
		data->item_bounds.X + (data->item_bounds.Width * 0.5f) - (stack.Archetype->icon_offset.X * ui_scale.X),
		data->item_bounds.Y + (data->item_bounds.Height * 0.5f) - (stack.Archetype->icon_offset.Y * ui_scale.Y)
	);
	
	Draw_Item(stack.Items[0], position, ui_scale, data->time, stack.Items.size() > 1, data->dragging);

	if (stack.Items.size() > 1)
	{
		m_font_renderer.Draw_String(StringHelper::Format("x%i", stack.Items.size()).c_str(), Rect2D(data->item_bounds.X, data->item_bounds.Y + (data->item_bounds.Height * 0.5f), data->item_bounds.Width * 0.9f, data->item_bounds.Height * 0.4f), 16.0f, base_color, TextAlignment::Right, TextAlignment::Bottom);
	}

	if (m_combine_item == stack.Items[0] && data->dragging == false)
	{
		PrimitiveRenderer pr;
		pr.Draw_Wireframe_Quad(data->item_bounds, 0.0f, Color::White, 2.0f * ui_scale.Y);
	}

	// Draw attached icons.
	std::vector<Item*> attached_items = m_profile->Get_Attached_Items(stack.Items[0]);
	for (std::vector<Item*>::iterator iter = attached_items.begin(); iter != attached_items.end(); iter++)
	{
		Item* attached_item = *iter;	
		
		Vector2 attached_position = Vector2
		(
			data->item_bounds.X + (data->item_bounds.Width * 0.525f),
			data->item_bounds.Y + (data->item_bounds.Height * 0.525f)
		);
	
		Draw_Item(attached_item, attached_position, data->ui_scale * 0.35f, data->time, false, data->dragging);	
	}

	if (stack.Items[0]->Is_Broken())
	{
		Vector2 broken_position = Vector2
		(
			data->item_bounds.X + (data->item_bounds.Width * 0.5f),
			data->item_bounds.Y + (data->item_bounds.Height * 0.5f)
		);
		renderer.Draw_Frame("screen_broken_item", broken_position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
	}

	// Draw upgrade information.
	if (item->archetype->Get_Upgrade_Tree() != NULL)
	{
		std::vector<int> upgrade_ids;
		std::vector<ItemUpgradeTreeNode*> nodes;
		item->Get_Upgrade_Ids(upgrade_ids);
		item->archetype->Get_Upgrade_Tree()->Get_Nodes(nodes);

		if (upgrade_ids.size() < nodes.size() - 1) // -1 for root node.
		{
			AtlasRenderer atlas_renderer;

			AtlasFrame* upgrade_frame = ResourceFactory::Get()->Get_Atlas_Frame("screen_upgrade_arrow");
			float upgrade_padding = data->item_bounds.Width * 0.1f;

			float cycle_length = 2500.0f;
			float offset_max = data->item_bounds.Height * 0.1f;

			float time = (float)Platform::Get()->Get_Ticks();
			float anim_delta = Min(1.0f, fmod(time, cycle_length) / (cycle_length * 0.75f));

			float offset_y = offset_max * Math::SmootherStep(0.0f, 1.0f, anim_delta);
			float alpha_value = Math::FadeInFadeOut(0.0f, 1.0f, anim_delta, 0.3f);

			Vector2 upgrade_scale = ui_scale * 0.5f;

			Vector2 upgrade_position(
				data->item_bounds.X + data->item_bounds.Width - ((upgrade_frame->Rect.Width * upgrade_scale.X) + upgrade_padding),
				data->item_bounds.Y + data->item_bounds.Height - ((upgrade_frame->Rect.Height * upgrade_scale.Y) + upgrade_padding) - offset_y
			);
			Color upgrade_color(255.0f, 255.0f, 255.0f, 255.0f * alpha_value);

			atlas_renderer.Draw_Frame(upgrade_frame, upgrade_position, 0.0f, upgrade_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, upgrade_scale);
		}
	}
}

void UIScene_Equipment::Recieve_Event(UIManager* manager, UIEvent e)
{
	Vector2 ui_scale = manager->Get_UI_Scale();

	switch (e.Type)
	{
	case UIEventType::Grid_Begin_Drag:
		{
			UIGrid* grid = dynamic_cast<UIGrid*>(e.Source);
			int index = grid->Get_Drag_Selected_Index();
			if (index >= 0 && index < (int)m_item_list.size())
			{
				ProfileItemStack& stack = m_item_list.at(index);
				m_combine_item = stack.Items[0];
			}
			break;
		}
	case UIEventType::Grid_Finish_Drag:
		{
			UIGrid* grid = dynamic_cast<UIGrid*>(e.Source);
			int index = grid->Get_Drag_Selected_Index();
			int drop_index = grid->Get_Drop_Selected_Index();

			UIPanel* equip_panel_1 = Find_Element<UIPanel*>("equip_slot_1");
			UIPanel* equip_panel_2 = Find_Element<UIPanel*>("equip_slot_2");
			UIPanel* equip_panel_3 = Find_Element<UIPanel*>("equip_slot_3");
			Vector2 pos = Input::Get()->Get_Mouse_State()->Get_Position();

			// Got item.
			if (index >= 0 && index < (int)m_item_list.size())
			{
				ProfileItemStack& item_stack = m_item_list.at(index);
				Item* item = item_stack.Items[0];

				// Dropping on other item?
				if (drop_index >= 0 && drop_index < (int)m_item_list.size())
				{
					ProfileItemStack& other_stack = m_item_list.at(drop_index);
					Item* other_item = other_stack.Items[0];

					if (item == other_item)
					{
						if (other_stack.Items.size() > 1)
						{
							other_item = other_stack.Items[1];
						}
						else
						{
							other_item = NULL;
							manager->Play_UI_Sound(UISoundType::Back);
						}
					}

					if (other_item != NULL)
					{
						if (!OnlineInventory::Get()->CanAccess() && (item->was_inventory_drop || other_item->was_inventory_drop))
						{
							manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_need_inventory_connection"), S("#menu_continue")), new UIFadeInTransition()));
						}
						else
						{
							if (m_profile->Can_Combine_Item(item, other_item))
							{
								Perform_Combine(item, other_item);
							}
							else
							{
								manager->Play_UI_Sound(UISoundType::Back);
							}
						}
					}
				}

				// Dropping on equip slot.
				if (equip_panel_1->Get_Screen_Box().Intersects(pos))
				{
					if (item->archetype->item_slot == 0)
					{
						if (item->Is_Broken())
						{
							manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_cannot_equip_broken"), S("#menu_continue")), new UIFadeInTransition()));
						}
						else
						{
							m_profile->Equip_Item(item);
							manager->Play_UI_Sound(UISoundType::Paint);
						}
					}
					else
					{	
						Item* other_item = m_profile->Get_Item_Slot(0);
						if (other_item != NULL && m_profile->Can_Combine_Item(item, other_item))
						{
							if (!OnlineInventory::Get()->CanAccess() && (item->was_inventory_drop || other_item->was_inventory_drop))
							{
								manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_need_inventory_connection"), S("#menu_continue")), new UIFadeInTransition()));
							}
							else
							{
								Perform_Combine(item, other_item);
							}
						}
					}
				}
				else if (equip_panel_2->Get_Screen_Box().Intersects(pos))
				{
					if (item->archetype->item_slot == 1)
					{
						if (item->Is_Broken())
						{
							manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_cannot_equip_broken"), S("#menu_continue")), new UIFadeInTransition()));
						}
						else
						{
							m_profile->Equip_Item(item);
							manager->Play_UI_Sound(UISoundType::Paint);
						}
					}
					else
					{	
						Item* other_item = m_profile->Get_Item_Slot(1);
						if (other_item != NULL && m_profile->Can_Combine_Item(item, other_item))
						{
							if (!OnlineInventory::Get()->CanAccess() && (item->was_inventory_drop || other_item->was_inventory_drop))
							{
								manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_need_inventory_connection"), S("#menu_continue")), new UIFadeInTransition()));
							}
							else
							{
								Perform_Combine(item, other_item);
							}
						}
					}
				}
				else if (equip_panel_3->Get_Screen_Box().Intersects(pos))
				{
					if (item->archetype->item_slot == 2)
					{
						if (item->Is_Broken())
						{
							manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_cannot_equip_broken"), S("#menu_continue")), new UIFadeInTransition()));
						}
						else
						{
							m_profile->Equip_Item(item);
							manager->Play_UI_Sound(UISoundType::Paint);
						}
					}
					else
					{	
						Item* other_item = m_profile->Get_Item_Slot(2);
						if (other_item != NULL && m_profile->Can_Combine_Item(item, other_item))
						{
							if (!OnlineInventory::Get()->CanAccess() && (item->was_inventory_drop || other_item->was_inventory_drop))
							{
								manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_need_inventory_connection"), S("#menu_continue")), new UIFadeInTransition()));
							}
							else
							{
								Perform_Combine(item, other_item);
							}
						}
					}
				}

				// If already equipped, then unequip.
				else
				{
					if (item->equip_slot != -1)
					{
						m_profile->Unequip_Item(item);
						m_profile->Ensure_Valid_Equips(item);
					}
				}
			}

			m_combine_item = NULL;
			break;
		}
	case UIEventType::Grid_Item_Click:
		{			
			UIGrid* grid = dynamic_cast<UIGrid*>(e.Source);
			int index = grid->Get_Selected_Index();
			if (index >= 0 && index < (int)m_item_list.size())
			{
				ProfileItemStack& stack = m_item_list.at(index);
				
				if (m_combine_item != NULL)
				{
					Item* item_b = stack.Items.at(0);

					if (m_combine_item == item_b)
					{
						if (stack.Items.size() > 1)
						{
							item_b = stack.Items[1];
						}
						else
						{
							item_b = NULL;
						}
					}

					if (item_b != NULL && m_profile->Can_Combine_Item(m_combine_item, item_b))
					{
						if (!OnlineInventory::Get()->CanAccess() && (m_combine_item->was_inventory_drop || item_b->was_inventory_drop))
						{
							manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_need_inventory_connection"), S("#menu_continue")), new UIFadeInTransition()));
						}
						else
						{
							Perform_Combine(m_combine_item, item_b);
							m_combine_item = NULL;
							//m_profile->Combine_Item(m_combine_item, item_b);
							//Refresh_Item_List();
							//manager->Play_UI_Sound(UISoundType::Select);
						}
					}
					else
					{
						manager->Play_UI_Sound(UISoundType::Back);
					}
				}
				else
				{
					UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("item_option_menu");

					Rect2D selection_box = grid->Get_Selected_Item_Box();

					int head_count = 0;
					for (std::vector<ProfileItemStack>::iterator iter = m_item_list.begin(); iter != m_item_list.end(); iter++)
					{
						ProfileItemStack& s = *iter;
						if (s.Archetype->item_slot == ItemSlot::Head && s.Items[0]->was_inventory_drop == false)
						{
							head_count += s.Items.size();
						}
					}

					Find_Element<UIButton*>("equip_button")->Set_Value(stack.Archetype->is_unpackable ? "#menu_unpack" : (stack.Items[0]->equip_slot != -1 ? "#menu_unequip" : "#menu_equip"));
					Find_Element<UIButton*>("equip_button")->Set_Enabled((stack.Archetype->is_unpackable != 0 || stack.Archetype->is_equippable != 0) && !stack.Items[0]->Is_Broken() && (stack.Items[0]->equip_slot != ItemSlot::Head || head_count > 1));

					Find_Element<UIButton*>("sell_button")->Set_Enabled(stack.Archetype->is_sellable != 0);
					Find_Element<UIButton*>("combine_button")->Set_Enabled(stack.Archetype->is_combinable != 0);
					Find_Element<UIButton*>("upgrade_button")->Set_Enabled(stack.Archetype->upgrade_tree.Get().Get() != NULL);

					drop_down_menu->Show(Rect2D(selection_box.X, (selection_box.Y + selection_box.Height) - (5 * ui_scale.Y), selection_box.Width, drop_down_menu->Get_Screen_Box().Height));
				}
			}
			else
			{
				manager->Play_UI_Sound(UISoundType::Back);
			}
		}
		break;
	case UIEventType::Dialog_Close:
		{
			if (m_color_selector_pending)
			{
				m_color_selector_pending = false;

				if (dynamic_cast<UIScene_PaintItemColorSelector*>(e.Scene)->Get_Selected_Index() == 1)
				{
					m_profile->Combine_Item(m_color_selector_item_a, m_color_selector_item_b, m_custom_tint_color);
					manager->Play_UI_Sound(UISoundType::Paint);
					Refresh_Item_List();
				}
			}
			else if (m_sell_item != NULL)
			{
				if (!OnlineInventory::Get()->CanAccess() && m_sell_item->was_inventory_drop)
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_need_inventory_connection"), S("#menu_continue")), new UIFadeInTransition()));
				}
				else
				{
					if (dynamic_cast<UIScene_ConfirmDialog*>(e.Scene)->Get_Selected_Index() == 1)
					{
						float multiplier = m_profile->Get_Stat_Multiplier(Profile_Stat::Sell_Price);
						if (m_sell_item->was_inventory_drop)
						{
							multiplier *= INVENTORY_ITEM_SELL_PRICE_MULTIPLIER;
						}

						m_profile->Coins += (int)(m_sell_item->archetype->cost * multiplier);
						m_profile->Remove_Item(m_sell_item, true);
					
						Refresh_Item_List();
					}
				}
				m_sell_item = NULL;
			}
			break;
		}
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{	
				if (m_combine_item != NULL)
				{
					m_combine_item = NULL;
				}
				else
				{
					if (m_profile->Unpack_Events.size() <= 0)
					{
						manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
					}
				}
			}
			else if (e.Source->Get_Name() == "equip_button")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("item_option_menu");
				UIGrid* grid = Find_Element<UIGrid*>("grid_view");

				drop_down_menu->Hide();
				
				int index = grid->Get_Selected_Index();
				ProfileItemStack& stack = m_item_list.at(index);

				if (stack.Archetype->is_unpackable)
				{
					m_profile->Unpack_Item(stack.Items[0]);
					Refresh_Item_List();
				}
				else
				{
					if (stack.Items[0]->equip_slot != -1)
					{
						m_profile->Unequip_Item(stack.Items[0]);
						m_profile->Ensure_Valid_Equips(stack.Items[0]);
					}
					else
					{
						if (stack.Items[0]->Is_Broken())
						{
							manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_cannot_equip_broken"), S("#menu_continue")), new UIFadeInTransition()));
						}
						else
						{
							m_profile->Equip_Item(stack.Items[0]);
						}
					}
				}

				Update_Equip_Items();
			}
			else if (e.Source->Get_Name() == "combine_button")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("item_option_menu");
				UIGrid* grid = Find_Element<UIGrid*>("grid_view");

				drop_down_menu->Hide();
				
				int index = grid->Get_Selected_Index();
				ProfileItemStack& stack = m_item_list.at(index);

				m_combine_item = stack.Items[0];
			}
			else if (e.Source->Get_Name() == "upgrade_button")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("item_option_menu");
				UIGrid* grid = Find_Element<UIGrid*>("grid_view");
				drop_down_menu->Hide();

				int index = grid->Get_Selected_Index();
				ProfileItemStack& stack = m_item_list.at(index);

				manager->Go(UIAction::Push(new UIScene_UpgradeWeapon(m_profile, stack.Items[0]), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "sell_button")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("item_option_menu");				
				UIGrid* grid = Find_Element<UIGrid*>("grid_view");

				int index = grid->Get_Selected_Index();
				ProfileItemStack& stack = m_item_list.at(index);

				int head_count = 0;
				for (std::vector<ProfileItemStack>::iterator iter = m_item_list.begin(); iter != m_item_list.end(); iter++)
				{
					ProfileItemStack& s = *iter;
					if (s.Archetype->item_slot == ItemSlot::Head && s.Items[0]->was_inventory_drop == false)
					{
						head_count += s.Items.size();
					}
				}

				drop_down_menu->Hide();

				if (!OnlineInventory::Get()->CanAccess() && stack.Items[0]->was_inventory_drop)
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_need_inventory_connection"), S("#menu_continue")), new UIFadeInTransition()));
				}				
				else if (stack.Archetype->item_slot == ItemSlot::Head && head_count <= 1 && stack.Items[0]->was_inventory_drop == false)
				{					
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_equipment_sell_must_have_head"), S("#menu_continue")), new UIFadeInTransition()));				
				}
				else
				{
					std::string name = stack.Items[0]->Get_Display_Name();

					float multiplier = m_profile->Get_Stat_Multiplier(Profile_Stat::Sell_Price);
					if (stack.Items[0]->was_inventory_drop)
					{
						multiplier *= INVENTORY_ITEM_SELL_PRICE_MULTIPLIER;
					}

					std::string price = StringHelper::Format_Number((float)(stack.Archetype->cost * multiplier));

					if (stack.Items[0]->was_inventory_drop)
					{
						manager->Go(UIAction::Push(new UIScene_ConfirmDialog(SF("#menu_equipment_sell_confirm_inventory", name.c_str(), price.c_str()), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));				
					}
					else
					{
						manager->Go(UIAction::Push(new UIScene_ConfirmDialog(SF("#menu_equipment_sell_confirm", name.c_str(), price.c_str()), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));				
					}

					m_sell_item = stack.Items.at(0);
				}
			}
			else if (e.Source->Get_Name() == "close_button")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("item_option_menu");
				drop_down_menu->Hide();
			}
		}
		break;
	}
}

void UIScene_Equipment::Perform_Combine(Item* item_a, Item* item_b)
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();

	if (item_a->archetype->use_custom_color || item_b->archetype->use_custom_color)
	{
		Item* colored_item = item_a;
		if (item_a->archetype->use_custom_color)
		{
			colored_item = item_b;
		}

		m_custom_tint_color = colored_item->archetype->default_tint;
		manager->Go(UIAction::Push(new UIScene_PaintItemColorSelector(&m_custom_tint_color, colored_item), new UIFadeInTransition()));

		m_color_selector_pending = true;
		m_color_selector_item_a = item_a;
		m_color_selector_item_b = item_b;
	}
	else
	{
		m_profile->Combine_Item(item_a, item_b);
		manager->Play_UI_Sound(UISoundType::Paint);
		Refresh_Item_List();
	}
}