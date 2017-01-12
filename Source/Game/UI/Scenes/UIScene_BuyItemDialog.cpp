// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_BuyItemDialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Online/OnlinePlatform.h"

#include "Game/Profile/Profile.h"
#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"

UIScene_BuyItemDialog::UIScene_BuyItemDialog(ItemArchetype* archetype, Profile* profile)
	: m_archetype(archetype)
	, m_profile(profile)
	, m_pending_close(false)
{
	Set_Layout("buy_item_dialog");
}

bool UIScene_BuyItemDialog::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_BuyItemDialog::Get_Name()
{
	return "UIScene_BuyItemDialog";
}

bool UIScene_BuyItemDialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_BuyItemDialog::Should_Display_Cursor()
{
	return true;
}

bool UIScene_BuyItemDialog::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene_BuyItemDialog::Is_Focusable()
{
	return true;
}

bool UIScene_BuyItemDialog::Should_Fade_Cursor()
{
	return true;
}

void UIScene_BuyItemDialog::Enter(UIManager* manager)
{
	Find_Element<UILabel*>("title")->Set_Value(SF("#menu_buy_item_title", m_archetype->Get_Display_Name(false, 0).c_str()));

	int cost = (int)(m_archetype->cost * m_profile->Get_Stat_Multiplier(Profile_Stat::Buy_Price));
	int required_coins = cost - m_profile->Coins;
	
#ifdef OPT_PREMIUM_ACCOUNTS
	if (m_archetype->is_premium_only)
	{
		Find_Element<UILabel*>("button_1")->Set_Value(S("#menu_buy_item_fragile_item_button_not_available"));
		Find_Element<UILabel*>("button_1")->Set_Enabled(false);		
	}
	else
#endif
	if (!m_profile->Is_Space_For_Item(m_archetype))
	{	
		Find_Element<UILabel*>("button_1")->Set_Value(S("#menu_buy_item_fragile_item_button_inventory_full"));
		Find_Element<UILabel*>("button_1")->Set_Enabled(false);
	}
	else if (required_coins > 0)
	{
		Find_Element<UILabel*>("button_1")->Set_Value(SF("#menu_buy_item_fragile_item_button_not_enoungh_coins", StringHelper::Format_Number((float)required_coins).c_str()));
		Find_Element<UILabel*>("button_1")->Set_Enabled(false);
	}
	else
	{
		Find_Element<UILabel*>("button_1")->Set_Value(SF("#menu_buy_item_fragile_item_button", StringHelper::Format_Number((float)cost).c_str()));
		Find_Element<UILabel*>("button_1")->Set_Enabled(true);
	}
}

void UIScene_BuyItemDialog::Exit(UIManager* manager)
{
}

void UIScene_BuyItemDialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	if (m_pending_close && !OnlinePlatform::Get()->Is_Dialog_Open())
	{
		m_pending_close = false;
		ProfileManager::Get()->Sync_Profiles();
		manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));
		manager->Go(UIAction::Pop(new UIFadeOutTransition()));
		return;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_BuyItemDialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_BuyItemDialog::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
		case UIEventType::Button_Click:
		{
			// Fragile item.
			if (e.Source->Get_Name() == "button_1")
			{
				m_profile->Add_Item(m_archetype);
				m_profile->Coins -= (int)(m_archetype->cost * m_profile->Get_Stat_Multiplier(Profile_Stat::Buy_Price));
			}
			// Indestructable item.
			else if (e.Source->Get_Name() == "button_2")
			{
				// Buy me!
				OnlinePlatform::Get()->Show_Web_Browser(StringHelper::Format("https://store.steampowered.com/buyitem/%i/%i/", STEAM_APP_ID, m_archetype->Get_Inventory_ItemDefID()).c_str());
			}
			// Cancel.
			else if (e.Source->Get_Name() == "button_3")
			{
				// Nothing to do.
			}

			m_pending_close = true;

			break;
		}
	}
}
