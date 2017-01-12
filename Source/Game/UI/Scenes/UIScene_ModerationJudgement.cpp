// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ModerationJudgement.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"

#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
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

UIScene_ModerationJudgement::UIScene_ModerationJudgement()
{
	Set_Layout("moderation_judgement");
	Update_Labels();
}

const char* UIScene_ModerationJudgement::Get_Name()
{
	return "UIScene_ModerationJudgement";
}

bool UIScene_ModerationJudgement::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_ModerationJudgement::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ModerationJudgement::Is_Focusable()
{
	return true;
}

UIScene* UIScene_ModerationJudgement::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}

void UIScene_ModerationJudgement::Enter(UIManager* manager)
{
}

void UIScene_ModerationJudgement::Exit(UIManager* manager)
{
}

void UIScene_ModerationJudgement::Update_Labels()
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

	bool bIsValid = false;
	if (Find_Element<UICheckBox*>("inconclusive_checkbox")->Get_Checked() ||
		Find_Element<UICheckBox*>("innocent_checkbox")->Get_Checked())
	{
		bIsValid = true;
	}
	else if (Find_Element<UICheckBox*>("guilty_checkbox")->Get_Checked())
	{
		if (Find_Element<UICheckBox*>("exploit_checkbox")->Get_Checked() ||
			Find_Element<UICheckBox*>("cheat_checkbox")->Get_Checked() ||
			Find_Element<UICheckBox*>("abuse_checkbox")->Get_Checked() ||
			Find_Element<UICheckBox*>("grief_checkbox")->Get_Checked())
		{
			bIsValid = true;
		}
	}

	begin_button->Set_Enabled(bIsValid);
}

void UIScene_ModerationJudgement::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Update_Labels();
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ModerationJudgement::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ModerationJudgement::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
}

void UIScene_ModerationJudgement::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
	{
		UICheckBox* exploit_box = Find_Element<UICheckBox*>("exploit_checkbox");
		UICheckBox* cheat_box = Find_Element<UICheckBox*>("cheat_checkbox");
		UICheckBox* abuse_box = Find_Element<UICheckBox*>("abuse_checkbox");
		UICheckBox* grief_box = Find_Element<UICheckBox*>("grief_checkbox");

		UICheckBox* guilty_checkbox = Find_Element<UICheckBox*>("guilty_checkbox");
		UICheckBox* inconclusive_checkbox = Find_Element<UICheckBox*>("inconclusive_checkbox");
		UICheckBox* innocent_checkbox = Find_Element<UICheckBox*>("innocent_checkbox");

		CheatReportCase& report_case = ReportManager::Get()->Get_Queue()[0];

		CheatReportJudgement report;
		report.Judgement_ID = report_case.Judgement_ID;
		report.Category = CheatCategory::None;
		report.Result = CheatJudgementResult::Inconclusive;

		if (guilty_checkbox->Get_Checked())
		{
			report.Result = CheatJudgementResult::Guilty;
			if (exploit_box->Get_Checked())
			{
				report.Category = (CheatCategory::Type)((int)report.Category | (int)CheatCategory::Exploit);
			}
			if (cheat_box->Get_Checked())
			{
				report.Category = (CheatCategory::Type)((int)report.Category | (int)CheatCategory::Cheat);
			}
			if (abuse_box->Get_Checked())
			{
				report.Category = (CheatCategory::Type)((int)report.Category | (int)CheatCategory::Abuse);
			}
			if (grief_box->Get_Checked())
			{
				report.Category = (CheatCategory::Type)((int)report.Category | (int)CheatCategory::Grief);
			}
		}
		else if (inconclusive_checkbox->Get_Checked())
		{
			report.Result = CheatJudgementResult::Inconclusive;
		}
		else if (innocent_checkbox->Get_Checked())
		{
			report.Result = CheatJudgementResult::Innocent;
		}

		ReportManager::Get()->Post_Judgement(report);

		manager->Play_UI_Sound(UISoundType::Exit_SubMenu);
		manager->Go(UIAction::Pop(NULL, "UIScene_GameSetupMenu"));
		manager->Go(UIAction::Push(new UIScene_Dialog(S("#moderation_judgement_upload_message"), S("#menu_confirm")), new UIFadeInTransition()));

		break;
	}
	case UIEventType::CheckBox_Click:
	{
		if (e.Source->Get_Name() == "guilty_checkbox" || 
			e.Source->Get_Name() == "exploit_checkbox" ||
			e.Source->Get_Name() == "cheat_checkbox" ||
			e.Source->Get_Name() == "abuse_checkbox" ||
			e.Source->Get_Name() == "grief_checkbox")
		{
			Find_Element<UICheckBox*>("exploit_checkbox")->Set_Enabled(true);
			Find_Element<UICheckBox*>("cheat_checkbox")->Set_Enabled(true);
			Find_Element<UICheckBox*>("abuse_checkbox")->Set_Enabled(true);
			Find_Element<UICheckBox*>("grief_checkbox")->Set_Enabled(true);

			Find_Element<UICheckBox*>("guilty_checkbox")->Set_Checked(true);
			Find_Element<UICheckBox*>("inconclusive_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("innocent_checkbox")->Set_Checked(false);
		}
		else if (e.Source->Get_Name() == "inconclusive_checkbox")
		{
			Find_Element<UICheckBox*>("exploit_checkbox")->Set_Enabled(false);
			Find_Element<UICheckBox*>("cheat_checkbox")->Set_Enabled(false);
			Find_Element<UICheckBox*>("abuse_checkbox")->Set_Enabled(false);
			Find_Element<UICheckBox*>("grief_checkbox")->Set_Enabled(false);
			Find_Element<UICheckBox*>("exploit_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("cheat_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("abuse_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("grief_checkbox")->Set_Checked(false);

			Find_Element<UICheckBox*>("guilty_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("inconclusive_checkbox")->Set_Checked(true);
			Find_Element<UICheckBox*>("innocent_checkbox")->Set_Checked(false);
		}
		else if (e.Source->Get_Name() == "innocent_checkbox")
		{
			Find_Element<UICheckBox*>("exploit_checkbox")->Set_Enabled(false);
			Find_Element<UICheckBox*>("cheat_checkbox")->Set_Enabled(false);
			Find_Element<UICheckBox*>("abuse_checkbox")->Set_Enabled(false);
			Find_Element<UICheckBox*>("grief_checkbox")->Set_Enabled(false);
			Find_Element<UICheckBox*>("exploit_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("cheat_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("abuse_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("grief_checkbox")->Set_Checked(false);

			Find_Element<UICheckBox*>("guilty_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("inconclusive_checkbox")->Set_Checked(false);
			Find_Element<UICheckBox*>("innocent_checkbox")->Set_Checked(true);
		}
		break;
	}
	}
}
