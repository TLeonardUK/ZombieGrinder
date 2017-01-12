// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Dialog.h"
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
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_Dialog::UIScene_Dialog(std::string value, std::string button_text, bool game_style, bool large_size, std::string override_layout)
	: m_text(value)
	, m_game_style(game_style)
{
	if (override_layout != "")
	{
		Set_Layout(override_layout.c_str());
	}
	else
	{
		if (game_style)
		{
			if (large_size)
			{
				Set_Layout("large_dialog");	
			}
			else
			{
				Set_Layout("dialog");	
			}
		}
		else
		{
			Set_Layout("editor_dialog");	
		}
	}

	if (button_text != "")
	{
		Find_Element<UILabel*>("back_button")->Set_Value(button_text.c_str());
	}
}

bool UIScene_Dialog::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_Dialog::Get_Name()
{
	return "UIScene_Dialog";
}

bool UIScene_Dialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_Dialog::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Dialog::Should_Display_Focus_Cursor()
{
	return m_game_style;
}

bool UIScene_Dialog::Is_Focusable()
{
	return true;
}

void UIScene_Dialog::Enter(UIManager* manager)
{
	Find_Element<UILabel*>("label")->Set_Value(S(m_text.c_str()));
}	

void UIScene_Dialog::Exit(UIManager* manager)
{
}	

void UIScene_Dialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Dialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Dialog::Recieve_Event(UIManager* manager, UIEvent e)
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
