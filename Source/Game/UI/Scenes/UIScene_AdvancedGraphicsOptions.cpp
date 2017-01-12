// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_AdvancedGraphicsOptions.h"
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

UIScene_AdvancedGraphicsOptions::UIScene_AdvancedGraphicsOptions()
{
	Set_Layout("advanced_graphics_options");	
}

const char* UIScene_AdvancedGraphicsOptions::Get_Name()
{
	return "UIScene_AdvancedGraphicsOptions";
}

bool UIScene_AdvancedGraphicsOptions::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_AdvancedGraphicsOptions::Should_Display_Cursor()
{
	return true;
}

bool UIScene_AdvancedGraphicsOptions::Is_Focusable()
{
	return true;
}

UIScene* UIScene_AdvancedGraphicsOptions::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_AdvancedGraphicsOptions::Enter(UIManager* manager)
{
	Find_Element<UICheckBox*>("vsync_checkbox")->Set_Checked(*GameOptions::vsync_enabled);
	Find_Element<UICheckBox*>("shaders_checkbox")->Set_Checked(*GameOptions::shaders_enabled);
	Find_Element<UICheckBox*>("distortion_checkbox")->Set_Checked(*GameOptions::distortion_enabled);
	Find_Element<UICheckBox*>("perf_overlay_checkbox")->Set_Checked(*GameOptions::show_perf_overlay);
	Find_Element<UICheckBox*>("extended_perf_overlay_checkbox")->Set_Checked(*GameOptions::show_extended_perf_overlay);
	Find_Element<UICheckBox*>("legacy_checkbox")->Set_Checked(*EngineOptions::render_legacy);

	if (*EngineOptions::render_legacy)
	{
		Find_Element<UICheckBox*>("shaders_checkbox")->Set_Enabled(false);
		Find_Element<UICheckBox*>("distortion_checkbox")->Set_Enabled(false);
	}

	if (*EngineOptions::render_legacy_was_forced)
	{
		Find_Element<UICheckBox*>("legacy_checkbox")->Set_Enabled(false);
	}
}	

void UIScene_AdvancedGraphicsOptions::Exit(UIManager* manager)
{
	*GameOptions::vsync_enabled					= Find_Element<UICheckBox*>("vsync_checkbox")->Get_Checked();
	*GameOptions::shaders_enabled				= Find_Element<UICheckBox*>("shaders_checkbox")->Get_Checked();
	*GameOptions::distortion_enabled			= Find_Element<UICheckBox*>("distortion_checkbox")->Get_Checked();
	*GameOptions::show_perf_overlay				= Find_Element<UICheckBox*>("perf_overlay_checkbox")->Get_Checked();
	*GameOptions::show_extended_perf_overlay	= Find_Element<UICheckBox*>("extended_perf_overlay_checkbox")->Get_Checked();
	*EngineOptions::render_legacy					= Find_Element<UICheckBox*>("legacy_checkbox")->Get_Checked();
}	

void UIScene_AdvancedGraphicsOptions::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_AdvancedGraphicsOptions::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_AdvancedGraphicsOptions::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Element_Focus:
		{
			if (e.Source->Get_Name() == "vsync_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_advanced_graphics_vsync_description"));
			}
			/*else if (e.Source->Get_Name() == "hq2x_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_advanced_graphics_hq2x_description"));
			}*/
			else if (e.Source->Get_Name() == "shaders_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_advanced_graphics_shaders_description"));
			}
			else if (e.Source->Get_Name() == "distortion_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_advanced_graphics_distortion_description"));
			}
			else if (e.Source->Get_Name() == "perf_overlay_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_advanced_graphics_perf_overlay_description"));
			}
			else if (e.Source->Get_Name() == "extended_perf_overlay_checkbox")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_advanced_graphics_extended_perf_overlay_description"));
			}
			else if (e.Source->Get_Name() == "legacy_checkbox")
			{
				if (*EngineOptions::render_legacy_was_forced)
				{
					Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_advanced_graphics_legacy_description_forced"));
				}
				else
				{
					Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_advanced_graphics_legacy_description"));
				}
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

	if (!Find_Element<UICheckBox*>("legacy_checkbox")->Get_Checked())
	{
		Find_Element<UICheckBox*>("shaders_checkbox")->Set_Enabled(true);
		Find_Element<UICheckBox*>("distortion_checkbox")->Set_Enabled(true);
	}
	else
	{
		Find_Element<UICheckBox*>("shaders_checkbox")->Set_Checked(false);
		Find_Element<UICheckBox*>("distortion_checkbox")->Set_Checked(false);
		Find_Element<UICheckBox*>("shaders_checkbox")->Set_Enabled(false);
		Find_Element<UICheckBox*>("distortion_checkbox")->Set_Enabled(false);
	}

	if (!Find_Element<UICheckBox*>("perf_overlay_checkbox")->Get_Checked())
	{
		Find_Element<UICheckBox*>("extended_perf_overlay_checkbox")->Set_Checked(false);
		Find_Element<UICheckBox*>("extended_perf_overlay_checkbox")->Set_Enabled(false);
	}
	else
	{
		Find_Element<UICheckBox*>("extended_perf_overlay_checkbox")->Set_Enabled(true);
	}
}
