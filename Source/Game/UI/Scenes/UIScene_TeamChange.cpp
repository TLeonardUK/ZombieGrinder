// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_TeamChange.h"
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
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"
#include "Game/Runner/GameMode.h"
#include "Game/Runner/GameModeTeam.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"

UIScene_TeamChange::UIScene_TeamChange()
{
	Set_Layout("team_change");	
}

const char* UIScene_TeamChange::Get_Name()
{
	return "UIScene_TeamChange";
}

bool UIScene_TeamChange::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_TeamChange::Should_Display_Cursor()
{
	return true;
}

bool UIScene_TeamChange::Is_Focusable()
{
	return true;
}

void UIScene_TeamChange::Enter(UIManager* manager)
{	
	UIComboBox* box = Find_Element<UIComboBox*>("team_box");
	std::vector<GameModeTeam> teams = Game::Get()->Get_Game_Mode()->Get_Teams();

	box->Clear_Items();

	for (std::vector<GameModeTeam>::iterator iter = teams.begin(); iter != teams.end(); iter++)
	{
		GameModeTeam& team = *iter;
		box->Add_Item(StringHelper::Format("[c=%i,%i,%i,255]%s[/c]", team.Primary_Color.R, team.Primary_Color.G, team.Primary_Color.B, S(team.Name)).c_str());
	}
}	

void UIScene_TeamChange::Exit(UIManager* manager)
{
}	

void UIScene_TeamChange::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_TeamChange::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_TeamChange::Recieve_Event(UIManager* manager, UIEvent e)
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
				UIComboBox* box = Find_Element<UIComboBox*>("team_box");

				std::vector<GameModeTeam> teams = Game::Get()->Get_Game_Mode()->Get_Teams();
				int team_index = box->Get_Selected_Item_Index();

				if (team_index >= 0 && team_index < (int)teams.size())
				{
					OnlineUser* user = e.Source_User;
					if (user != NULL)
					{
						std::vector<GameNetUser*> users = GameNetManager::Get()->Get_Game_Net_Users();
						for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
						{
							GameNetUser* netuser = *iter;
							if (netuser->Get_Online_User() == user)
							{	
								GameNetManager::Get()->Game_Client()->Request_Team_Change(netuser, team_index);
								break;
							}
						}
					}
				}
				
				manager->Play_UI_Sound(UISoundType::Exit_SubMenu);
				manager->Go(UIAction::Pop(new UISlideInTransition(), "UIScene_Game"));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
		}
		break;
	}
}
