// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_SubMenu.h"
#include "Game/UI/Scenes/UIScene_Inventory.h"
#include "Game/UI/Scenes/UIScene_Achievements.h"
#include "Game/UI/Scenes/UIScene_Leaderboards.h"
#include "Game/UI/Scenes/UIScene_Options.h"
#include "Game/UI/Scenes/UIScene_Vote.h"
#include "Game/UI/Scenes/UIScene_TeamChange.h"
#include "Game/UI/Scenes/UIScene_Statistics.h"
#include "Game/UI/Scenes/UIScene_GlobalChat.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_SubMenuBackground.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Transitions/UIFadeInToNewTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UIButton.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/UIManager.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"
#include "Game/Runner/GameModeTeam.h"

#include "Engine/Online/OnlineUser.h"
#include "Game/Profile/ProfileManager.h"

UIScene_SubMenu::UIScene_SubMenu()
	: m_should_sync_profile(false)
	, m_leaving(false)
{
	Set_Layout("SubMenu");

	Game::Get()->Set_Sub_Menu_Open(true);
}

UIScene_SubMenu::~UIScene_SubMenu()
{
	Game::Get()->Set_Sub_Menu_Open(false);

	if (m_leaving == true)
	{
		NetManager::Get()->Leave_Network_Game();
	}
}

bool UIScene_SubMenu::Should_Tick_When_Not_Top()
{
	return true;
}

const char* UIScene_SubMenu::Get_Name()
{
	return "UIScene_SubMenu";
}

bool UIScene_SubMenu::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_SubMenu::Should_Render_Lower_Scenes_Background()
{
	return false;
}

UIScene* UIScene_SubMenu::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_SubMenuBackground");
}	

bool UIScene_SubMenu::Is_Focusable()
{
	return true;
}

bool UIScene_SubMenu::Should_Display_Cursor()
{
	return true;
}

void UIScene_SubMenu::Enter(UIManager* manager)
{	
	std::vector<GameModeTeam> teams = Game::Get()->Get_Game_Mode()->Get_Teams();
	Find_Element<UIButton*>("team_button")->Set_Enabled(teams.size() > 1);

	if (DemoManager::Get()->Is_Playing())
	{
		Find_Element<UIButton*>("inventory_button")->Set_Enabled(false);
		Find_Element<UIButton*>("vote_button")->Set_Enabled(false);
		Find_Element<UIButton*>("team_button")->Set_Enabled(false);
		Find_Element<UIButton*>("global_chat_button")->Set_Enabled(false);
	}
}	

void UIScene_SubMenu::Exit(UIManager* manager)
{
}	

void UIScene_SubMenu::Close(UIManager* manager)
{
	manager->Play_UI_Sound(UISoundType::Exit_SubMenu);
	manager->Go(UIAction::Pop(new UISlideInTransition()));

	if (m_should_sync_profile)
	{
		NetManager* netmanager = NetManager::Get();
		GameNetClient* client = static_cast<GameNetClient*>(netmanager->Client());
		if (client != NULL)
			client->Send_Profile_Update();

		m_should_sync_profile = false;
	}
}

void UIScene_SubMenu::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Close sub menu?
	if (manager->Get_Top_Scene_Index() == scene_index)
	{
		if (Input::Get()->Was_Pressed_By_Any(OutputBindings::GUI_Back) ||
			Input::Get()->Was_Pressed_By_Any(OutputBindings::Escape))
		{
			Close(manager);
			return;
		}
	}
	
	// Check what our child is.
	bool show_bg_titles = true;
	if (manager->Get_Top_Scene_Index() > scene_index)
	{
		UIScene* child = manager->Get_Scene(scene_index + 1);
		if (dynamic_cast<UIScene_Vote*>(child) == NULL &&
			dynamic_cast<UIScene_TeamChange*>(child) == NULL)
		{
			show_bg_titles = false;
		}
	}
	dynamic_cast<UIScene_SubMenuBackground*>(manager->Retrieve_Persistent_Scene("UIScene_SubMenuBackground"))->Show_Title(show_bg_titles);
	
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_SubMenu::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_SubMenu::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "inventory_button")
			{
				OnlineUser* user = e.Source_User;
				if (user != NULL)
				{
					Profile* profile = ProfileManager::Get()->Get_Profile(user->Get_Profile_Index());
					if (profile != NULL)
					{
						manager->Go(UIAction::Push(new UIScene_Inventory(profile, false), new UISlideInTopLevelTransition()));
						m_should_sync_profile = true;	
					}
				}
			}
			else if (e.Source->Get_Name() == "vote_button")
			{
				manager->Go(UIAction::Push(new UIScene_Vote(), new UISlideInTopLevelTransition()));		
			}
			else if (e.Source->Get_Name() == "team_button")
			{
				manager->Go(UIAction::Push(new UIScene_TeamChange(), new UISlideInTopLevelTransition()));		
			}
			else if (e.Source->Get_Name() == "achievements_button")
			{ 
				manager->Go(UIAction::Push(new UIScene_Achievements(), new UISlideInTopLevelTransition()));		
			}
			else if (e.Source->Get_Name() == "leaderboards_button")
			{
				manager->Go(UIAction::Push(new UIScene_Leaderboards(), new UISlideInTopLevelTransition()));		
			}
			else if (e.Source->Get_Name() == "statistics_button")
			{
				manager->Go(UIAction::Push(new UIScene_Statistics(), new UISlideInTopLevelTransition()));		
			}
			else if (e.Source->Get_Name() == "options_button")
			{
				manager->Go(UIAction::Push(new UIScene_Options(), new UISlideInTopLevelTransition()));		
			}
			else if (e.Source->Get_Name() == "global_chat_button")
			{
				manager->Go(UIAction::Push(new UIScene_GlobalChat(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				Close(manager);
			}
			else if (e.Source->Get_Name() == "exit_button")
			{
				m_leaving = true;
				manager->Play_UI_Sound(UISoundType::Exit_SubMenu);

				if (manager->Get_Scene_By_Name("UIScene_GameSetupMenu") != NULL)
					manager->Go(UIAction::Pop(new UIFadeInToNewTransition(), "UIScene_GameSetupMenu"));	
				else
					manager->Go(UIAction::Pop(new UIFadeInToNewTransition(), "UIScene_PlayerLogin"));	
			}
		}
		break;
	}
}