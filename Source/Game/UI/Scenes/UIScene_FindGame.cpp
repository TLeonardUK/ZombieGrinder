// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_FindGame.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_ConnectToServer.h"
#include "Game/UI/Scenes/UIScene_Lobby.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
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

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

// Generic sort predicate, sorts by lowest to highest ping.
bool Sort_Found_Servers_By_Ping(OnlineMatching_Server a, OnlineMatching_Server b)
{
	return (a.Ping < b.Ping);
}

UIScene_FindGame::UIScene_FindGame(MapFileHandle* map, bool join_lobbies, bool join_games)
	: m_map(map)
	, m_join_lobbies(join_lobbies)
	, m_join_games(join_games)
	, m_checking_servers(false)
	, m_server_refresh_state(0)
	, m_connecting(false)
	, m_accept_invite(false)
{
	Set_Layout("find_game");	
}

UIScene_FindGame::UIScene_FindGame(bool accept_invite)
	: m_accept_invite(accept_invite)
	, m_map(NULL)
	, m_join_games(true)
	, m_join_lobbies(true)
	, m_checking_servers(false)
	, m_server_refresh_state(0)
	, m_connecting(false)
{
	Set_Layout("find_game");	
}

bool UIScene_FindGame::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_FindGame::Get_Name()
{
	return "UIScene_FindGame";
}

bool UIScene_FindGame::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_FindGame::Should_Display_Cursor()
{
	return true;
}

bool UIScene_FindGame::Is_Focusable()
{
	return true;
}

void UIScene_FindGame::Enter(UIManager* manager)
{
	std::string map_name = "";

	if (m_connecting == true)
	{
		manager->Go(UIAction::Pop(new UISlideInTransition()));
		return;
	}

	// Filter lobbies by map?
	if (m_map != NULL)
	{
		map_name = m_map->Get()->Get_Header()->GUID;
	}

	if (m_accept_invite == true)
	{
		OnlineMatching::Get()->Join_Invite_Lobby();
	}
	else
	{
		OnlineMatching::Get()->Find_Lobby(map_name, true, true);
	}
}	

void UIScene_FindGame::Exit(UIManager* manager)
{
	if (m_checking_servers)
	{
		OnlineMatching::Get()->Cancel_Server_List_Refresh(OnlineMatching_ServerSource::Internet);
	}
}	

void UIScene_FindGame::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Try to look for lobbies first.
	if (!m_checking_servers)
	{
		switch (OnlineMatching::Get()->Get_State())
		{
		case OnlineMatching_State::Finding_Lobby:
		case OnlineMatching_State::Joining_Lobby:
			{
				break;
			}
		case OnlineMatching_State::In_Lobby:
			{
				manager->Go(UIAction::Replace(new UIScene_Lobby(), new UISlideInTransition()));				
			}
			break;
		case OnlineMatching_State::Error:
			{
				OnlineMatching_Error::Type error = OnlineMatching::Get()->Get_Error();
				if (m_accept_invite == true)
				{
					manager->Go(UIAction::Replace(new UIScene_Dialog(OnlineMatching::Get()->Get_Error_Message(error)), NULL));
				}
				else
				{
					m_checking_servers = true;
				}
			}
			break;
		default:
			{
				if (m_accept_invite == true)
				{
					manager->Go(UIAction::Replace(new UIScene_Dialog("#matching_error_internal_error"), NULL));
				}
				else
				{
					m_checking_servers = true;
				}
			}
			break;
		}
	}

	// No lobbies, look for servers.
	else
	{
		switch (m_server_refresh_state)
		{
		// Start refresh.
		case 0:
			{
				OnlineMatching::Get()->Refresh_Server_List(OnlineMatching_ServerSource::Internet);
				m_server_refresh_state = 1;
				m_server_search_timer = 0.0f;
				break;
			}
		// Wait for refresh.
		case 1:
			{
				m_server_search_timer += time.Get_Frame_Time();

				if (!OnlineMatching::Get()->Is_Server_List_Refreshing(OnlineMatching_ServerSource::Internet) || 
					m_server_search_timer >= server_search_timeout)
				{
					std::vector<OnlineMatching_Server> servers = OnlineMatching::Get()->Get_Server_List(OnlineMatching_ServerSource::Internet);

					DBG_LOG("[Find Game] Finished retrieving server list (%i total).", servers.size());

					std::vector<OnlineMatching_Server> potential;

					// Grab all servers with no players and a low ping as potential servers to join.
					for (std::vector<OnlineMatching_Server>::iterator iter = servers.begin(); iter != servers.end(); iter++)
					{
						OnlineMatching_Server server = *iter;
						if (server.PlayerCount > 0 && server.PlayerCount < server.MaxPlayerCount && server.Ping < max_server_ping)
						{
							if (m_map == NULL || server.Map == m_map->Get()->Get_Header()->Short_Name)
							{
								potential.push_back(server);
							}
						}
					}

					// Sort list from low ping to high ping.
					std::sort(potential.begin(), potential.end(), Sort_Found_Servers_By_Ping);

					// Join the best!
					if (potential.size() > 0)
					{
						DBG_LOG("[Find Game] %i servers are potential matches.", potential.size());
						manager->Go(UIAction::Replace(new UIScene_ConnectToServer(potential.at(0)), new UIFadeInTransition()));
						m_connecting = true;
					}
					else
					{
						DBG_LOG("[Find Game] No servers found, find game over.");
						manager->Go(UIAction::Replace(new UIScene_Dialog("#matching_error_no_lobby_matches"), NULL));
					}

					m_server_refresh_state = 2;
				}

				break;
			}
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_FindGame::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_FindGame::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				OnlineMatching::Get()->Leave_Lobby();
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
		}
		break;
	}
}
