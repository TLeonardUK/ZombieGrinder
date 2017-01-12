// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Vote.h"
#include "Game/UI/Scenes/UIScene_VoteSelectPlayer.h"
#include "Game/UI/Scenes/UIScene_VoteSelectMap.h"
#include "Game/UI/Scenes/UIScene_ReportPlayer.h"

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
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Game/Online/ChallengeManager.h"

#include "Game/Game/VoteManager.h"
#include "Game/Game/Votes/RestartVote.h"
#include "Game/Game/Votes/EditModeVote.h"
#include "Game/Game/Votes/KickVote.h"
#include "Game/Game/Votes/ChangeMapVote.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Moderation/ReportManager.h"

UIScene_Vote::UIScene_Vote()
{
	Set_Layout("vote");	
}

const char* UIScene_Vote::Get_Name()
{
	return "UIScene_Vote";
}

bool UIScene_Vote::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Vote::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Vote::Is_Focusable()
{
	return true;
}

void UIScene_Vote::Enter(UIManager* manager)
{
	UIButton* button = Find_Element<UIButton*>("edit_mode_button");

	if (GameNetManager::Get()->Game_Client()->Get_Server_State().In_Editor_Mode == true)
	{
		button->Set_Value(S("#sub_menu_vote_exit_edit_mode"));
	}
	else
	{
		button->Set_Value(S("#sub_menu_vote_enter_edit_mode"));
	}

	button->Set_Enabled(!ChallengeManager::Get()->In_Custom_Map());

	UIButton* change_button = Find_Element<UIButton*>("change_map_button");
	change_button->Set_Enabled(!ChallengeManager::Get()->In_Custom_Map());

	UIButton* kick_button = Find_Element<UIButton*>("kick_player_button");
	kick_button->Set_Enabled(!ChallengeManager::Get()->In_Custom_Map());

	UIButton* report_button = Find_Element<UIButton*>("report_button");
	report_button->Set_Enabled(ReportManager::Get()->Is_Available() && DemoManager::Get()->Get_Recent_Players().size() > 0);	
}	

void UIScene_Vote::Exit(UIManager* manager)
{
}	

void UIScene_Vote::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Vote::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Vote::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Element_Focus:
		{
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "kick_player_button")
			{
				manager->Go(UIAction::Push(new UIScene_VoteSelectPlayer(new KickVote()), new UISlideInTopLevelTransition()));		
			}
			else if (e.Source->Get_Name() == "restart_map_button")
			{
				VoteManager::Get()->Call_Vote(new RestartVote());

				manager->Play_UI_Sound(UISoundType::Exit_SubMenu);
				manager->Go(UIAction::Pop(new UISlideInTransition(), "UIScene_Game"));
			}
			else if (e.Source->Get_Name() == "edit_mode_button")
			{
				VoteManager::Get()->Call_Vote(new EditModeVote());

				manager->Play_UI_Sound(UISoundType::Exit_SubMenu);
				manager->Go(UIAction::Pop(new UISlideInTransition(), "UIScene_Game"));
			}
			else if (e.Source->Get_Name() == "change_map_button")
			{
				manager->Go(UIAction::Push(new UIScene_VoteSelectMap(new ChangeMapVote()), new UISlideInTopLevelTransition()));	
			}
			else if (e.Source->Get_Name() == "report_button")
			{
				manager->Go(UIAction::Push(new UIScene_ReportPlayer(), new UISlideInTopLevelTransition()));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
		}
		break;
	}
}
