// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ReportPlayer.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
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
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Game/Game/VoteManager.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"

#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetManager.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Moderation/ReportManager.h"

UIScene_ReportPlayer::UIScene_ReportPlayer()
	: m_closing(false)
{
	Set_Layout("report_player");
}

const char* UIScene_ReportPlayer::Get_Name()
{
	return "UIScene_ReportPlayer";
}

bool UIScene_ReportPlayer::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_ReportPlayer::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ReportPlayer::Is_Focusable()
{
	return true;
}

UIScene* UIScene_ReportPlayer::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_SubMenuBackground");
}

void UIScene_ReportPlayer::Enter(UIManager* manager)
{
	if (m_closing)
	{
		manager->Go(UIAction::Pop(new UISlideInTransition(), "UIScene_Game"));
	}
	else
	{
		UIComboBox* timeline_box = Find_Element<UIComboBox*>("timeline_box");
		timeline_box->Add_Item("#menu_report_player_timeline_5min",  (void*)(5 * 60));
		timeline_box->Add_Item("#menu_report_player_timeline_10min", (void*)(10 * 60));
		timeline_box->Add_Item("#menu_report_player_timeline_15min", (void*)(15 * 60));
		//timeline_box->Add_Item("#menu_report_player_timeline_30min", (void*)(30 * 60));
		//timeline_box->Add_Item("#menu_report_player_timeline_1hr",   (void*)(60 * 60));

		m_recent_players = DemoManager::Get()->Get_Recent_Players();

		UIComboBox* player_box = Find_Element<UIComboBox*>("player_box");
		for (unsigned int i = 0; i < m_recent_players.size(); i++)
		{
			DemoPlayer& player = m_recent_players[i];
			player_box->Add_Item(player.Username.c_str(), NULL);
		}

		UIButton* ok_button = Find_Element<UIButton*>("ok_button");
		ok_button->Set_Enabled(false);
	}
}

void UIScene_ReportPlayer::Exit(UIManager* manager)
{
}

void UIScene_ReportPlayer::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);

	// Check valid state to allow for report.
	UITextBox* description_box = Find_Element<UITextBox*>("description_box");
	UICheckBox* exploit_box = Find_Element<UICheckBox*>("exploit_checkbox");
	UICheckBox* cheat_box = Find_Element<UICheckBox*>("cheat_checkbox");
	UICheckBox* abuse_box = Find_Element<UICheckBox*>("abuse_checkbox");
	UICheckBox* grief_box = Find_Element<UICheckBox*>("grief_checkbox");
	UIButton* ok_button = Find_Element<UIButton*>("ok_button");
	bool bValid = true;

	if (StringHelper::Trim(description_box->Get_Value().c_str()).size() < 8)
	{
		bValid = false;
	}
	else if (!exploit_box->Get_Checked() &&
		!cheat_box->Get_Checked() &&
		!abuse_box->Get_Checked() &&
		!grief_box->Get_Checked())
	{
		bValid = false;
	}

	ok_button->Set_Enabled(bValid);
}

void UIScene_ReportPlayer::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ReportPlayer::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Element_Focus:
	{
	}
	break;
	case UIEventType::Button_Click:
	{
		if (e.Source->Get_Name() == "ok_button")
		{
			UITextBox* description_box = Find_Element<UITextBox*>("description_box");
			UICheckBox* exploit_box = Find_Element<UICheckBox*>("exploit_checkbox");
			UICheckBox* cheat_box = Find_Element<UICheckBox*>("cheat_checkbox");
			UICheckBox* abuse_box = Find_Element<UICheckBox*>("abuse_checkbox");
			UICheckBox* grief_box = Find_Element<UICheckBox*>("grief_checkbox");
			UIComboBox* timeline_box = Find_Element<UIComboBox*>("timeline_box");
			UIComboBox* player_box = Find_Element<UIComboBox*>("player_box");

			float time_frame = 0.0f;

			CheatReport report;
			report.Description		 = description_box->Get_Value();
			report.Category			 = CheatCategory::None;
			report.Timeframe		 = (int)timeline_box->Get_Selected_Item().MetaData1;
			report.Reported_User	 = m_recent_players[player_box->Get_Selected_Item_Index()].Platform_ID;
			report.Reported_Username = m_recent_players[player_box->Get_Selected_Item_Index()].Username;

			Demo* demo = DemoManager::Get()->Get_Demo((float)report.Timeframe);
			demo->Serialize(report.Demo, true);
			SAFE_DELETE(demo);

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

			ReportManager::Get()->Post_Report(report);

			manager->Play_UI_Sound(UISoundType::Exit_SubMenu);
			manager->Go(UIAction::Push(new UIScene_Dialog(S("#menu_report_player_confirm_dialog"), S("#menu_confirm")), new UIFadeInTransition()));
			m_closing = true;
		}
		else if (e.Source->Get_Name() == "back_button")
		{
			manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
		}
	}
	break;
	}
}
