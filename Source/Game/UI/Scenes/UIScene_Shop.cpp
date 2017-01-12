// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Shop.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Game/UI/Scenes/UIScene_BuyItemDialog.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
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
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Online/OnlinePlatform.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"

bool Sort_Item_By_Name(ItemArchetype* a, ItemArchetype* b)
{
	if (a->tmp_unlocked != b->tmp_unlocked)
	{
		return (a->tmp_unlocked && !b->tmp_unlocked);
	}
	if (a->item_slot != b->item_slot)
	{
		return (a->item_slot < b->item_slot);
	}
	return (strcmp(a->base_name.c_str(), b->base_name.c_str()) < 0);
}

UIScene_Shop::UIScene_Shop(Profile* profile)
	: m_profile(profile)
	, m_direction(0)
	, m_direction_change_timer(0.0f)
	, m_purchase_item(NULL)
	, m_display_coins(0)
	, m_coin_sound_timer(0.0f)
	, m_info_alpha(0.0f)
	, m_last_selected_item_index(0)
	, m_custom_color_delta(0.0f)
{
	DBG_ASSERT(profile != NULL);
	DBG_LOG("Opening shop for: %s", profile->Name.c_str());
	
	m_display_coins = (float)m_profile->Coins;

	Set_Layout("shop");	
	
	Refresh_Item_List();
	
	m_draw_item_delegate = new Delegate<UIScene_Shop, UIGridDrawItemData>(this, &UIScene_Shop::On_Draw_Item);
	
	UIGrid* grid = Find_Element<UIGrid*>("grid_view");
	grid->On_Draw_Item += m_draw_item_delegate;

	// Add all item categories to combo box.
	UIComboBox* source_box = Find_Element<UIComboBox*>("source_box");
	source_box->Clear_Items();

	for (std::vector<std::string>::iterator iter = m_item_categories.begin(); iter != m_item_categories.end(); iter++)
	{
		source_box->Add_Item((*iter).c_str());
	}

	Category_Changed(m_item_categories.at(0).c_str());
}

UIScene_Shop::~UIScene_Shop()
{
	SAFE_DELETE(m_draw_item_delegate);
}

const char* UIScene_Shop::Get_Name()
{
	return "UIScene_Shop";
}

bool UIScene_Shop::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Shop::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Shop::Is_Focusable()
{
	return true;
}

void UIScene_Shop::Category_Changed(const char* name)
{
	DBG_LOG("Changed to shop category '%s'.", name);

	m_category_items.clear();
	
	for (std::vector<ItemArchetype*>::iterator iter = m_item_list.begin(); iter != m_item_list.end(); iter++)
	{
		ItemArchetype* item = *iter;
		
		std::string resolved_category = S(item->category);

		if (resolved_category == name)
		{
			m_category_items.push_back(item);
		}

		item->tmp_unlocked = m_profile->Is_Item_Unlocked(item);
	}

	std::sort(m_category_items.begin(), m_category_items.end(), Sort_Item_By_Name);

	UIGrid* grid = Find_Element<UIGrid*>("grid_view");
	grid->Set_Total_Items(m_category_items.size());

	m_last_selected_item_index = -1;
}

void UIScene_Shop::Refresh_Item_List()
{
	std::vector<ItemArchetype*> items = ItemManager::Get()->Get_Archetypes();
	
	m_item_list.clear();
	m_item_categories.clear();

	for (std::vector<ItemArchetype*>::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		ItemArchetype* item = *iter;
		if (item->is_buyable)
		{			
			m_item_list.push_back(item);

			std::string resolved_category = S(item->category);

			std::vector<std::string>::iterator find_iter = std::find(m_item_categories.begin(), m_item_categories.end(), resolved_category);
			if (find_iter == m_item_categories.end())
			{
				m_item_categories.push_back(resolved_category);
			}
		}	
	}
}

void UIScene_Shop::Enter(UIManager* manager)
{
	UILabel* gold_label = Find_Element<UILabel*>("gold_label");

	if (m_display_coins>= m_profile->Wallet_Size)
	{
		gold_label->Set_Value(SF("#menu_inventory_gold_full", StringHelper::Format_Number((float)ceilf(m_display_coins)).c_str()));
	}
	else
	{
		gold_label->Set_Value(SF("#menu_inventory_gold", StringHelper::Format_Number((float)ceilf(m_display_coins)).c_str()));
	}
}	

void UIScene_Shop::Exit(UIManager* manager)
{
}	

void UIScene_Shop::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{	
	Vector2 ui_scale = manager->Get_UI_Scale();

	UILabel* gold_label = Find_Element<UILabel*>("gold_label");
	UIGrid* grid = Find_Element<UIGrid*>("grid_view");
	UIPanel* info_panel = Find_Element<UIPanel*>("info_panel");
	UILabel* info_name_label = Find_Element<UILabel*>("info_name_label");
	UILabel* info_price_label = Find_Element<UILabel*>("info_price_label");
	UILabel* info_description_label = Find_Element<UILabel*>("info_description_label");

	int item_index = grid->Get_Selected_Index();

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

	if (m_display_coins>= m_profile->Wallet_Size)
	{
		gold_label->Set_Value(SF("#menu_inventory_gold_full", StringHelper::Format_Number((float)ceilf(m_display_coins)).c_str()));
	}
	else
	{
		gold_label->Set_Value(SF("#menu_inventory_gold", StringHelper::Format_Number((float)ceilf(m_display_coins)).c_str()));
	}

	// Show info dialog if we hover long enough.
	if (grid->Is_Focused(manager) && grid->Get_Selection_Time() >= INFO_FADE_IN_DELAY && item_index >= 0 && item_index < (int)m_category_items.size())
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
	Rect2D info_box = info_panel->Get_Screen_Box();

	bool left = (item_box.X + (item_box.Width * 0.5f)) <= (screen_width * 0.5f);
	bool top  = (item_box.Y + (item_box.Height * 0.5f)) <= (screen_height * 0.5f);

	float spacing = 5.0f * ui_scale.Y;

	// Top-Left
	if (left == true && top == true)
	{
		info_panel->Reposition(Rect2D(item_box.X, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height));
	}
	// Top-Right
	else if (left == false && top == true)
	{
		info_panel->Reposition(Rect2D(item_box.X + item_box.Width - info_box.Width, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height));
	}
	// Bottom-Left
	else if (left == true && top == false)
	{
		info_panel->Reposition(Rect2D(item_box.X, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height));
	}
	// Bottom-Right
	else if (left == false && top == false)
	{
		info_panel->Reposition(Rect2D(item_box.X + item_box.Width - info_box.Width, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height));
	}

	info_panel->Set_Frame_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_name_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_price_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_description_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));

	// Show info.
	if (item_index >= 0 && item_index < (int)m_category_items.size() && m_last_selected_item_index != item_index)
	{
		ItemArchetype* item = m_category_items.at(item_index);
		bool bUnlocked = m_profile->Is_Item_Unlocked(item);

#ifdef OPT_PREMIUM_ACCOUNTS
		if (item->is_premium_only && OnlinePlatform::Get()->Check_Purchased(*GameOptions::premium_account_appid))
		{
			bUnlocked = true;
		}
#endif

		info_name_label->Set_Value(item->Get_Base_Display_Name());

		if (bUnlocked)
		{
			info_price_label->Set_Value(SF("#menu_inventory_gold", StringHelper::Format_Number((float)((int)(item->cost * m_profile->Get_Stat_Multiplier(Profile_Stat::Buy_Price)))).c_str()));
		}
		else
		{
			info_price_label->Set_Value(S("#menu_inventory_locked"));
		}
		
		std::string result = item->Get_Description(false);

		if (!bUnlocked)
		{
			ItemArchetype* unlock_item = NULL;
			if (item->unlock_critera_item != NULL)
			{
				unlock_item = ItemManager::Get()->Find_Archetype(item->unlock_critera_item);
			}

			switch (item->unlock_critera)
			{
			case ItemUnlockCriteria::Required_Rank:
				{
					result += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_required_rank", item->unlock_critera_threshold));
					break;
				}
			case ItemUnlockCriteria::Ailments_From_Weapon:
				{
					float amt = m_profile->Get_Item_Stat(unlock_item, ProfileItemStat::Ailments_Inflicted);
					result += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_ailments_from_weapon", (int)amt, item->unlock_critera_threshold, unlock_item->Get_Base_Display_Name().c_str()));
					break;
				}
			case ItemUnlockCriteria::Damage_With_Weapon:
				{
					float amt = m_profile->Get_Item_Stat(unlock_item, ProfileItemStat::Damage_Inflicted);
					if (item->unlock_critera_threshold < 0)
					{
						result += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_heal_from_weapon", (int)abs(amt), (int)abs(item->unlock_critera_threshold), unlock_item->Get_Base_Display_Name().c_str()));
					}
					else
					{
						result += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_damage_from_weapon", (int)amt, item->unlock_critera_threshold, unlock_item->Get_Base_Display_Name().c_str()));
					}
					break;
				}
			case ItemUnlockCriteria::Kills_With_Weapon:
				{
					float amt = m_profile->Get_Item_Stat(unlock_item, ProfileItemStat::Enemies_Killed);
					result += StringHelper::Format("\n\n%s\n%s", S("#menu_shop_unlock_criteria"), SF("#menu_shop_unlock_kills_from_weapon", (int)amt, item->unlock_critera_threshold, unlock_item->Get_Base_Display_Name().c_str()));
					break;
				}
			}
		}

		info_description_label->Set_Value(result);
	}

	m_last_selected_item_index = item_index;

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Shop::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	m_direction_change_timer += time.Get_Frame_Time();
	if (m_direction_change_timer >= DIRECTION_CHANGE_INTERVAL)
	{
		m_direction = (m_direction + 1) % 8;
		m_direction_change_timer = 0.0f;
	}

	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Shop::On_Draw_Item(UIGridDrawItemData* data)
{
	Vector2 ui_scale = data->manager->Get_UI_Scale();
	ItemArchetype* item = m_category_items.at(data->item_index);

	ui_scale.X *= 0.75f;
	ui_scale.Y *= 0.75f;

	Vector2 position = Vector2
	(
		data->item_bounds.X + (data->item_bounds.Width * 0.5f) - (item->icon_offset.X * ui_scale.X),
		data->item_bounds.Y + (data->item_bounds.Height * 0.5f) - (item->icon_offset.Y * ui_scale.Y)
	);

	float w = data->item_bounds.Width * 0.33f;
	float h = data->item_bounds.Height * 0.33f;
	float p = 5.0f * ui_scale.X;
	Rect2D lock_area = Rect2D(data->item_bounds.X + data->item_bounds.Width - (w + p), data->item_bounds.Y + data->item_bounds.Height - (h + p), w, h);//data->item_bounds.Inflate(-5, -5);
	
	Color tint_color = item->default_tint;
	Color base_color = Color::White;

	if (item->use_custom_color)
	{
		m_custom_color_delta += data->time.Get_Delta_Seconds();
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

	bool bUnlocked = m_profile->Is_Item_Unlocked(item);

#ifdef OPT_PREMIUM_ACCOUNTS
	if (item->is_premium_only && OnlinePlatform::Get()->Check_Purchased(*GameOptions::premium_account_appid))
	{
		bUnlocked = true;
	}
#endif

	if (!bUnlocked)
	{
		base_color.A = 32;
		tint_color.A = 32;
		
		/*
		int grey = (base_color.R + base_color.G + base_color.B) / 3;
		base_color = Color(grey, grey, grey, 96);

		grey = (tint_color.R + tint_color.G + tint_color.B) / 3;
		tint_color = Color(grey, grey, grey, 96);*/
	}

	AtlasRenderer atlas_renderer(NULL);
	if (item->is_icon_direction_based && item->item_slot != 1)
	{
		if (item->is_tintable)
		{
			atlas_renderer.Draw_Frame(item->icon_animations[m_direction]->Frames[0], position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);	
			atlas_renderer.Draw_Frame(item->icon_tint_animations[m_direction]->Frames[0], position, 0.0f, tint_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
		}
		else
		{
			atlas_renderer.Draw_Frame(item->icon_animations[m_direction]->Frames[0], position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
		}
	}
	else
	{
		if (item->is_tintable)
		{
			atlas_renderer.Draw_Frame(item->icon_animations[0]->Frames[0], position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
			atlas_renderer.Draw_Frame(item->icon_tint_animations[0]->Frames[0], position, 0.0f, tint_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
		}
		else
		{
			atlas_renderer.Draw_Frame(item->icon_animations[0]->Frames[0], position, 0.0f, base_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
		}
	}

	if (!bUnlocked)
	{
		// Draw lock!
		atlas_renderer.Draw_Frame("achievement_locked_clear", lock_area, 0.0f, Color(255, 255, 255, 128));
	}
}

void UIScene_Shop::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
#ifndef OPT_MICROTRANSACTION_ITEMS
	case UIEventType::Dialog_Close:
		{
			if (m_purchase_item != NULL)
			{
				if (dynamic_cast<UIScene_ConfirmDialog*>(e.Scene)->Get_Selected_Index() == 1)
				{
					m_profile->Add_Item(m_purchase_item);
					m_profile->Coins -= (int)(m_purchase_item->cost * m_profile->Get_Stat_Multiplier(Profile_Stat::Buy_Price));
				}
				m_purchase_item = NULL;
			}
			break;
		}
#endif
	case UIEventType::Grid_Item_Click:
		{			
			UIGrid* grid = dynamic_cast<UIGrid*>(e.Source);
			int index = grid->Get_Selected_Index();
			if (index >= 0 && index < (int)m_category_items.size())
			{
				manager->Play_UI_Sound(UISoundType::Select);

				ItemArchetype* item = m_category_items.at(index);

#ifndef OPT_MICROTRANSACTION_ITEMS
				int required_coins = m_profile->Coins - (int)(item->cost * m_profile->Get_Stat_Multiplier(Profile_Stat::Buy_Price));

#ifdef OPT_PREMIUM_ACCOUNTS
				if (item->is_premium_only && !OnlinePlatform::Get()->Check_Purchased(*GameOptions::premium_account_appid))
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_shop_purchase_premium_only")), new UIFadeInTransition()));
				}
				else if (!m_profile->Is_Item_Unlocked(item) && !item->is_premium_only)
#else
				if (!m_profile->Is_Item_Unlocked(item))
#endif
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_shop_purchase_item_locked")), new UIFadeInTransition()));
				}
				else if (!m_profile->Is_Space_For_Item(item))
				{	
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_shop_purchase_full_inventory")), new UIFadeInTransition()));
				}
				else if (required_coins >= 0)
				{
					std::string name = item->Get_Base_Display_Name();
					std::string price = StringHelper::Format_Number((float)((int)(item->cost * m_profile->Get_Stat_Multiplier(Profile_Stat::Buy_Price))));

					manager->Go(UIAction::Push(new UIScene_ConfirmDialog(SF("#menu_shop_purchase_item_confirm", name.c_str(), price.c_str()), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));
					//	manager->Play_UI_Sound(UISoundType::Popup);

					m_purchase_item = item;
				}
				else
				{
					std::string name = item->Get_Base_Display_Name();
					std::string price = StringHelper::Format_Number((float)abs(required_coins));

					manager->Go(UIAction::Push(new UIScene_Dialog(SF("#menu_shop_purchase_not_enough_coins", price.c_str(), name.c_str()), S("#menu_continue")), new UIFadeInTransition()));
					//manager->Play_UI_Sound(UISoundType::Popup);
				}
#else
				manager->Go(UIAction::Push(new UIScene_BuyItemDialog(item, m_profile), new UIFadeInTransition()));
#endif
			}
			else
			{
				manager->Play_UI_Sound(UISoundType::Back);
			}
		}
		break;
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			UIComboBox* combo_box = dynamic_cast<UIComboBox*>(e.Source);
			Category_Changed(combo_box->Get_Selected_Item().Text.c_str());
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{	
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
		}
		break;
	}
}
