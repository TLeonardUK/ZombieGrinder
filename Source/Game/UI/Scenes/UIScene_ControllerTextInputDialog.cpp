// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ControllerTextInputDialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/Input/Steam/Steam_JoystickState.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_ControllerTextInputDialog::UIScene_ControllerTextInputDialog(std::string value, std::string label_text, int max_length, JoystickState* joystick)
	: m_text(value)
	, m_uppercase(true)
	, m_max_length(max_length)
	, m_joystick(joystick)
	, m_using_native_dialog(false)
{
	if (m_joystick != NULL && m_joystick->ShowInputDialog(m_text, label_text, max_length))
	{
		m_using_native_dialog = true;
	}
	else
	{
		Set_Layout("controller_text_input_dialog");

		if (label_text != "")
		{
			Find_Element<UILabel*>("prompt_label")->Set_Value(label_text.c_str());
		}
	}
}

bool UIScene_ControllerTextInputDialog::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_ControllerTextInputDialog::Get_Name()
{
	return "UIScene_ControllerTextInputDialog";
}

bool UIScene_ControllerTextInputDialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_ControllerTextInputDialog::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ControllerTextInputDialog::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene_ControllerTextInputDialog::Is_Focusable()
{
	return true;
}

void UIScene_ControllerTextInputDialog::Enter(UIManager* manager)
{
	if (!m_using_native_dialog)
	{
		manager->Focus(Find_Element<UIButton*>("0_button"));
		Find_Element<UITextBox*>("textbox")->Is_Always_Focused = true;
		Find_Element<UITextBox*>("textbox")->Set_Max_Length(m_max_length);
		Find_Element<UITextBox*>("textbox")->Set_Value(S(m_text.c_str()));
	}
}	

void UIScene_ControllerTextInputDialog::Exit(UIManager* manager)
{
}

std::string UIScene_ControllerTextInputDialog::Get_Input()
{
	if (m_using_native_dialog)
	{
		return m_joystick->GetInputDialogResult();
	}
	else
	{
		return Find_Element<UITextBox*>("textbox")->Get_Value();
	}
}
                                                                                                                                              
void UIScene_ControllerTextInputDialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	if (!m_using_native_dialog)
	{
		for (int i = 0; i < MAX_INPUT_SOURCES; i++)
		{
			Input* input = Input::Get(i);
			if (input->Was_Pressed(InputBindings::Joystick_Button30)) // Y Button
			{
				SwitchCase();
				manager->Play_UI_Sound(UISoundType::Select);
				break;
			}
		}
	}
	else
	{
		if (!m_joystick->IsInputDialogActive())
		{
			manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));
			manager->Go(UIAction::Pop(new UIFadeOutTransition()));
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ControllerTextInputDialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ControllerTextInputDialog::SwitchCase()
{
	m_uppercase = !m_uppercase;

	std::vector<UIElement*> elements = Get_Elements();
	for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
	{
		UIButton* elm = dynamic_cast<UIButton*>(*iter);
		if (elm != NULL &&
			elm->Get_Name() != "case_button" &&
			elm->Get_Name() != "backspace_button" &&
			elm->Get_Name() != "space_button" &&
			elm->Get_Name() != "back_button")
		{
			if (m_uppercase)
			{					
				elm->Set_Value(StringHelper::Uppercase(elm->Get_Value().c_str()));
			}
			else
			{
				elm->Set_Value(StringHelper::Lowercase(elm->Get_Value().c_str()));
			}
		}
	}
}

void UIScene_ControllerTextInputDialog::Recieve_Event(UIManager* manager, UIEvent e)
{
	if (!m_using_native_dialog)
	{
		UITextBox* box = Find_Element<UITextBox*>("textbox");
		std::string text_value = box->Get_Value();

		switch (e.Type)
		{
		case UIEventType::Button_Click:
			{	
				if (e.Source->Get_Name() == "case_button")
				{
					SwitchCase();
				}
				else if (e.Source->Get_Name() == "backspace_button")
				{
					if (text_value.size() > 0)
					{
						box->Set_Value(text_value.substr(0, text_value.size() - 1));
					}
				}
				else if (e.Source->Get_Name() == "space_button")
				{
					if ((int)text_value.size() < m_max_length)
					{
						box->Set_Value(text_value + " ");
					}
				}
				else if (e.Source->Get_Name() == "back_button")
				{
					manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));
					manager->Go(UIAction::Pop(new UIFadeOutTransition()));
				}
				else
				{
					if ((int)text_value.size() < m_max_length)
					{
						box->Set_Value(text_value + e.Source->Get_Value());
					}
				}
			}
			break;
		}
	}
}
