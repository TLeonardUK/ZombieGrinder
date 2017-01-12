// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_InputOptions.h"
#include "Game/UI/Scenes/UIScene_KeyBindDialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
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

UIScene_InputOptions::UIScene_InputOptions()
{
	Set_Layout("input_options");	
}

const char* UIScene_InputOptions::Get_Name()
{
	return "UIScene_InputOptions";
}

bool UIScene_InputOptions::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_InputOptions::Should_Display_Cursor()
{
	return true;
}

bool UIScene_InputOptions::Is_Focusable()
{
	return true;
}

UIScene* UIScene_InputOptions::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

InputBindings::Type UIScene_InputOptions::Get_Keyboard_For_Bind(OutputBindings::Type type)
{
	for (std::vector<InputBindingConfig>::iterator iter = (*GameOptions::input_bindings).begin(); iter != (*GameOptions::input_bindings).end(); iter++)
	{
		InputBindingConfig& bind = *iter;
		if (bind.output == type && bind.input > InputBindings::KEYBOARD_START  && bind.input < InputBindings::KEYBOARD_END)
		{
			return bind.input;
		}
	}
	return InputBindings::INVALID;
}

InputBindings::Type  UIScene_InputOptions::Get_Joystick_For_Bind(OutputBindings::Type type)
{
	for (std::vector<InputBindingConfig>::iterator iter = (*GameOptions::input_bindings).begin(); iter != (*GameOptions::input_bindings).end(); iter++)
	{
		InputBindingConfig& bind = *iter;
		if (bind.output == type && bind.input > InputBindings::JOYSTICK_START  && bind.input < InputBindings::JOYSTICK_END)
		{
			return bind.input;
		}
	}
	return InputBindings::INVALID;
}

void UIScene_InputOptions::Enter(UIManager* manager)
{
	Find_Element<UISlider*>("deadzone_slider")->Set_Progress(*GameOptions::joystick_deadzone);
	Find_Element<UISlider*>("vibration_slider")->Set_Progress(*GameOptions::joystick_vibration);
	
	// Add columns.
	UIListView* listview = Find_Element<UIListView*>("bindings_list");
	listview->Clear_Columns();
	/*
	listview->Add_Column(S("#menu_options_input_column_1"), 0.45f, true, true);
	listview->Add_Column(S("#menu_options_input_column_2"), 0.275f, true, true);
	listview->Add_Column(S("#menu_options_input_column_3"), 0.275f, true, true);
	*/
	listview->Add_Column(S("#menu_options_input_column_1"), 0.45f);
	listview->Add_Column(S("#menu_options_input_column_2"), 0.275f);
	listview->Add_Column(S("#menu_options_input_column_3"), 0.275f);
	
	// Refresh items.
	Refresh_Items();
}	

void UIScene_InputOptions::Refresh_Items()
{
	UIListView* listview = Find_Element<UIListView*>("bindings_list");

	listview->Clear_Items();
	for (int i = (int)OutputBindings::GAME_START + 1; i < (int)OutputBindings::GAME_END; i++)
	{
		OutputBindings::Type binding		 = (OutputBindings::Type)i;
		InputBindings::Type keyboard_binding = Get_Keyboard_For_Bind(binding);
		InputBindings::Type joystick_binding = Get_Joystick_For_Bind(binding);

		const char* output_name   = "";
		std::string keyboard_name = "";
		std::string joystick_name = "";
		const char* output_id	  = "";
		
		OutputBindings::ToDescription(binding, output_name);
		OutputBindings::ToString(binding, output_id);

		if (keyboard_binding != InputBindings::INVALID)
		{
			InputBindings::ToDescription(keyboard_binding, keyboard_name, Input::Get()->Get_Active_Joystick_Icon_Set());
		}

		if (joystick_binding != InputBindings::INVALID)
		{
			InputBindings::ToDescription(joystick_binding, joystick_name, Input::Get()->Get_Active_Joystick_Icon_Set());
		}

		std::vector<std::string> columns;
		columns.push_back(output_name);
		columns.push_back(S(keyboard_name.c_str()));
		columns.push_back(S(joystick_name.c_str()));
		listview->Add_Item(columns);

		/*
		std::vector<std::string> columns;
		columns.push_back(output_name);
		columns.push_back(StringHelper::Format("[key=%s,keyboard]", output_id));
		columns.push_back(StringHelper::Format("[key=%s,joystick]", output_id));
		listview->Add_Item(columns);
		*/
	}
}

void UIScene_InputOptions::Exit(UIManager* manager)
{
	*GameOptions::joystick_deadzone = Find_Element<UISlider*>("deadzone_slider")->Get_Progress();
	*GameOptions::joystick_vibration = Find_Element<UISlider*>("vibration_slider")->Get_Progress();
}	

void UIScene_InputOptions::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_InputOptions::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_InputOptions::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ListView_Item_Click:
		{
			if (e.Source->Get_Name() == "bindings_list")
			{
				UIListView* listview = static_cast<UIListView*>(e.Source);
				OutputBindings::Type output_binding = static_cast<OutputBindings::Type>(listview->Get_Selected_Item_Index() + (OutputBindings::GAME_START + 1));

				manager->Go(UIAction::Push(new UIScene_KeyBindDialog(output_binding), new UIFadeInTransition()));
			}
		}
		break;
	case UIEventType::Element_Focus:
		{
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
