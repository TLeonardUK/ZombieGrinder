// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_KeyBindDialog.h"
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

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"

UIScene_KeyBindDialog::UIScene_KeyBindDialog(OutputBindings::Type type)
	: m_output_binding(type)
{
	Set_Layout("text_dialog");	
}

const char* UIScene_KeyBindDialog::Get_Name()
{
	return "UIScene_KeyBindDialog";
}

bool UIScene_KeyBindDialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_KeyBindDialog::Should_Display_Cursor()
{
	return true;
}

bool UIScene_KeyBindDialog::Is_Focusable()
{
	return true;
}

void UIScene_KeyBindDialog::Enter(UIManager* manager)
{
	Find_Element<UILabel*>("label")->Set_Value("#menu_options_input_press_bind_key");
}	


void UIScene_KeyBindDialog::Exit(UIManager* manager)
{
}	

void UIScene_KeyBindDialog::Perform_Binding(InputBindings::Type type)
{
	bool is_joystick = (type > InputBindings::JOYSTICK_START && type < InputBindings::JOYSTICK_END);

	// Unbind the old joystick/keyboard binding.
	for (std::vector<InputBindingConfig>::iterator iter = (*GameOptions::input_bindings).begin(); iter != (*GameOptions::input_bindings).end(); iter++)
	{
		InputBindingConfig& bind_config = *iter;

		bool is_bind_joystick = (bind_config.input > InputBindings::JOYSTICK_START && bind_config.input < InputBindings::JOYSTICK_END);

		if (bind_config.output == m_output_binding && is_bind_joystick == is_joystick)
		{
			(*GameOptions::input_bindings).erase(iter);
			DBG_LOG("Erased old binding");
			break;
		}
	}

	// Push new binding.
	InputBindingConfig new_binding;
	new_binding.output = m_output_binding;
	new_binding.input = type;
	(*GameOptions::input_bindings).push_back(new_binding);
}

void UIScene_KeyBindDialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	bool				found_binding = false;
	InputBindings::Type output_binding;

	for (int i = 0; i < MAX_INPUT_SOURCES && found_binding == false; i++)
	{
		Input* input = Input::Get(i);

		for (int j = InputBindings::JOYSTICK_START + 1; j < InputBindings::JOYSTICK_END; j++)
		{
			InputBindings::Type binding = static_cast<InputBindings::Type>(j);

			if (input->Was_Pressed(binding))
			{
				found_binding = true;
				output_binding = binding;
			}
		}

		for (int j = InputBindings::KEYBOARD_START + 1; j < InputBindings::KEYBOARD_END; j++)
		{
			InputBindings::Type binding = static_cast<InputBindings::Type>(j);

			if (input->Was_Pressed(binding))
			{
				found_binding = true;
				output_binding = binding;
			}
		}
	}

	if (found_binding == true)
	{
		Perform_Binding(output_binding);
		manager->Go(UIAction::Pop(new UIFadeOutTransition()));
		return;		
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_KeyBindDialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_KeyBindDialog::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
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
