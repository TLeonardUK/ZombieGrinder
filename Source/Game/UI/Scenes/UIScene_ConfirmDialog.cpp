// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
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

UIScene_ConfirmDialog::UIScene_ConfirmDialog(std::string text, std::string opt_1, std::string opt_2, bool game_style)
	: m_selected_option(0)
	, m_game_style(game_style)
{
	if (game_style)
		Set_Layout("confirm_dialog");	
	else
		Set_Layout("editor_confirm_dialog");	

	Find_Element<UILabel*>("label")->Set_Value(text);
	Find_Element<UILabel*>("button_1")->Set_Value(opt_1);
	Find_Element<UILabel*>("button_2")->Set_Value(opt_2);
}

int UIScene_ConfirmDialog::Get_Selected_Index()
{
	return m_selected_option;
}

bool UIScene_ConfirmDialog::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_ConfirmDialog::Get_Name()
{
	return "UIScene_ConfirmDialog";
}

bool UIScene_ConfirmDialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_ConfirmDialog::Should_Display_Cursor()
{
	return true;
}
	
bool UIScene_ConfirmDialog::Should_Display_Focus_Cursor()
{
	return m_game_style;
}

bool UIScene_ConfirmDialog::Is_Focusable()
{
	return true;
}

bool UIScene_ConfirmDialog::Should_Fade_Cursor()
{
	return m_game_style;
}

void UIScene_ConfirmDialog::Enter(UIManager* manager)
{
}	

void UIScene_ConfirmDialog::Exit(UIManager* manager)
{
}	

void UIScene_ConfirmDialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ConfirmDialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ConfirmDialog::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "button_1")
			{
				m_selected_option = 0;
				manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
			else if (e.Source->Get_Name() == "button_2")
			{
				m_selected_option = 1;
				manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
		}
		break;
	}
}
