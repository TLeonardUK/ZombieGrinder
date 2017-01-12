// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Options.h"
#include "Game/UI/Scenes/UIScene_InputOptions.h"
#include "Game/UI/Scenes/UIScene_GraphicsOptions.h"
#include "Game/UI/Scenes/UIScene_MiscOptions.h"
#include "Game/UI/Scenes/UIScene_AudioOptions.h"
#include "Game/UI/Scenes/UIScene_GameplayOptions.h"
#include "Game/UI/Scenes/UIScene_MainMenu.h"
#include "Game/UI/Scenes/UIScene_ModOptions.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_Credits.h"
#include "Game/UI/Scenes/UIScene_ConfirmResolutionChange.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Input/Steam/Steam_Input.h"
#include "Engine/Input/Steam/Steam_JoystickState.h"

#include "Engine/Online/OnlinePlatform.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameOptions.h"
#include "Engine/Engine/EngineOptions.h"
#include "Engine/Options/OptionRegistry.h"

UIScene_Options::UIScene_Options()
	: m_resolution_changed_confirmed(false)
{
	OptionRegistry::Get()->Push_State();

	old_display_width	= *EngineOptions::render_display_width;
	old_display_height	= *EngineOptions::render_display_height;
	old_display_mode	= *EngineOptions::render_display_mode;
	old_display_hertz	= *EngineOptions::render_display_hertz;

	Set_Layout("options");	
}

UIScene_Options::~UIScene_Options()
{
}

const char* UIScene_Options::Get_Name()
{
	return "UIScene_Options";
}

bool UIScene_Options::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Options::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Options::Is_Focusable()
{
	return true;
}

UIScene* UIScene_Options::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_Options::Enter(UIManager* manager)
{
}	

void UIScene_Options::Exit(UIManager* manager)
{
}	

void UIScene_Options::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	if (m_resolution_changed_confirmed)
	{
		manager->Go(UIAction::Pop(new UISlideInTransition()));
		return;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Options::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Options::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Element_Focus:
		{
			if (e.Source->Get_Name() == "input_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_input_description"));
			}
			else if (e.Source->Get_Name() == "audio_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_audio_description"));
			}
			else if (e.Source->Get_Name() == "graphics_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_graphics_description"));
			}
			else if (e.Source->Get_Name() == "gameplay_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_gameplay_description"));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_back_description"));
			}
			else if (e.Source->Get_Name() == "apply_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_apply_description"));
			}
			else if (e.Source->Get_Name() == "misc_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_misc_description"));
			}
			else if (e.Source->Get_Name() == "mod_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_mod_description"));
			}
			else if (e.Source->Get_Name() == "credits_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_options_credits_description"));
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "input_button")
			{
#ifdef STEAM_CONTROLLER_SUPPORT
				if (e.Source_Joystick != NULL && dynamic_cast<Steam_JoystickState*>(e.Source_Joystick) != NULL)
				{
					if (!dynamic_cast<Steam_JoystickState*>(e.Source_Joystick)->Show_Binding_Screen())
					{
						manager->Go(UIAction::Push(new UIScene_Dialog(S("#big_picture_overlay_disabled_joystick_binding")), new UIFadeInTransition()));
					}
				}
				else
				{
#endif
				manager->Go(UIAction::Push(new UIScene_InputOptions(), new UISlideInTransition()));
#ifdef STEAM_CONTROLLER_SUPPORT
				}
#endif
			}
			else if (e.Source->Get_Name() == "audio_button")
			{
				manager->Go(UIAction::Push(new UIScene_AudioOptions(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "graphics_button")
			{
				manager->Go(UIAction::Push(new UIScene_GraphicsOptions(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "gameplay_button")
			{
				manager->Go(UIAction::Push(new UIScene_GameplayOptions(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "misc_button")
			{
				manager->Go(UIAction::Push(new UIScene_MiscOptions(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "mod_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_ModOptions(), new UISlideInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "credits_button")
			{
				manager->Go(UIAction::Push(new UIScene_Credits(), new UIFadeTransition()));
			}
			else if (e.Source->Get_Name() == "apply_button")
			{
				Game::Get()->Apply_Config();

				// Incase we transition to legacy render prepare for it!
				RenderPipeline::Get()->Prepare_For_Legacy();

				if (*EngineOptions::render_display_width	!= old_display_width ||
					*EngineOptions::render_display_height	!= old_display_height ||
					*EngineOptions::render_display_mode		!= old_display_mode ||
					*EngineOptions::render_display_hertz	!= old_display_hertz)
				{
					OptionRegistry::Get()->Pop_State(false);

					manager->Go(UIAction::Push(new UIScene_ConfirmResolutionChange(old_display_width, old_display_height, old_display_mode, old_display_hertz), new UIFadeInTransition()));
					m_resolution_changed_confirmed = true;
				}
				else
				{
					OptionRegistry::Get()->Pop_State(false);

					manager->Go(UIAction::Pop(new UISlideInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				OptionRegistry::Get()->Pop_State(true);

				Game::Get()->Apply_Config();
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}
