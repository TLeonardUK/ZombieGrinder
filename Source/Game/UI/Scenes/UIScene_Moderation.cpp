// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Moderation.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Online/StatisticsManager.h"
#include "Engine/Online/OnlineAchievements.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Moderation/ReportManager.h"

UIScene_Moderation::UIScene_Moderation()
{
	Set_Layout("moderation");
	Update_Labels();
}

const char* UIScene_Moderation::Get_Name()
{
	return "UIScene_Moderation";
}

bool UIScene_Moderation::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Moderation::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Moderation::Is_Focusable()
{
	return true;
}

void UIScene_Moderation::Enter(UIManager* manager)
{
	if (!(*GameOptions::mod_message_shown))
	{
		manager->Go(UIAction::Push(new UIScene_Dialog(S("#moderation_notice"), S("#menu_continue"), true, false, "game_mod_warning"), new UIFadeInTransition()));
		*GameOptions::mod_message_shown = true;
	}
}

void UIScene_Moderation::Exit(UIManager* manager)
{
}

void UIScene_Moderation::Update_Labels()
{
	CheatReportCase& report_case = ReportManager::Get()->Get_Queue()[0];

	UILabel* case_id = Find_Element<UILabel*>("case_id");
	UILabel* case_length = Find_Element<UILabel*>("case_length");
	UILabel* reported_user = Find_Element<UILabel*>("reported_user");
	UILabel* reported_reasons = Find_Element<UILabel*>("reported_reasons");
	UILabel* download_progress = Find_Element<UILabel*>("download_progress");
	UIButton* begin_button = Find_Element<UIButton*>("begin_button");

	std::string cheat_types = "";
	int val = (int)report_case.Category;

	if ((val & (int)CheatCategory::Exploit) != 0)
	{
		cheat_types += S("#ban_notice_exploit");
	}
	if ((val & (int)CheatCategory::Cheat) != 0)
	{
		if (cheat_types != "")
		{
			cheat_types += S("#ban_notice_type_seperator");
		}
		cheat_types += S("#ban_notice_cheat");
	}
	if ((val & (int)CheatCategory::Abuse) != 0)
	{
		if (cheat_types != "")
		{
			cheat_types += S("#ban_notice_type_seperator");
		}
		cheat_types += S("#ban_notice_abuse");
	}
	if ((val & (int)CheatCategory::Grief) != 0)
	{
		if (cheat_types != "")
		{
			cheat_types += S("#ban_notice_type_seperator");
		}
		cheat_types += S("#ban_notice_grief");
	}

	int minutes = report_case.Timeframe / 60;
	case_id->Set_Value(SF("#moderation_case_id", report_case.Judgement_ID));
	case_length->Set_Value(SF("#moderation_case_timeframe", minutes));
	reported_user->Set_Value(SF("#moderation_reported_user", report_case.Reported_Username.c_str()));
	reported_reasons->Set_Value(SF("#moderation_reported_for", cheat_types.c_str()));
	begin_button->Set_Enabled(false);

	if (report_case.Download_State == CheatDownloadState::Downloading)
	{
		download_progress->Set_Value(SF("#moderation_downloading", report_case.Download_Progress * 100.0f));
	}
	else if (report_case.Download_State == CheatDownloadState::Failed)
	{
		download_progress->Set_Value(S("#moderation_download_failed"));
		begin_button->Set_Value(S("#moderation_download_retry"));
		begin_button->Set_Enabled(true);
	}
	else if (report_case.Download_State == CheatDownloadState::Success)
	{
		download_progress->Set_Value(S("#moderation_download_success"));
		begin_button->Set_Value(S("#menu_begin"));
		begin_button->Set_Enabled(true);
	}
	else if (report_case.Download_State == CheatDownloadState::Idle)
	{
		ReportManager::Get()->Download_Case(report_case.Judgement_ID);
	}
}

void UIScene_Moderation::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Update_Labels();
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Moderation::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Moderation::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
}

void UIScene_Moderation::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
	{
		if (e.Source->Get_Name() == "back_button")
		{
			manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
		}
		else if (e.Source->Get_Name() == "begin_button")
		{
			CheatReportCase& report_case = ReportManager::Get()->Get_Queue()[0];
			UIButton* begin_button = Find_Element<UIButton*>("begin_button");

			if (report_case.Download_State == CheatDownloadState::Success)
			{
				Demo* demo = new Demo();
				if (demo->Serialize(report_case.Download_Data, false))
				{
					Game::Get()->Activate_Demo(demo);
				}
				else
				{
					SAFE_DELETE(demo);
				}
			}
			else
			{
				ReportManager::Get()->Download_Case(report_case.Judgement_ID);
				begin_button->Set_Enabled(false);
			}
		}
		break;
	}
	}
}
