// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ItemBrokenDialog.h"
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

UIScene_ItemBrokenDialog::UIScene_ItemBrokenDialog(Item* item, Profile* profile)
	: m_item(item)
	, m_profile(profile)
{
	Set_Layout("item_broken");
}

bool UIScene_ItemBrokenDialog::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_ItemBrokenDialog::Get_Name()
{
	return "UIScene_ItemBrokenDialog";
}

bool UIScene_ItemBrokenDialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_ItemBrokenDialog::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ItemBrokenDialog::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene_ItemBrokenDialog::Is_Focusable()
{
	return true;
}

void UIScene_ItemBrokenDialog::Enter(UIManager* manager)
{
	AtlasFrame* back_image = NULL;
	AtlasFrame* fore_image = NULL;
	std::string item_name = "";
	std::string item_description = "";
	Color fore_color = Color::White;

	ItemArchetype* archetype = m_item->archetype;
	std::string name = m_item->Get_Display_Name();

	item_description = archetype->Get_Description(false);

	if (archetype->is_tintable)
	{
		back_image = archetype->icon_animations[Directions::S]->Frames[0];
		fore_image = archetype->icon_tint_animations[Directions::S]->Frames[0];
	}
	else
	{
		back_image = archetype->icon_animations[Directions::S]->Frames[0];
	}

	fore_color = m_item->primary_color;

	Find_Element<UILabel*>("name_label")->Set_Value(item_name);
	Find_Element<UILabel*>("description_label")->Set_Value(item_description);
	Find_Element<UIPanel*>("image_panel")->Set_Background_Image(back_image);
	Find_Element<UIPanel*>("image_panel")->Set_Foreground_Image(fore_image);
	Find_Element<UIPanel*>("image_panel")->Set_Foreground_Color(fore_color);

	// Remove item from users inventory.
	m_profile->Remove_Item(m_item);
	
	Game::Get()->Queue_Save();
}

void UIScene_ItemBrokenDialog::Exit(UIManager* manager)
{
}

void UIScene_ItemBrokenDialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ItemBrokenDialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ItemBrokenDialog::Recieve_Event(UIManager* manager, UIEvent e)
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
