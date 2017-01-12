// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_OpenWorkshopDialog.h"
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
#include "Engine/Online/OnlinePlatform.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"

UIScene_OpenWorkshopDialog::UIScene_OpenWorkshopDialog(std::string text, u64 workshop_id)
	: m_workshop_id(workshop_id)
{
	Set_Layout("confirm_dialog");	

	Find_Element<UILabel*>("label")->Set_Value(text);
	Find_Element<UILabel*>("button_1")->Set_Value(S("#menu_open_workshop"));
	Find_Element<UILabel*>("button_2")->Set_Value(S("#menu_close"));
}

bool UIScene_OpenWorkshopDialog::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_OpenWorkshopDialog::Get_Name()
{
	return "UIScene_OpenWorkshopDialog";
}

bool UIScene_OpenWorkshopDialog::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_OpenWorkshopDialog::Should_Display_Cursor()
{
	return false;
}

bool UIScene_OpenWorkshopDialog::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene_OpenWorkshopDialog::Is_Focusable()
{
	return true;
}

bool UIScene_OpenWorkshopDialog::Should_Fade_Cursor()
{
	return true;
}

void UIScene_OpenWorkshopDialog::Enter(UIManager* manager)
{
}	

void UIScene_OpenWorkshopDialog::Exit(UIManager* manager)
{
}	

void UIScene_OpenWorkshopDialog::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_OpenWorkshopDialog::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_OpenWorkshopDialog::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "button_1")
			{
				OnlinePlatform::Get()->Show_Web_Browser(StringHelper::Format("http://steamcommunity.com/sharedfiles/filedetails/?id=%llu", m_workshop_id).c_str());
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
			else if (e.Source->Get_Name() == "button_2")
			{
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
		}
		break;
	}
}
