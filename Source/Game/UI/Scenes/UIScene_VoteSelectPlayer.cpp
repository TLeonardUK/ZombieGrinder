// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_VoteSelectPlayer.h"
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

UIScene_VoteSelectPlayer::UIScene_VoteSelectPlayer(Vote* vote)
	: m_vote(vote)
{
	Set_Layout("vote_select_player");	
}

const char* UIScene_VoteSelectPlayer::Get_Name()
{
	return "UIScene_VoteSelectPlayer";
}

bool UIScene_VoteSelectPlayer::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_VoteSelectPlayer::Should_Display_Cursor()
{
	return true;
}

bool UIScene_VoteSelectPlayer::Is_Focusable()
{
	return true;
}

void UIScene_VoteSelectPlayer::Enter(UIManager* manager)
{
	UIComboBox* box = Find_Element<UIComboBox*>("player_box");
	std::vector<NetUser*> user = GameNetManager::Get()->Get_Net_Users();

	box->Clear_Items();

	for (std::vector<NetUser*>::iterator iter = user.begin(); iter != user.end(); iter++)
	{
		NetUser* netuser = *iter;
		box->Add_Item(netuser->Get_Username().c_str(), reinterpret_cast<void*>(netuser->Get_Net_ID()));
	}
}	

void UIScene_VoteSelectPlayer::Exit(UIManager* manager)
{
}	

void UIScene_VoteSelectPlayer::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_VoteSelectPlayer::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_VoteSelectPlayer::Recieve_Event(UIManager* manager, UIEvent e)
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
				UIComboBox* box = Find_Element<UIComboBox*>("player_box");

				int net_id = (int)reinterpret_cast<long long>(box->Get_Selected_Item().MetaData1);

				m_vote->Set_User_ID(net_id);
				VoteManager::Get()->Call_Vote(m_vote);
			
				manager->Play_UI_Sound(UISoundType::Exit_SubMenu);
				manager->Go(UIAction::Pop(new UISlideInTransition(), "UIScene_Game"));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				SAFE_DELETE(m_vote);
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
		}
		break;
	}
}
