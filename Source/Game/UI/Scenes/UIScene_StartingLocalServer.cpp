// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_StartingLocalServer.h"
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

#include "Engine/Engine/GameEngine.h"

#include "Engine/Online/OnlineMatching.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineServer.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_StartingLocalServer::UIScene_StartingLocalServer(bool start_editor)
	: m_start_editor(start_editor)
{
	Set_Layout("starting_local_server");	
}

bool UIScene_StartingLocalServer::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_StartingLocalServer::Get_Name()
{
	return "UIScene_StartingLocalServer";
}

bool UIScene_StartingLocalServer::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_StartingLocalServer::Should_Display_Cursor()
{
	return true;
}

bool UIScene_StartingLocalServer::Is_Focusable()
{
	return true;
}

void UIScene_StartingLocalServer::Enter(UIManager* manager)
{
	OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();
	if (!GameEngine::Get()->Start_Local_Server(&settings))
	{
		Failed();
		return;
	}
}	

void UIScene_StartingLocalServer::Exit(UIManager* manager)
{
}	

void UIScene_StartingLocalServer::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	OnlineServer* server = OnlinePlatform::Get()->Server();
	switch (server->Get_State())
	{
		case OnlineServer_State::Initializing:
		case OnlineServer_State::Securing:
			{
				// Dum de dum do
				break;
			}
		case OnlineServer_State::Active:
			{
				Succeeded();
				return;
			}
		case OnlineServer_State::Idle:
			// Fallthrough
		case OnlineServer_State::Error:
			// Fallthrough
		default:
			{
				if (server->Get_Error() == OnlineServer_Error::Offline && 
					OnlineMatching::Get()->Get_State() == OnlineMatching_State::In_Lobby &&
					OnlineMatching::Get()->Get_Lobby_Settings().IsOffline)
				{
					Succeeded();
				}
				else
				{
					Failed();
				}
				return;
			}
			break;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_StartingLocalServer::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_StartingLocalServer::Succeeded()
{
	// Connect the lobby to the local-server.
	OnlineMatching::Get()->Connect_Lobby_To_Server(OnlineMatching::Get()->Get_Local_Server());
				
	// Set us as in-game!
	OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();
	settings.InGame = true;
	OnlineMatching::Get()->Set_Lobby_Settings(settings);

	// Start editor?
	if (m_start_editor == true)
	{
		GameNetManager::Get()->Game_Server()->Toggle_Editor_Mode();
	}

	// Back to lobby screen!
	GameEngine::Get()->Get_UIManager()->Go(UIAction::Pop(NULL));
}

void UIScene_StartingLocalServer::Failed()
{
	OnlineMatching::Get()->Send_Lobby_Chat("#lobby_member_search_no_local_server", true);
	GameEngine::Get()->Stop_Local_Server();

	// Back to lobby screen!
	GameEngine::Get()->Get_UIManager()->Go(UIAction::Pop(NULL));
}

void UIScene_StartingLocalServer::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				Failed();
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
		}
		break;
	}
}
