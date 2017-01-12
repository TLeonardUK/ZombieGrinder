// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Splash.h"
#include "Game/UI/Scenes/UIScene_Loading.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_MenuBackground.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Online/OnlinePlatform.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/GameOptions.h"
#include "Game/Runner/Game.h"

UIScene_Splash::UIScene_Splash()
	: m_splash_index(0)
	, m_timer(0)
	, m_splash_screen_interval(4000.0f)
	, m_timer_step(0)
	, m_showing_autosave(false)
{	
	// Skip premium-account splash screen.
#ifdef OPT_PREMIUM_ACCOUNTS
	if (OnlinePlatform::Get()->Check_Purchased(*GameOptions::premium_account_appid))
#endif
	{
		m_splash_index++;
	}

	DBG_LOG("Resetting");
	Set_Layout("splash");	
}

const char* UIScene_Splash::Get_Name()
{
	return "UIScene_Splash";
}

bool UIScene_Splash::Can_Accept_Invite()
{
	return false;
}

bool UIScene_Splash::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_Splash::Next_Splash()
{
	char name[256];
	sprintf(name, "screen_intro_%i", m_splash_index);

	AtlasFrame* frame = ResourceFactory::Get()->Get_Atlas_Frame(name);
	if (frame != NULL)
	{
		UIPanel* splash_image = Find_Element<UIPanel*>("splash_image");
		splash_image->Set_Background_Image(frame);
		m_splash_index++;
	}

	if (m_splash_index == 1)
	{
		m_splash_screen_interval = 5000.0f;
	}
	else
	{
		m_splash_screen_interval = 2500.0f;
	}

	m_timer = 0.0f;
	m_timer_step = 0;

	return (frame != NULL);
}

void UIScene_Splash::Enter(UIManager* manager)
{
	if (!Next_Splash())
	{
		manager->Go(UIAction::Replace(new UIScene_Loading()));
	}
}	

void UIScene_Splash::Exit(UIManager* manager)
{
}	

void UIScene_Splash::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Input* input = Input::Get();

	// Increment splash timer.
	m_timer += time.Get_Frame_Time();

	// Quick fade?
	if (input->Was_Pressed(OutputBindings::GUI_Select) && m_splash_index > 1)
	{
		manager->Go(UIAction::Replace(new UIScene_Loading(), new UIFadeTransition()));
		return;
	}
	
	// Auto load map.
	if (*GameOptions::editor == true || 
		*GameOptions::map != "" ||
		*EngineOptions::connect != 0)
	{
		manager->Go(UIAction::Replace(new UIScene_Loading(), new UIFadeTransition()));
		return;
	}

	// Fade in/out.
	float step_interval = 0.0f;
	if (m_timer_step == 0)
	{
		step_interval = 500.0f;
	}
	else if (m_timer_step == 3)
	{
		step_interval = 500.0f;
	}
	else
	{
		step_interval = (m_splash_screen_interval) - (500.0f * 2.0f);
	}

	float fade_delta = m_timer / step_interval;
	if (fade_delta >= 1.0f)
	{
		m_timer			= 0.0f;
		m_timer_step++;
		fade_delta		= 0.0f;
	}
	
	// Fade-In
	if (m_timer_step == 0)	
	{
		// Fade-delta is correct.
	}

	// Wait
	else if (m_timer_step == 1 || m_timer_step == 2)
	{
		fade_delta = 1.0f;
	}

	// Fade-Out
	else if (m_timer_step == 3)
	{
		fade_delta = 1.0f - fade_delta;
	}

	// Next!
	else
	{
		if (!Next_Splash())
		{
			if (!m_showing_autosave)
			{
				m_showing_autosave = true;
			}
			else
			{
				manager->Go(UIAction::Replace(new UIScene_Loading(), new UIFadeTransition()));
				return;
			}
		}
	}

	// Premium is displayed as text.
	UIPanel* splash_image = Find_Element<UIPanel*>("splash_image");
	UILabel* autosave_text = Find_Element<UILabel*>("autosave_text");

	bool bShowText = m_showing_autosave;
	if (m_showing_autosave)
	{
		autosave_text->Set_Value("#autosave_warning");
	}
	else if (m_splash_index == 1)
	{
		autosave_text->Set_Value("#premium_warning");
		bShowText = true;
	}

	if (bShowText)
	{
		autosave_text->Set_Color(Color(255.0f, 255.0f, 255.0f, fade_delta * 255.0f));
	}
	else
	{
		splash_image->Set_Background_Color(Color(255.0f, 255.0f, 255.0f, fade_delta * 255.0f));
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Splash::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}



