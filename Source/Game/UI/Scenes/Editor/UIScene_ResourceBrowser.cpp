// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/UIScene_ResourceBrowser.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
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
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UIResourceBrowser.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_ResourceBrowser::UIScene_ResourceBrowser(UIScene_ResourceBrowserMode::Type mode, std::string filter)
	: m_mode(mode)
	, m_closing(false)
	, m_selected_resource("")
	, m_filter(filter)
{
	Set_Layout("resource_browser");	
}

bool UIScene_ResourceBrowser::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_ResourceBrowser::Get_Name()
{
	return "UIScene_ResourceBrowser";
}

bool UIScene_ResourceBrowser::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_ResourceBrowser::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ResourceBrowser::Should_Display_Focus_Cursor()
{
	return false;
}

bool UIScene_ResourceBrowser::Is_Focusable()
{
	return true;
}

bool UIScene_ResourceBrowser::Should_Fade_Cursor()
{
	return false;
}

void UIScene_ResourceBrowser::Enter(UIManager* manager)
{
	switch (m_mode)
	{
	case UIScene_ResourceBrowserMode::Image:
		{
			Find_Element<UIResourceBrowser*>("browser")->Set_Mode(UIResourceBrowserMode::Image);
			break;
		}
	case UIScene_ResourceBrowserMode::Audio:
		{
			Find_Element<UIResourceBrowser*>("browser")->Set_Mode(UIResourceBrowserMode::Audio);
			break;
		}
	case UIScene_ResourceBrowserMode::Animation:
		{
			Find_Element<UIResourceBrowser*>("browser")->Set_Mode(UIResourceBrowserMode::Animation);
			break;
		}
	default:
		{
			DBG_ASSERT(false);
		}
	}

	Find_Element<UIResourceBrowser*>("browser")->Set_Filter(m_filter);
}	

void UIScene_ResourceBrowser::Exit(UIManager* manager)
{
}	

void UIScene_ResourceBrowser::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	std::string path = Find_Element<UIResourceBrowser*>("browser")->Get_Selected_Item();
	Find_Element<UIButton*>("ok_button")->Set_Enabled(path != "");

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ResourceBrowser::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

std::string UIScene_ResourceBrowser::Get_Selected_Resource()
{
	return m_selected_resource;
}

void UIScene_ResourceBrowser::Return_Resource(UIManager* manager, std::string path)
{
	if (m_closing)
	{
		return;
	}

	DBG_LOG("Selected resource '%s'.", path.c_str());
	m_selected_resource = path;

	manager->Get_Scene_Below(this)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));
	manager->Go(UIAction::Pop(new UIFadeOutTransition()));
	m_closing = true;
}

void UIScene_ResourceBrowser::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "ok_button")
			{
				Return_Resource(manager, Find_Element<UIResourceBrowser*>("browser")->Get_Selected_Item());
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				Return_Resource(manager, "");
			}
		}
		break;
	case UIEventType::ResourceBrowser_Select:
		{
			Return_Resource(manager, Find_Element<UIResourceBrowser*>("browser")->Get_Selected_Item());
		}
		break;
	}
}
