// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_InventoryDropDialog.h"
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

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Profile/ItemManager.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_InventoryDropDialog::UIScene_InventoryDropDialog(OnlineInventoryItem item)
	: m_item(item)
	, m_use_online_item(true)
	, m_use_unpack_event(false)
{
	Set_Layout("inventory_drop");	
}

UIScene_InventoryDropDialog::UIScene_InventoryDropDialog(ItemArchetype* type)
	: m_archetype(type)
	, m_use_online_item(false)
	, m_use_unpack_event(false)
{
	Set_Layout("inventory_drop");

	Find_Element<UILabel*>("intro_label")->Set_Value("#inventory_item_challenge_drop_title");
}

UIScene_InventoryDropDialog::UIScene_InventoryDropDialog(ProfileUnpackEvent evt)
	: m_use_unpack_event(true)
	, m_use_online_item(false)
	, m_unpack_event(evt)
{
	Set_Layout("inventory_drop");

	Find_Element<UILabel*>("intro_label")->Set_Value("#inventory_item_unpack_drop_title");
}

bool UIScene_InventoryDropDialog::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_InventoryDropDialog::Get_Name()
{
	return "UIScene_InventoryDropDialog";
}

bool UIScene_InventoryDropDialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_InventoryDropDialog::Should_Display_Cursor()
{
	return true;
}

bool UIScene_InventoryDropDialog::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene_InventoryDropDialog::Is_Focusable()
{
	return true;
}

void UIScene_InventoryDropDialog::Enter(UIManager* manager)
{
	AtlasFrame* back_image = NULL;
	AtlasFrame* fore_image = NULL;
	std::string item_name = "";
	std::string item_description = "";
	Color fore_color = Color::White;

	if (!m_use_unpack_event || m_unpack_event.type == ProfileUnpackEventType::Item)
	{
		ItemArchetype* archetype = m_archetype;
		if (m_use_online_item)
		{
			archetype = ItemManager::Get()->Find_Archetype_By_InventoryId(m_item.type_id);
		}
		else if (m_use_unpack_event)
		{
			archetype = m_unpack_event.item;
		}


		std::string name = archetype->Get_Display_Name(m_use_online_item, 0);
		item_description = archetype->Get_Description(false);

		if (m_use_online_item)
		{
			item_description += "\n\n";
			item_description += UniqueItemStats(m_item.original_unique_id64, archetype->is_equippable).ToString();
		}

		if (archetype->is_tintable)
		{
			back_image = archetype->icon_animations[Directions::S]->Frames[0];
			fore_image = archetype->icon_tint_animations[Directions::S]->Frames[0];
		}
		else
		{
			back_image = archetype->icon_animations[Directions::S]->Frames[0];
		}

		fore_color = archetype->default_tint;
	}
	else
	{
		switch (m_unpack_event.type)
		{
		case ProfileUnpackEventType::Coins:
			{
				item_name = SF("#inventory_item_drop_coins_name", (int)m_unpack_event.amount);
				item_description = "#inventory_item_drop_coins_description";
				back_image = ResourceFactory::Get()->Get_Atlas_Frame("item_accessory_coins_drop_0");
				break;
			}
		case ProfileUnpackEventType::XP:
			{
				item_name = SF("#inventory_item_drop_xp_name", (int)m_unpack_event.amount);
				item_description = "#inventory_item_drop_xp_description";
				back_image = ResourceFactory::Get()->Get_Atlas_Frame("item_accessory_xp_drop_0");
				fore_image = NULL;
				break;
			}
		}
	}

	Find_Element<UILabel*>("name_label")->Set_Value(item_name);
	Find_Element<UILabel*>("description_label")->Set_Value(item_description);
	Find_Element<UIPanel*>("image_panel")->Set_Background_Image(back_image);
	Find_Element<UIPanel*>("image_panel")->Set_Foreground_Image(fore_image);
	Find_Element<UIPanel*>("image_panel")->Set_Foreground_Color(fore_color);

	Game::Get()->Queue_Save();
}	

void UIScene_InventoryDropDialog::Exit(UIManager* manager)
{
}	

void UIScene_InventoryDropDialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_InventoryDropDialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_InventoryDropDialog::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
		}
		break;
	}
}
