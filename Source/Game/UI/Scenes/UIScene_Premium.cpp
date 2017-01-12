// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Premium.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Online/OnlinePlatform.h"

#include "Game/Runner/GameOptions.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/UI/Transitions/UIFadeTransition.h"

#include "Engine/Engine/GameEngine.h"
#include "Game/Runner/Game.h"

UIScene_Premium::UIScene_Premium()
	: m_dialog_open(false)
{
	Set_Layout("premium");
}

UIScene_Premium::~UIScene_Premium()
{
}

const char* UIScene_Premium::Get_Name()
{
	return "UIScene_Premium";
}

bool UIScene_Premium::Should_Render_Lower_Scenes()
{
	return false;
}

UIScene* UIScene_Premium::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

bool UIScene_Premium::Is_Focusable()
{
	return true;
}

bool UIScene_Premium::Should_Display_Cursor()
{
	return true;
}

void UIScene_Premium::Enter(UIManager* manager)
{
}	

void UIScene_Premium::Exit(UIManager* manager)
{
}	

void UIScene_Premium::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	if (m_dialog_open == true)
	{
		m_dialog_close_delay += time.Get_Frame_Time();

		if (!OnlinePlatform::Get()->Is_Dialog_Open() && m_dialog_close_delay > DIALOG_CLOSE_DELAY)
		{
			m_dialog_open = false;

#ifdef OPT_PREMIUM_ACCOUNTS
			if (OnlinePlatform::Get()->Check_Purchased(*GameOptions::premium_account_appid))
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
				return;
			}
#endif
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Premium::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Premium::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "purchase_button")
			{
#ifdef OPT_PREMIUM_ACCOUNTS
				OnlinePlatform::Get()->Show_Purchase_Dialog(*GameOptions::premium_account_appid);
#endif
				m_dialog_close_delay = 0.0f;
				m_dialog_open = true;
			}
			else if (e.Source->Get_Name() == "quit_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}



