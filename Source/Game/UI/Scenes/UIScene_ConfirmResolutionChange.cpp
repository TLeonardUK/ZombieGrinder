// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ConfirmResolutionChange.h"
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

UIScene_ConfirmResolutionChange::UIScene_ConfirmResolutionChange(int old_width, int old_height, int old_mode, int old_hertz)
	: m_revert_timer(REVERT_TIME_INTERVAL)
	, m_old_width(old_width)
	, m_old_height(old_height)
	, m_old_mode(old_mode)
	, m_old_hertz(old_hertz)
{
	Set_Layout("confirm_resolution_dialog");	
}

bool UIScene_ConfirmResolutionChange::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_ConfirmResolutionChange::Get_Name()
{
	return "UIScene_ConfirmResolutionChange";
}

bool UIScene_ConfirmResolutionChange::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_ConfirmResolutionChange::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ConfirmResolutionChange::Is_Focusable()
{
	return true;
}

void UIScene_ConfirmResolutionChange::Enter(UIManager* manager)
{
	Find_Element<UILabel*>("label")->Set_Value(StringHelper::Format(S("#menu_options_resolution_change_confirm"), (int)(m_revert_timer / 1000)));
}	

void UIScene_ConfirmResolutionChange::Exit(UIManager* manager)
{
}	

void UIScene_ConfirmResolutionChange::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Input* input = Input::Get(0);

	if (m_revert_timer <= 0)
	{			
		*EngineOptions::render_display_width	= m_old_width;
		*EngineOptions::render_display_height	= m_old_height;
		*EngineOptions::render_display_mode		= m_old_mode;
		*EngineOptions::render_display_hertz	= m_old_hertz;
		Game::Get()->Apply_Config();

		manager->Go(UIAction::Pop(new UIFadeOutTransition()));
	}
	else
	{		
		m_revert_timer -= time.Get_Frame_Time();

		Find_Element<UILabel*>("label")->Set_Value(StringHelper::Format(S("#menu_options_resolution_change_confirm"), (int)(m_revert_timer / 1000)));
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ConfirmResolutionChange::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ConfirmResolutionChange::Recieve_Event(UIManager* manager, UIEvent e)
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
