// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Lobby.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_ConnectToServer.h"
#include "Game/UI/Scenes/UIScene_StartingLocalServer.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UITextPanel.h"
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UIDropDownMenu.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/GeoIP/GeoIPManager.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"
#include "Engine/Online/OnlineMatching.h"
#include "Engine/Online/OnlineClient.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_HostGameSetup.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_Lobby::UIScene_Lobby(bool auto_continue, bool start_editor)
	: m_game_server_finder_old_state(OnlineGameServerFinder_State::Idle)
	, m_connecting(false)
	, m_auto_continue(auto_continue)
	, m_start_editor(start_editor)
	, m_first_enter(false)
{
	Set_Layout("lobby");	

	m_lobby_join_event_delegate = new Delegate<UIScene_Lobby, OnlineMatching_LobbyMemberJoinedEventData>(this, &UIScene_Lobby::Callback_Lobby_Member_Joined);
	m_lobby_left_event_delegate = new Delegate<UIScene_Lobby, OnlineMatching_LobbyMemberLeftEventData>(this, &UIScene_Lobby::Callback_Lobby_Member_Left);
	m_lobby_chat_event_delegate = new Delegate<UIScene_Lobby, OnlineMatching_LobbyChatEventData>(this, &UIScene_Lobby::Callback_Lobby_Chat);
	m_lobby_member_data_update_event_delegate = new Delegate<UIScene_Lobby, OnlineMatching_LobbyMemberDataUpdateEventData>(this, &UIScene_Lobby::Callback_Lobby_Member_Data_Update);

	OnlineMatching::Get()->On_Lobby_Member_Joined			+= m_lobby_join_event_delegate;
	OnlineMatching::Get()->On_Lobby_Member_Left				+= m_lobby_left_event_delegate;
	OnlineMatching::Get()->On_Lobby_Chat					+= m_lobby_chat_event_delegate;
	OnlineMatching::Get()->On_Lobby_Member_Data_Updated		+= m_lobby_member_data_update_event_delegate;
}

UIScene_Lobby::~UIScene_Lobby()
{
	NetManager::Get()->Leave_Network_Game();

	OnlineMatching::Get()->On_Lobby_Member_Joined			-= m_lobby_join_event_delegate;
	OnlineMatching::Get()->On_Lobby_Member_Left				-= m_lobby_left_event_delegate;
	OnlineMatching::Get()->On_Lobby_Chat					-= m_lobby_chat_event_delegate;
	OnlineMatching::Get()->On_Lobby_Member_Data_Updated		-= m_lobby_member_data_update_event_delegate;

	SAFE_DELETE(m_lobby_join_event_delegate);
	SAFE_DELETE(m_lobby_left_event_delegate);
	SAFE_DELETE(m_lobby_chat_event_delegate);
	SAFE_DELETE(m_lobby_member_data_update_event_delegate);
}
	
void UIScene_Lobby::Callback_Lobby_Member_Joined(OnlineMatching_LobbyMemberJoinedEventData* data)
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();

	UITextPanel* panel = Find_Element<UITextPanel*>("chat_history");
	
	std::string safe_username = MarkupFontRenderer::Escape(data->User->Get_Username());
	panel->Append_Text(SF("#lobby_member_joined_lobby", safe_username.c_str()));

	Refresh_Member_List(manager);
}

void UIScene_Lobby::Callback_Lobby_Member_Left(OnlineMatching_LobbyMemberLeftEventData* data)
{	
	UIManager* manager = GameEngine::Get()->Get_UIManager();

	UITextPanel* panel = Find_Element<UITextPanel*>("chat_history");

	std::string safe_username = MarkupFontRenderer::Escape(data->User->Get_Username());
	panel->Append_Text(SF("#lobby_member_left_lobby", safe_username.c_str()));

	Refresh_Member_List(manager);
}

void UIScene_Lobby::Callback_Lobby_Chat(OnlineMatching_LobbyChatEventData* data)
{
	UITextPanel* panel = Find_Element<UITextPanel*>("chat_history");

	if (data->User == NULL)
	{
		panel->Append_Text(S(data->Text.c_str()));
	}
	else
	{
		std::string safe_username = MarkupFontRenderer::Escape(data->User->Get_Username());
		std::string safe_text = MarkupFontRenderer::Escape(data->Text);

		panel->Append_Text(StringHelper::Format("[c=160,160,160,255](%s)[/c] %s", safe_username.c_str(), safe_text.c_str()));
	}
}

void UIScene_Lobby::Callback_Lobby_Member_Data_Update(OnlineMatching_LobbyMemberDataUpdateEventData* data)
{
	Refresh_Member_List(GameEngine::Get()->Get_UIManager());
}

const char* UIScene_Lobby::Get_Name()
{
	return "UIScene_Lobby";
}

bool UIScene_Lobby::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Lobby::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Lobby::Is_Focusable()
{
	return true;
}

UIScene* UIScene_Lobby::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_Lobby::Refresh_Preview(UIManager* manager)
{
	OnlineMatching_LobbySettings lobby_settings = OnlineMatching::Get()->Get_Lobby_Settings();
	bool is_host				 = OnlineMatching::Get()->Is_Lobby_Host();
	bool looking_for_game_server = (m_game_server_finder.Get_State() != OnlineGameServerFinder_State::Idle);
	
	//UILabel*	player_count_label		= Find_Element<UILabel*>("max_players_label");	
	UILabel*	secure_label			= Find_Element<UILabel*>("secure_label");	
	UILabel*	privacy_label			= Find_Element<UILabel*>("private_label");
	UIPanel*	map_panel				= Find_Element<UIPanel*>("map_image_panel");
	UILabel*	name_label				= Find_Element<UILabel*>("map_name_label");
	UILabel*	description_label		= Find_Element<UILabel*>("map_description_label");
	UIButton*	change_settings_button	= Find_Element<UIButton*>("change_settings_button");
	UIButton*	start_button			= Find_Element<UIButton*>("start_button");
	UIButton*	kick_button				= Find_Element<UIButton*>("kick_button");
	
	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(lobby_settings.MapGUID.c_str());
	/*
	if (handle == NULL)
	{
		OnlineMatching::Get()->Leave_Lobby();
		NetManager::Get()->Leave_Network_Game();

		manager->Go(UIAction::Replace(new UIScene_Dialog(S("#matching_error_map_not_found")), new UIFadeTopLevelTransition()));
		return;
	}
	*/

	if (lobby_settings.Private == true)
		privacy_label->Set_Value("#lobby_private_game");
	else
		privacy_label->Set_Value("#lobby_public_game");
	
	if (lobby_settings.Secure == true)
		secure_label->Set_Value("#lobby_vac_secure");
	else
		secure_label->Set_Value("#lobby_vac_insecure");

	//player_count_label->Set_Value(SF("#lobby_number_of_players", lobby_settings.MaxPlayers));
	
	if (handle != NULL)
	{
		name_label->Set_Value(S(handle->Get()->Get_Header()->Long_Name.c_str()));
		map_panel->Set_Background_Image(handle->Get()->Get_Preview_Image());
	}
	else
	{
		name_label->Set_Value(S("#lobby_custom_map"));
		map_panel->Set_Background_Image((Texture*)NULL);
	}

	change_settings_button->Set_Enabled(is_host);
	start_button->Set_Enabled(is_host);

	if (looking_for_game_server == false)
	{
		start_button->Set_Value(S("#lobby_start_game"));
	}
	else
	{
		start_button->Set_Value(S("#lobby_cancel_start"));
	}
	
	kick_button->Set_Enabled(is_host);
}

void UIScene_Lobby::Refresh_Member_List(UIManager* manager)
{
	UIListView* member_list = Find_Element<UIListView*>("player_list");
	member_list->Clear_Items();

	std::vector<OnlineUser*> members = OnlineMatching::Get()->Get_Lobby_Members();
	
	OnlineMatching_LobbySettings lobby_settings = OnlineMatching::Get()->Get_Lobby_Settings();
	OnlineUser* lobby_owner = OnlineMatching::Get()->Get_Lobby_Host();
	int slots_used = 0;

	for (std::vector<OnlineUser*>::iterator iter = members.begin(); iter != members.end(); iter++)
	{
		OnlineUser* user = *iter;
		
		GeoIPResult result = user->Get_GeoIP_Result();

		std::string safe_username = MarkupFontRenderer::Escape(user->Get_Username());
		
		std::vector<std::string> values;
		values.push_back(StringHelper::Lowercase(StringHelper::Format("[img=flag_%s,1,1,0,0]", result.CountryShortName).c_str()));
		values.push_back(StringHelper::Format("[avatar=%i,1,1,0,0]", user->Get_ID()));

		// Is user talking?
		if (user->Is_Talking())
		{
			values.push_back("[img=voice_chat_indicator_1,1.0,1.0,0,0]");
		}
		else
		{
			values.push_back("[img=voice_chat_indicator_0,1.0,1.0,0,0]");
		}

		if (user == lobby_owner)
		{
			values.push_back(StringHelper::Format("[c=255,64,64,128]%s[/c]", safe_username.c_str()));	
			//values.push_back(StringHelper::Format("%s %s", safe_username.c_str(), S("#lobby_member_owner_postfix")));	
		}
		else
		{
			values.push_back(safe_username.c_str());
		}
		member_list->Add_Item(values, user);

		// Add guests.
		for (int i = 0; i < user->Get_Guest_Count(); i++)
		{
			std::vector<std::string> subvalues;
			subvalues.push_back(StringHelper::Lowercase(StringHelper::Format("[img=flag_%s,1.0,1.0,0,0]", result.CountryShortName).c_str()));
			subvalues.push_back(StringHelper::Format("[avatar=%i,1,1,0,0]", user->Get_ID()));
			subvalues.push_back("");
			subvalues.push_back(StringHelper::Format("%s (%i)", safe_username.c_str(), i + 1));
			member_list->Add_Item(subvalues, user);
		}

		slots_used += 1 + user->Get_Guest_Count();
	}

	for (int i = 0; i < (lobby_settings.MaxPlayers - slots_used); i++)
	{
		std::vector<std::string> values;
		values.push_back("");
		values.push_back("");
		values.push_back("");
		values.push_back(S("#lobby_member_open_slot"));
		member_list->Add_Item(values, NULL);
	}
}

void UIScene_Lobby::Refresh(UIManager* manager)
{	
	UIScene::Refresh(manager);

	UITextPanel* panel = Find_Element<UITextPanel*>("chat_history");
	UIListView* member_list = Find_Element<UIListView*>("player_list");
	
	// Add joined lobby message for all local users.
	std::vector<OnlineUser*> users = OnlinePlatform::Get()->Get_Local_Users();
	for (std::vector<OnlineUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		OnlineUser* user = *iter;
		if (user->Is_Guest())
		{
			continue;
		}

		std::string safe_username = MarkupFontRenderer::Escape(user->Get_Username());

		panel->Append_Text(SF("#lobby_member_joined_lobby", safe_username.c_str()));
	}

	// Add everyone to the list.
	member_list->Add_Column("Flag",		0.16f, false, true);
	member_list->Add_Column("Avatar",   0.10f, false, true);
	member_list->Add_Column("Voice",    0.10f, false, true);
	member_list->Add_Column("Username", 0.64f, true, true);

	// Refresh the member list.
	Refresh_Member_List(manager);
}

void UIScene_Lobby::Enter(UIManager* manager)
{
	Refresh_Preview(manager);

	if (!m_first_enter)
	{
		manager->Focus(Find_Element<UIButton*>("start_button"));
		m_first_enter = true;
	}

	if (m_connecting == true)
	{
		manager->Go(UIAction::Pop(new UISlideInTransition()));
	}
}	

void UIScene_Lobby::Exit(UIManager* manager)
{
}	

void UIScene_Lobby::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	OnlineMatching_State::Type state = OnlineMatching::Get()->Get_State();

	// Update preview if the host has changed the lobby settings.
	if (OnlineMatching::Get()->Get_Lobby_Settings_Changed() ||
		m_game_server_finder.Get_State() != m_game_server_finder_old_state)
	{
		Refresh_Preview(manager);
		Refresh_Member_List(manager);
	}
	m_game_server_finder_old_state = m_game_server_finder.Get_State();

	// Blah we're updating each frame because of the dam voice-icon indicator.
	Refresh_Member_List(manager);

	// Derp.
	if (m_auto_continue == true)
	{		
		m_auto_continue = false;
		manager->Go(UIAction::Push(new UIScene_StartingLocalServer(m_start_editor), new UIFadeInTransition()));		
		return;
	}

	// Are we connected to a server now?
	OnlineMatching_Server* server = OnlineMatching::Get()->Get_Lobby_Server();
	if (server != NULL)
	{
		DBG_LOG("Lobby is now connected to a server, connecting to server ...");
		manager->Go(UIAction::Push(new UIScene_ConnectToServer(*server), new UIFadeInTransition()));
		m_connecting = true;
		return;
	}
	
	// Lobby state changed?
	switch (state)
	{
		case OnlineMatching_State::In_Lobby:
			{		
				// Dum de dum dum do.
			}
			break;
		case OnlineMatching_State::Error:
			{
				OnlineMatching_Error::Type error = OnlineMatching::Get()->Get_Error();
				manager->Go(UIAction::Replace(new UIScene_Dialog(OnlineMatching::Get()->Get_Error_Message(error)), NULL));
			}
			break;
		default:
			{
				manager->Go(UIAction::Replace(new UIScene_Dialog("#matching_error_internal_error"), NULL));
			}
			break;
	}

	// Tick the game server finder.
	m_game_server_finder.Tick(time);
	switch (m_game_server_finder.Get_State())
	{
	case OnlineGameServerFinder_State::Idle:
	case OnlineGameServerFinder_State::Searching:
	case OnlineGameServerFinder_State::Reserving:
		{
			// We don't care about any of these states.
			break;
		}
	case OnlineGameServerFinder_State::No_Server_Found:
		{
			OnlineMatching::Get()->Send_Lobby_Chat("#lobby_member_search_no_server_found", true);
			Refresh_Preview(manager);

			m_game_server_finder.Cancel_Search();
			
			DBG_LOG("Starting local server ..");				
			manager->Go(UIAction::Push(new UIScene_StartingLocalServer(), new UIFadeInTransition()));		
			
			break;
		}
	case OnlineGameServerFinder_State::Found_Server:
		{
			OnlineMatching::Get()->Send_Lobby_Chat("#lobby_member_search_server_found", true);
			Refresh_Preview(manager);

			m_game_server_finder.Cancel_Search();

			OnlineMatching::Get()->Connect_Lobby_To_Server(m_game_server_finder.Get_Reserved_Server());

			// Set us as in-game!
			OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();
			settings.InGame = true;
			OnlineMatching::Get()->Set_Lobby_Settings(settings);
			
			break;
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Lobby::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Lobby::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::TextBox_Enter:
		{
			if (e.Source->Get_Name() == "chat_box")
			{
				UITextBox* box = static_cast<UITextBox*>(e.Source);
				OnlineMatching::Get()->Send_Lobby_Chat(box->Get_Value());

				box->Set_Value("");
			}
		}
		break;
	case UIEventType::ListView_Item_Click:
		{
			if (e.Source->Get_Name() == "player_list")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("player_list_option_menu");
				UIListView* member_list = Find_Element<UIListView*>("player_list");

				Rect2D selection_box = member_list->Get_Selected_Item_Rectangle();
				const UIListViewItem& item = member_list->Get_Selected_Item();

				if (item.MetaData != NULL)
				{
					drop_down_menu->Show(Rect2D(selection_box.X, selection_box.Y + selection_box.Height, selection_box.Width, drop_down_menu->Get_Screen_Box().Height));
				}
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "change_settings_button")
			{
				manager->Go(UIAction::Push(new UIScene_HostGameSetup(true), new UIFadeInTransition()));
			}
			else if (e.Source->Get_Name() == "start_button")
			{
				bool looking_for_game_server = (m_game_server_finder.Get_State() != OnlineGameServerFinder_State::Idle);

				if (looking_for_game_server == true)
				{
					OnlineMatching::Get()->Send_Lobby_Chat("#lobby_member_search_server_cancel", true);

					m_game_server_finder.Cancel_Search();

					Refresh_Preview(manager);
				}
				else
				{	
					int max_ping = *EngineOptions::matching_max_ping;
					int search_timeout = *EngineOptions::matching_search_timeout;
					OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();

					// Private and local user limited lobbies should not allow dedi server usage.
					if (!settings.Private && !settings.Local && settings.MaxPlayers > OnlinePlatform::Get()->Get_Local_User_Count())
					{
						OnlineMatching::Get()->Send_Lobby_Chat("#lobby_member_searching_server", true);

						m_game_server_finder.Begin_Search(max_ping, search_timeout, settings);
					}
					else
					{
						m_auto_continue = true;
					}

					Refresh_Preview(manager);
				}
			}
			else if (e.Source->Get_Name() == "invite_button")
			{
				if (OnlineMatching::Get()->Get_Lobby_Settings().IsOffline)
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_offline_lobby")), new UIFadeInTransition()));
				}
				else
				{
					OnlinePlatform::Get()->Client()->Show_Invite_Dialog();
				}
			}
			else if (e.Source->Get_Name() == "kick_button")
			{
				if (OnlineMatching::Get()->Get_Lobby_Settings().IsOffline)
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_offline_lobby")), new UIFadeInTransition()));
				}
				else
				{
					UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("player_list_option_menu");
					UIListView* member_list = Find_Element<UIListView*>("player_list");

					drop_down_menu->Hide();

					UIListViewItem item = member_list->Get_Selected_Item();
					OnlineUser* user = reinterpret_cast<OnlineUser*>(item.MetaData);

					OnlineMatching::Get()->Kick_Lobby_Member(user);
				}
			}
			else if (e.Source->Get_Name() == "profile_button")
			{
				if (OnlineMatching::Get()->Get_Lobby_Settings().IsOffline)
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_offline_lobby")), new UIFadeInTransition()));
				}
				else
				{
					UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("player_list_option_menu");
					UIListView* member_list = Find_Element<UIListView*>("player_list");

					drop_down_menu->Hide();

					UIListViewItem item = member_list->Get_Selected_Item();
					OnlineUser* user = reinterpret_cast<OnlineUser*>(item.MetaData);

					OnlinePlatform::Get()->Client()->Show_Profile_Dialog(user);
				}
			}
			else if (e.Source->Get_Name() == "close_button")
			{				
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("player_list_option_menu");

				drop_down_menu->Hide();
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}
