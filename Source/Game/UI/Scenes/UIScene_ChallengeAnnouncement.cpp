// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ChallengeAnnouncement.h"
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

UIScene_ChallengeAnnouncement::UIScene_ChallengeAnnouncement(ChallengeAnnouncement announcement)
	: m_announcement(announcement)
{
	Set_Layout("challenge_announcement");
}

bool UIScene_ChallengeAnnouncement::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_ChallengeAnnouncement::Get_Name()
{
	return "UIScene_ChallengeAnnouncement";
}

bool UIScene_ChallengeAnnouncement::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_ChallengeAnnouncement::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ChallengeAnnouncement::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene_ChallengeAnnouncement::Is_Focusable()
{
	return true;
}

void UIScene_ChallengeAnnouncement::Enter(UIManager* manager)
{
	Find_Element<UILabel*>("name_label")->Set_Value(m_announcement.name.c_str());
	Find_Element<UILabel*>("description_label")->Set_Value(m_announcement.description.c_str());
	Find_Element<UIPanel*>("image_panel")->Set_Background_Image(ResourceFactory::Get()->Get_Atlas_Frame(m_announcement.image.c_str()));
}

void UIScene_ChallengeAnnouncement::Exit(UIManager* manager)
{
}

void UIScene_ChallengeAnnouncement::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ChallengeAnnouncement::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ChallengeAnnouncement::Recieve_Event(UIManager* manager, UIEvent e)
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
