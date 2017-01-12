// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_GameplayOptions.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_GameplayOptions::UIScene_GameplayOptions()
{
	Set_Layout("gameplay_options");	
}

const char* UIScene_GameplayOptions::Get_Name()
{
	return "UIScene_GameplayOptions";
}

bool UIScene_GameplayOptions::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_GameplayOptions::Should_Display_Cursor()
{
	return true;
}

bool UIScene_GameplayOptions::Is_Focusable()
{
	return true;
}

UIScene* UIScene_GameplayOptions::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_GameplayOptions::Enter(UIManager* manager)
{
	Find_Element<UICheckBox*>("screen_shake_checkbox")->Set_Checked(*GameOptions::screen_shake_enabled);
	Find_Element<UICheckBox*>("interaction_arrows_checkbox")->Set_Checked(*GameOptions::interaction_arrows_enabled);
	Find_Element<UICheckBox*>("interaction_messages_checkbox")->Set_Checked(*GameOptions::interaction_messages_enabled);
	//Find_Element<UICheckBox*>("occluding_objects_checkbox")->Set_Checked(*GameOptions::occlusion_transparency_enabled);
	Find_Element<UICheckBox*>("input_to_pickup_checkbox")->Set_Checked(*GameOptions::require_input_to_pickup_items);
	Find_Element<UICheckBox*>("fire_when_aiming_checkbox")->Set_Checked(*GameOptions::fire_when_aiming_enabled);
	Find_Element<UICheckBox*>("show_numeric_stats_checkbox")->Set_Checked(*GameOptions::show_numeric_stats);
}

void UIScene_GameplayOptions::Exit(UIManager* manager)
{
	*GameOptions::screen_shake_enabled				= Find_Element<UICheckBox*>("screen_shake_checkbox")->Get_Checked();
	*GameOptions::interaction_arrows_enabled		= Find_Element<UICheckBox*>("interaction_arrows_checkbox")->Get_Checked();
	*GameOptions::interaction_messages_enabled		= Find_Element<UICheckBox*>("interaction_messages_checkbox")->Get_Checked();
	//*GameOptions::occlusion_transparency_enabled	= Find_Element<UICheckBox*>("occluding_objects_checkbox")->Get_Checked();
	*GameOptions::require_input_to_pickup_items		= Find_Element<UICheckBox*>("input_to_pickup_checkbox")->Get_Checked();
	*GameOptions::fire_when_aiming_enabled			= Find_Element<UICheckBox*>("fire_when_aiming_checkbox")->Get_Checked();
	*GameOptions::show_numeric_stats				= Find_Element<UICheckBox*>("show_numeric_stats_checkbox")->Get_Checked();
}

void UIScene_GameplayOptions::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_GameplayOptions::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_GameplayOptions::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Element_Focus:
		{
			if (e.Source->Get_Name() == "screen_shake_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_gameplay_screen_shake_description"));
			}
			else if (e.Source->Get_Name() == "interaction_arrows_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_gameplay_interaction_arrows_description"));
			}
			else if (e.Source->Get_Name() == "interaction_messages_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_gameplay_interaction_messages_description"));
			}
			/*else if (e.Source->Get_Name() == "occluding_objects_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_gameplay_occluding_objects_description"));
			}*/
			else if (e.Source->Get_Name() == "input_to_pickup_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_gameplay_input_to_pickup_description"));
			}
			else if (e.Source->Get_Name() == "fire_when_aiming_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_gameplay_fire_when_aiming_description"));
			}
			else if (e.Source->Get_Name() == "show_numeric_stats_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_gameplay_show_numeric_stats_description"));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_back_description"));
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}
