// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/UIScene_FileBrowser.h"
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
#include "Engine/UI/Elements/UIFileBrowser.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_FileBrowser::UIScene_FileBrowser(bool save, std::string path, std::string extension)
	: m_save(save)
	, m_path(path)
	, m_overwrite_path("")
	, m_selected_path("")
	, m_extension(extension)
	, m_closing(false)
{
	Set_Layout("file_browser");	
}

bool UIScene_FileBrowser::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_FileBrowser::Get_Name()
{
	return "UIScene_FileBrowser";
}

bool UIScene_FileBrowser::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_FileBrowser::Should_Display_Cursor()
{
	return true;
}
	
bool UIScene_FileBrowser::Should_Display_Focus_Cursor()
{
	return false;
}

bool UIScene_FileBrowser::Is_Focusable()
{
	return true;
}

bool UIScene_FileBrowser::Should_Fade_Cursor()
{
	return false;
}

void UIScene_FileBrowser::Enter(UIManager* manager)
{
	Find_Element<UIFileBrowser*>("browser")->Set_Extension_Filter(m_extension);
	Find_Element<UIFileBrowser*>("browser")->Set_Path(m_path);
	Find_Element<UIButton*>("ok_button")->Set_Value(m_save ? "Save File" : "Load File");
	Find_Element<UILabel*>("label")->Set_Value(m_save ? "Save File ..." : "Load File ...");
}	

void UIScene_FileBrowser::Exit(UIManager* manager)
{
}	

void UIScene_FileBrowser::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	std::string path = Find_Element<UIFileBrowser*>("browser")->Get_Path();
	bool selectable = (path != "");

	if (m_save == false)
	{
		if (!Platform::Get()->Is_File(path.c_str()))
		{
			selectable = false;
		}
	}

	Find_Element<UIButton*>("ok_button")->Set_Enabled(selectable);

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_FileBrowser::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_FileBrowser::Return_Path(UIManager* manager, std::string path)
{	
	if (m_closing)
	{
		return;
	}

	// Append extension if not provided.
	if (m_save == true && !Platform::Get()->Is_File(path.c_str()))
	{
		if (path != "" && path.find('.') == std::string::npos)
		{
			path = path + "." + m_extension;
		}
	}

	DBG_LOG("Selected file '%s'.", path.c_str());
	m_selected_path = path;

	manager->Get_Scene_Below(this)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));
	manager->Go(UIAction::Pop(new UIFadeOutTransition()));
	m_closing = true;
}

std::string UIScene_FileBrowser::Get_Selected_Path()
{
	return m_selected_path;
}

void UIScene_FileBrowser::Selected_Path(UIManager* manager)
{
	std::string path = Find_Element<UIFileBrowser*>("browser")->Get_Path();
	
	// Save logic.
	if (m_save == true)
	{
		if (Platform::Get()->Is_File(path.c_str()))
		{
			if (m_overwrite_path == "")
			{
				DBG_LOG("Asking for overwrite confirmation for '%s' ...", path.c_str());

				m_overwrite_path = path;
				manager->Go(UIAction::Push(new UIScene_ConfirmDialog(StringHelper::Format("Are you sure you wish to overwrite '%s'?", path.c_str()).c_str(), S("#menu_no"), S("#menu_yes"), false), new UIFadeInTransition()));				
			}
		}
		else
		{		
			Return_Path(manager, path);
		}
	}

	// Load logic.
	else
	{
		if (!Platform::Get()->Is_File(path.c_str()))
		{
			return;
		}

		Return_Path(manager, path);
	}
}

void UIScene_FileBrowser::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "ok_button")
			{
				Selected_Path(manager);
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				Return_Path(manager, "");
			}
		}
		break;
	case UIEventType::FileBrowser_Select:
		{
			if (e.Source->Get_Name() == "browser")
			{
				Selected_Path(manager);
			}
		}
		break;		
	case UIEventType::Dialog_Close:
		{
			if (dynamic_cast<UIScene_ConfirmDialog*>(e.Scene)->Get_Selected_Index() == 1)
			{
				Return_Path(manager, m_overwrite_path);
			}
			m_overwrite_path = "";
		}
		break;	
	}
}
