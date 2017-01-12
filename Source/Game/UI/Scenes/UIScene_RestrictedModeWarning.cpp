// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_RestrictedModeWarning.h"
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

UIScene_RestrictedModeWarning::UIScene_RestrictedModeWarning(bool bInMode)
	: m_in_mode(bInMode)
{
	Set_Layout("restricted_mode_warning");
}

bool UIScene_RestrictedModeWarning::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_RestrictedModeWarning::Get_Name()
{
	return "UIScene_Dialog";
}

bool UIScene_RestrictedModeWarning::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_RestrictedModeWarning::Should_Display_Cursor()
{
	return true;
}

bool UIScene_RestrictedModeWarning::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene_RestrictedModeWarning::Is_Focusable()
{
	return true;
}

void UIScene_RestrictedModeWarning::Enter(UIManager* manager)
{
	std::string mods = "";
	std::vector<PackageFile*> files = Game::Get()->Get_Restricted_Packages();

	for (std::vector<PackageFile*>::iterator iter = files.begin(); iter != files.end(); iter++)
	{
		PackageFile* file = *iter;
		if (mods != "")
		{
			mods += "\n";
		}
		mods += "[c=255,100,100]" + file->Get_Name() + "[/c]";
	}

	if (m_in_mode)
	{
		std::string message = SF("#restricted_mode_warning", mods.c_str());
		Find_Element<UILabel*>("description_label")->Set_Value(message.c_str());
		Find_Element<UILabel*>("intro_label")->Set_Value(S("#restricted_mode_warning_title"));
	}
	else
	{
		std::string message = S("#mod_whitelisted_warning");
		Find_Element<UILabel*>("description_label")->Set_Value(message.c_str());
		Find_Element<UILabel*>("intro_label")->Set_Value(S("#mod_whitelisted_warning_title"));
	}
}

void UIScene_RestrictedModeWarning::Exit(UIManager* manager)
{
}

void UIScene_RestrictedModeWarning::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_RestrictedModeWarning::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_RestrictedModeWarning::Recieve_Event(UIManager* manager, UIEvent e)
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
