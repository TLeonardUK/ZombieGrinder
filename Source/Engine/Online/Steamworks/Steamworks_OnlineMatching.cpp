// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineServer.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineVoiceChat.h"

#include "Engine/Engine/GameEngine.h"

#include "Generic/Math/Math.h"

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/NetUser.h"

#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Version.h"

#include <algorithm>

Steamworks_OnlineMatching::Steamworks_OnlineMatching()
	: m_lobby_invite_callback(this, &Steamworks_OnlineMatching::Callback_LobbyInvite)
	, m_lobby_enter_callback(this, &Steamworks_OnlineMatching::Callback_LobbyEnter)
	, m_lobby_data_update_callback(this, &Steamworks_OnlineMatching::Callback_LobbyDataUpdate)
	, m_lobby_chat_update_callback(this, &Steamworks_OnlineMatching::Callback_LobbyChatUpdate)
	, m_lobby_chat_message_callback(this, &Steamworks_OnlineMatching::Callback_LobbyChatMessage)
	, m_lobby_game_created_callback(this, &Steamworks_OnlineMatching::Callback_LobbyGameCreated)
	, m_lobby_kicked_callback(this, &Steamworks_OnlineMatching::Callback_LobbyKicked)
	, m_lobby_created_callback(this, &Steamworks_OnlineMatching::Callback_LobbyCreated)
	, m_lobby_join_requested_callback(this, &Steamworks_OnlineMatching::Callback_LobbyJoinRequested)
	, m_game_join_requested_callback(this, &Steamworks_OnlineMatching::Callback_GameJoinRequested)
	, m_invite_index(0)
	, m_lobby_offline_connected(false)
{
	memset(m_server_list_requests, 0, sizeof(HServerListRequest) * OnlineMatching_ServerSource::COUNT);
	memset(m_server_list_updated, 0, sizeof(bool) * OnlineMatching_ServerSource::COUNT);
} 

Steamworks_OnlineMatching::~Steamworks_OnlineMatching()
{
	for (int i = 0; i < OnlineMatching_ServerSource::COUNT; i++)
	{
		// Delete pending requests.
		HServerListRequest request = m_server_list_requests[i];
		if (request != NULL)
		{
			SteamMatchmakingServers()->CancelQuery(request);
			SteamMatchmakingServers()->ReleaseRequest(request);
			m_server_list_requests[i] = NULL;
		}
	}
}

OnlineMatching_ServerSource::Type Steamworks_OnlineMatching::Get_Source_From_Request(HServerListRequest hRequest)
{
	for (int i = 0; i < OnlineMatching_ServerSource::COUNT; i++)
	{
		HServerListRequest request = m_server_list_requests[i];
		if (request == hRequest)
		{
			return (OnlineMatching_ServerSource::Type)i;
		}
	}

	return OnlineMatching_ServerSource::Unknown;
}

bool Steamworks_OnlineMatching::Initialize()
{
	// Grab appid from online platform.
	m_appid = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get())->Get_AppID();	
	m_state = OnlineMatching_State::Idle;
	m_error = OnlineMatching_Error::NONE;
	m_lobby_settings_changed = false;
	m_lobby_match_request = k_uAPICallInvalid;
	m_invite_pending = false;

	// Recieved a command line invite?
	u64 id = (*EngineOptions::connect_lobby);
	if (id != 0)
	{
		m_invite_lobby_id = id;
		m_invite_pending = true;
		m_invite_index++;
	}

	return true;
}

void Steamworks_OnlineMatching::Tick(const FrameTime& time)
{
	m_lobby_settings_changed = false;
}

void Steamworks_OnlineMatching::ServerResponded(HServerListRequest hRequest, int iServer)
{
	OnlineMatching_ServerSource::Type source = Get_Source_From_Request(hRequest);
	m_server_list_updated[(int)source] = true;
}

void Steamworks_OnlineMatching::ServerFailedToRespond(HServerListRequest hRequest, int iServer)
{
	OnlineMatching_ServerSource::Type source = Get_Source_From_Request(hRequest);
	m_server_list_updated[(int)source] = true;
}

void Steamworks_OnlineMatching::RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response)
{
	OnlineMatching_ServerSource::Type source = Get_Source_From_Request(hRequest);
	m_server_list_updated[(int)source] = true;
}

void Steamworks_OnlineMatching::Refresh_Server_List(OnlineMatching_ServerSource::Type source)
{
	DBG_LOG("[Steam Matchmaking] Refreshing server list from source '%i'.", source);

	HServerListRequest request = m_server_list_requests[(int)source];

	if (request != NULL)
	{
		SteamMatchmakingServers()->RefreshQuery(request);
		return;
	}

	std::string game_dir = *EngineOptions::server_game_dir;
	std::string version_string = StringHelper::Format("protocol_%i", NetManager::Get()->Get_Net_Version());

	MatchMakingKeyValuePair_t default_params[4];
	MatchMakingKeyValuePair_t* pdefault_params = default_params;
	int default_param_count = 4;

//#ifdef MASTER_BUILD
	strcpy(default_params[0].m_szKey, "and");				strcpy(default_params[0].m_szValue, "3");
		strcpy(default_params[1].m_szKey, "gamedir");		strcpy(default_params[1].m_szValue, game_dir.c_str());
		strcpy(default_params[2].m_szKey, "gametagsnor");	strcpy(default_params[2].m_szValue, "private");
		strcpy(default_params[3].m_szKey, "gametagsand");	strcpy(default_params[3].m_szValue, version_string.c_str());
//#else
//	strcpy(default_params[0].m_szKey, "and");				strcpy(default_params[0].m_szValue, "1");
//		strcpy(default_params[1].m_szKey, "gamedir");		strcpy(default_params[1].m_szValue, game_dir.c_str());

//	default_param_count = 2;
//#endif

	switch (source)
	{
	case OnlineMatching_ServerSource::Internet:			
		{
			request = SteamMatchmakingServers()->RequestInternetServerList(m_appid, &pdefault_params, default_param_count, this);		
			break;
		}
	case OnlineMatching_ServerSource::LAN:				
		{
			request = SteamMatchmakingServers()->RequestLANServerList(m_appid, this);						
			break;
		}
	case OnlineMatching_ServerSource::History:			
		{
			request = SteamMatchmakingServers()->RequestHistoryServerList(m_appid, &pdefault_params, default_param_count, this);			
			break;
		}
	case OnlineMatching_ServerSource::Friends:			
		{
			request = SteamMatchmakingServers()->RequestFriendsServerList(m_appid, &pdefault_params, default_param_count, this);			
			break;
		}
	case OnlineMatching_ServerSource::Favorites:		
		{
			request = SteamMatchmakingServers()->RequestFavoritesServerList(m_appid, &pdefault_params, default_param_count, this);		
			break;
		}
	case OnlineMatching_ServerSource::Reservable_Secure:		
		{
			// noplayers && dedicated && secure	

			MatchMakingKeyValuePair_t params[6];
			MatchMakingKeyValuePair_t* pParams = params;
			
			strcpy(params[0].m_szKey, "and");				strcpy(params[0].m_szValue, "5");
				strcpy(params[1].m_szKey, "gamedir");		strcpy(params[1].m_szValue, game_dir.c_str());
				strcpy(params[2].m_szKey, "noplayers");
				strcpy(params[3].m_szKey, "dedicated");
				strcpy(params[4].m_szKey, "secure");
				strcpy(params[5].m_szKey, "gametagsand");	strcpy(params[5].m_szValue, version_string.c_str());
			
			request = SteamMatchmakingServers()->RequestInternetServerList(m_appid, &pParams, 6, this);			
			break;
		}
	case OnlineMatching_ServerSource::Reservable_None_Secure:		
		{
			// noplayers && dedicated && !secure		
			
			MatchMakingKeyValuePair_t params[7];
			MatchMakingKeyValuePair_t* pParams = params;

			strcpy(params[0].m_szKey, "and");				strcpy(params[0].m_szValue, "6");
				strcpy(params[1].m_szKey, "gamedir");		strcpy(params[1].m_szValue, game_dir.c_str());
				strcpy(params[2].m_szKey, "noplayers");
				strcpy(params[3].m_szKey, "dedicated");
				strcpy(params[4].m_szKey, "gametagsand");	strcpy(params[4].m_szValue, version_string.c_str());
				strcpy(params[5].m_szKey, "nand");			strcpy(params[5].m_szValue, "1");
					strcpy(params[6].m_szKey, "secure");
			
			request = SteamMatchmakingServers()->RequestInternetServerList(m_appid, &pParams, 7, this);			
			break;
		}
	}

	m_server_list_requests[(int)source] = request;
	m_server_list_updated[(int)source] = true;
}

void Steamworks_OnlineMatching::Cancel_Server_List_Refresh(OnlineMatching_ServerSource::Type source)
{
	DBG_LOG("[Steam Matchmaking] Cancel refresh of server list from source '%i'.", source);

	HServerListRequest request = m_server_list_requests[(int)source];
	if (request != NULL)
	{
		SteamMatchmakingServers()->CancelQuery(request);
		SteamMatchmakingServers()->ReleaseRequest(request);
	
		m_server_list_requests[(int)source] = NULL;
	}
}

std::vector<OnlineMatching_Server> Steamworks_OnlineMatching::Get_Server_List(OnlineMatching_ServerSource::Type source)
{
	HServerListRequest request = m_server_list_requests[(int)source];
	if (request == NULL)
	{
		Refresh_Server_List(source);
		request = m_server_list_requests[(int)source];
	}

	int count = SteamMatchmakingServers()->GetServerCount(request);

	std::vector<OnlineMatching_Server> results;

	for (int i = 0; i < count; i++)
	{
		gameserveritem_t* item = SteamMatchmakingServers()->GetServerDetails(request, i);
		if (true) /* We ignore successful responses, we will survive without as we are connecting via nat-bypassing code. */ //item->m_bHadSuccessfulResponse == true)
		{
			if (item->m_nAppID != m_appid)
			{
				DBG_LOG("[Steam Matchmaking] Got server with incorrect appid '%s' (appid:%i, expected:%i).", item->GetName(), item->m_nAppID, m_appid);
				continue;
			}

			// Ignore duplicates (fffff, why you do this to me steam?)
			bool bIsDuplicate = false;
			for (std::vector<OnlineMatching_Server>::iterator iter = results.begin(); iter != results.end(); iter++)
			{
				OnlineMatching_Server& server = *iter;
				Steamworks_OnlineUser* sw_user = static_cast<Steamworks_OnlineUser*>(server.User);
				if (item->m_steamID == sw_user->Get_SteamID())
				{
					// Later items in server list will be more up to date (hopefully!?)
					server.Ping				= item->m_nPing;
					server.Name				= item->m_bHadSuccessfulResponse ? item->GetName() : "Unknown Server";
					server.Private			= item->m_bPassword;
					server.Secure			= item->m_bSecure;
					server.PlayerCount		= item->m_nPlayers;
					server.MaxPlayerCount	= item->m_nMaxPlayers;
					server.Map				= item->m_szMap;
					bIsDuplicate = true;
					break;
				}
			}

			if (bIsDuplicate)
			{
				continue;
			}
			
			// Get a user representation of this server.
			Steamworks_OnlinePlatform* client = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());
			Steamworks_OnlineUser* user = client->Get_User_By_SteamID(item->m_steamID);
			if (user == NULL)
			{
				user = client->Register_Remote_User(item->m_steamID);
			}

			OnlineMatching_Server server;
			server.Ping				= item->m_nPing;
			server.Name				= item->m_bHadSuccessfulResponse ? item->GetName() : "Unknown Server";
			server.Private			= item->m_bPassword;
			server.Secure			= item->m_bSecure;
			server.PlayerCount		= item->m_nPlayers;
			server.MaxPlayerCount	= item->m_nMaxPlayers;
			server.Map				= item->m_szMap;
			server.Address			= NetAddress(item->m_NetAdr.GetIP(), item->m_NetAdr.GetConnectionPort());
			server.User				= user;

			results.push_back(server);
		}
	}

	return results;
}

bool Steamworks_OnlineMatching::Has_Server_List_Updated(OnlineMatching_ServerSource::Type source)
{
	bool res = m_server_list_updated[(int)source];
	m_server_list_updated[(int)source] = false;
	return res;
}

bool Steamworks_OnlineMatching::Is_Server_List_Refreshing(OnlineMatching_ServerSource::Type source)
{
	HServerListRequest request = m_server_list_requests[(int)source];

	if (request == NULL)
	{
		return false;
	}

	return SteamMatchmakingServers()->IsRefreshing(request);
}

void Steamworks_OnlineMatching::Retrieve_Lobby_Settings()
{
	m_lobby_settings.MapGUID	= SteamMatchmaking()->GetLobbyData(m_lobby_id, "map_guid");

	const char* workshop_string = SteamMatchmaking()->GetLobbyData(m_lobby_id, "map_workshop_id");
#if defined(LANGUAGE_STANDARD_CPP11) || defined(PLATFORM_WIN32)
	m_lobby_settings.MapWorkshopID	= std::stoull(workshop_string);
#else
	m_lobby_settings.MapWorkshopID	= strtoull(workshop_string, NULL, 10);
#endif

	m_lobby_settings.MaxPlayers = atoi(SteamMatchmaking()->GetLobbyData(m_lobby_id, "max_players"));
	m_lobby_settings.Private	= atoi(SteamMatchmaking()->GetLobbyData(m_lobby_id, "private")) != 0;
	m_lobby_settings.Secure		= atoi(SteamMatchmaking()->GetLobbyData(m_lobby_id, "secure")) != 0;
	m_lobby_settings.Local		= atoi(SteamMatchmaking()->GetLobbyData(m_lobby_id, "local")) != 0;
	m_lobby_settings.InGame		= atoi(SteamMatchmaking()->GetLobbyData(m_lobby_id, "in_game")) != 0;
}

void Steamworks_OnlineMatching::Store_Lobby_Settings()
{
	if (m_lobby_settings.IsOffline)
	{
		return;
	}

	std::string version_string = StringHelper::Format("protocol_%i", NetManager::Get()->Get_Net_Version());
	SteamMatchmaking()->SetLobbyData(m_lobby_id, "map_guid",		m_lobby_settings.MapGUID.c_str());
	SteamMatchmaking()->SetLobbyData(m_lobby_id, "map_workshop_id",	StringHelper::To_String(m_lobby_settings.MapWorkshopID).c_str());
	SteamMatchmaking()->SetLobbyData(m_lobby_id, "max_players",		StringHelper::To_String(m_lobby_settings.MaxPlayers).c_str()); 
	SteamMatchmaking()->SetLobbyData(m_lobby_id, "private",			StringHelper::To_String(m_lobby_settings.Private).c_str()); 
	SteamMatchmaking()->SetLobbyData(m_lobby_id, "secure",			StringHelper::To_String(m_lobby_settings.Secure).c_str());
	SteamMatchmaking()->SetLobbyData(m_lobby_id, "local",			StringHelper::To_String(m_lobby_settings.Local).c_str());
	SteamMatchmaking()->SetLobbyData(m_lobby_id, "in_game",			StringHelper::To_String(m_lobby_settings.InGame).c_str()); 
	SteamMatchmaking()->SetLobbyData(m_lobby_id, "version",			version_string.c_str()); 

	if (m_lobby_settings.Private)
	{
		SteamMatchmaking()->SetLobbyType(m_lobby_id, k_ELobbyTypePrivate);
	}
	else
	{
		SteamMatchmaking()->SetLobbyType(m_lobby_id, k_ELobbyTypePublic);	
	}

	Enforce_Lobby_Member_Limit();
}

void Steamworks_OnlineMatching::Retrieve_Member_Lobby_Settings(Steamworks_OnlineUser* user)
{
	CSteamID id = user->Get_SteamID();

	const char* country = SteamMatchmaking()->GetLobbyMemberData(m_lobby_id, id, "country");
	if (country[0] != '\0')
	{
		user->Set_Short_Country_Name(country);
	}

	const char* local_user_count = SteamMatchmaking()->GetLobbyMemberData(m_lobby_id, id, "guest-count");
	if (local_user_count[0] != '\0')
	{
		user->Set_Guest_Count(atoi(local_user_count));
	}
	
	// Let everyone know he's joined!
	OnlineMatching_LobbyMemberDataUpdateEventData param;
	param.User = user;
	On_Lobby_Member_Data_Updated.Fire(&param);

	// Enfore lobby member limit.
	Enforce_Lobby_Member_Limit();
}

void Steamworks_OnlineMatching::Store_Member_Lobby_Settings()
{
	OnlineUser* user = Steamworks_OnlinePlatform::Get()->Get_Initial_User();

	SteamMatchmaking()->SetLobbyMemberData(m_lobby_id, "country",	 user->Get_GeoIP_Result().CountryShortName);
	SteamMatchmaking()->SetLobbyMemberData(m_lobby_id, "guest-count", StringHelper::To_String(OnlinePlatform::Get()->Get_Local_User_Count() - 1).c_str());
}

bool Steamworks_OnlineMatching::Is_Lobby_Host()
{
	if (m_state != OnlineMatching_State::In_Lobby)
	{
		return false;
	}

	if (m_lobby_settings.IsOffline)
	{
		return true;
	}

	Steamworks_OnlineUser* user = static_cast<Steamworks_OnlineUser*>(Steamworks_OnlinePlatform::Get()->Get_Initial_User());

	return SteamMatchmaking()->GetLobbyOwner(m_lobby_id) == user->Get_SteamID();
}

OnlineUser* Steamworks_OnlineMatching::Get_Lobby_Host()
{
	if (m_state != OnlineMatching_State::In_Lobby)
	{
		return false;
	}

	CSteamID owner_id = SteamMatchmaking()->GetLobbyOwner(m_lobby_id) ;
	
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_lobby_users.begin(); iter != m_lobby_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Get_SteamID() == owner_id)
		{
			return user;
		}
	}

	return NULL;
}

void Steamworks_OnlineMatching::Join_Next_Lobby_In_Queue()
{
	if (m_state != OnlineMatching_State::Joining_Lobby)
	{
		DBG_LOG("[Steam Matchmaking] Cannot join next lobby, in invalid state '%i'.", m_state);	
		return;
	}

	if (m_lobby_join_queue.size() == 0)
	{
		if (m_lobby_join_queue_started_by_find == true)
		{
			DBG_LOG("[Steam Matchmaking] Ran out of joinable lobbies.");	
			Set_Error(OnlineMatching_Error::No_Lobby_Matches);
		}
		else
		{
			DBG_LOG("[Steam Matchmaking] Ran out of joinable lobbies.");

			switch (m_last_lobby_join_error)
			{
				case k_EChatRoomEnterResponseDoesntExist:			Set_Error(OnlineMatching_Error::Dosent_Exist);				break;
				case k_EChatRoomEnterResponseNotAllowed:			Set_Error(OnlineMatching_Error::No_Permission);				break;
				case k_EChatRoomEnterResponseFull:					Set_Error(OnlineMatching_Error::Full);						break;
				case k_EChatRoomEnterResponseError:					Set_Error(OnlineMatching_Error::Unknown);					break;
				case k_EChatRoomEnterResponseBanned:				Set_Error(OnlineMatching_Error::Banned);					break;
				case k_EChatRoomEnterResponseLimited:				Set_Error(OnlineMatching_Error::No_Permissions);			break;
				case k_EChatRoomEnterResponseClanDisabled:			Set_Error(OnlineMatching_Error::Clan_Disabled);				break;
				case k_EChatRoomEnterResponseCommunityBan:			Set_Error(OnlineMatching_Error::Community_Lock);			break;
				case k_EChatRoomEnterResponseMemberBlockedYou:		Set_Error(OnlineMatching_Error::Blocked_By_Member);			break;
				case k_EChatRoomEnterResponseYouBlockedMember:		Set_Error(OnlineMatching_Error::Contains_Blocked_Member);	break;
				default:											Set_Error(OnlineMatching_Error::Unknown);					break;
			}
		}
		return;
	}
	else
	{
		CSteamID lobby_id = m_lobby_join_queue.back();
		m_lobby_join_queue.pop_back();
		
		DBG_LOG("[Steam Matchmaking] Attempting to join next lobby '%llu'.", lobby_id.ConvertToUint64());	
		SteamMatchmaking()->JoinLobby(lobby_id);
	}
}

void Steamworks_OnlineMatching::Callback_LobbyInvite(LobbyInvite_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved LobbyInvite_t callback.");	
}

void Steamworks_OnlineMatching::Callback_LobbyEnter(LobbyEnter_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved LobbyEnter_t callback.");	

	switch (param->m_EChatRoomEnterResponse)
	{
	case k_EChatRoomEnterResponseSuccess:
		{
			m_lobby_id = param->m_ulSteamIDLobby;

			// Clear old lobby list.
			m_lobby_users.clear();
						
			DBG_LOG("[Steam Matchmaking] Lobby Members:");	

			// Add all lobby members.
			int lobby_members = SteamMatchmaking()->GetNumLobbyMembers(m_lobby_id);
			for (int i = 0; i < lobby_members; i++)
			{
				CSteamID member_id = SteamMatchmaking()->GetLobbyMemberByIndex(m_lobby_id, i);
				
				Steamworks_OnlineUser* user = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get())->Get_User_By_SteamID(member_id);
				Steamworks_OnlineUser* lobby_user = Get_Lobby_Member_By_SteamID(member_id);
				
				// Already have user-cached?
				if (user != NULL)
				{
					DBG_LOG("[Steam Matchmaking] \t[%i] %s", i, user->Get_Username().c_str());	
					
					// Not already in lobby?
					if (lobby_user == NULL)
					{
						Lobby_User_Joined(user);
					}
				}

				// New user we haven't seen before.
				else
				{
					lobby_user = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get())->Register_Remote_User(member_id);

					DBG_LOG("[Steam Matchmaking] \t[%i] %s", i, lobby_user->Get_Username().c_str());	

					Lobby_User_Joined(lobby_user);
				}
			}

			// Set our lobby-specific-data
			Store_Member_Lobby_Settings();

			Set_State(OnlineMatching_State::In_Lobby);

			break;
		}
	
	default:
		{
			// Try and join next lobby in queue if it's available.
			m_last_lobby_join_error = param->m_EChatRoomEnterResponse;
			Join_Next_Lobby_In_Queue();

			break;
		}
	}
}

void Steamworks_OnlineMatching::Callback_LobbyDataUpdate(LobbyDataUpdate_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved LobbyDataUpdate_t callback.");	

	// Lobby settings update?
	if (param->m_ulSteamIDMember == m_lobby_id.ConvertToUint64())
	{
		DBG_LOG("[Steam Matchmaking] Lobby '%llu' settings updated.", param->m_ulSteamIDMember);		

		m_lobby_settings_changed = true;
		Retrieve_Lobby_Settings();
	}

	// Player settings update?
	else
	{
		DBG_LOG("[Steam Matchmaking] Player '%llu' settings updated.", param->m_ulSteamIDMember);
		
		Steamworks_OnlineUser* lobby_user = Get_Lobby_Member_By_SteamID(param->m_ulSteamIDMember);
		if (lobby_user != NULL)
		{
			Retrieve_Member_Lobby_Settings(lobby_user);
		}
	}
}

void Steamworks_OnlineMatching::Callback_LobbyChatUpdate(LobbyChatUpdate_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved LobbyChatUpdate_t callback.");	

	if (m_lobby_id != param->m_ulSteamIDLobby)
	{
		return;
	}

	CSteamID steam_id = CSteamID(param->m_ulSteamIDUserChanged);
	Steamworks_OnlineUser* user = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get())->Get_User_By_SteamID(steam_id);
	Steamworks_OnlineUser* lobby_user = Get_Lobby_Member_By_SteamID(steam_id);

	switch (param->m_rgfChatMemberStateChange)
	{
		case k_EChatMemberStateChangeEntered:
			{
				DBG_LOG("[Steam Matchmaking] User '%llu' entered the lobby.", param->m_ulSteamIDUserChanged);	
					
				// Already have user-cached?
				if (user != NULL)
				{
					// Not already in lobby?
					if (lobby_user == NULL)
					{
						Lobby_User_Joined(user);
					}
				}

				// New user we haven't seen before.
				else
				{
					lobby_user = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get())->Register_Remote_User(steam_id);
					Lobby_User_Joined(lobby_user);
				}

				break;
			}
		case k_EChatMemberStateChangeLeft:
		case k_EChatMemberStateChangeDisconnected:
		case k_EChatMemberStateChangeKicked:
		case k_EChatMemberStateChangeBanned:
			{
				DBG_LOG("[Steam Matchmaking] User '%llu' left the lobby.", param->m_ulSteamIDUserChanged);	

				// User actually in lobby?
				if (lobby_user != NULL)
				{
					Lobby_User_Left(lobby_user);
				}

				break;
			}
	}
}

void Steamworks_OnlineMatching::Callback_LobbyChatMessage(LobbyChatMsg_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved LobbyChatMsg_t callback.");

	switch (param->m_eChatEntryType)
	{
	case k_EChatEntryTypeChatMsg:
		{
			CSteamID		from_id;
			char			chat_data[sizeof(Steamworks_OnlineMatchingChatPacket)];
			EChatEntryType	type;

			if (SteamMatchmaking()->GetLobbyChatEntry(m_lobby_id, param->m_iChatID, &from_id, &chat_data, sizeof(Steamworks_OnlineMatchingChatPacket) - 1, &type) > 0)
			{
				Steamworks_OnlineUser* lobby_user = Get_Lobby_Member_By_SteamID(from_id);
				Steamworks_OnlineMatchingChatPacket* packet = reinterpret_cast<Steamworks_OnlineMatchingChatPacket*>(chat_data);

				if (lobby_user != NULL)
				{
					Process_Packet(packet, lobby_user);
				}
				else
				{
					DBG_LOG("[Steam Matchmaking] Recieved chat message from unknown user. Ignoring.");
				}
			}

			break;
		}
	}
}

void Steamworks_OnlineMatching::Callback_LobbyGameCreated(LobbyGameCreated_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved LobbyGameCreated_t callback.");	
}

void Steamworks_OnlineMatching::Callback_LobbyMatchList(LobbyMatchList_t* param, bool ioFailure)
{
	if (ioFailure == true)
	{
		DBG_LOG("[Steam Matchmaking] Recieved LobbyMatchList_t callback with an IOFailure :(.");	
	}
	else
	{
		DBG_LOG("[Steam Matchmaking] Recieved LobbyMatchList_t callback (%i results).", param->m_nLobbiesMatching);	
	}

	// No lobbies? Gay.
	if (param->m_nLobbiesMatching == 0 || ioFailure == true)
	{
		Set_Error(OnlineMatching_Error::No_Lobby_Matches);
		return;
	}

	// Lets add these lobbies to the list and start trying to join them!
	m_lobby_join_queue.clear();
	for (int i = 0; i < (int)param->m_nLobbiesMatching; i++)
	{
		CSteamID lobby_id = SteamMatchmaking()->GetLobbyByIndex(i);
		std::string guid = SteamMatchmaking()->GetLobbyData(lobby_id, "map_guid");

		MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(guid.c_str());

		if (handle != NULL)
		{
			m_lobby_join_queue.push_back(lobby_id);
		}
	}
	
	// No lobbies? Gay.
	if (m_lobby_join_queue.size() <= 0)
	{
		Set_Error(OnlineMatching_Error::No_Lobby_Matches);
		return;
	}
	m_lobby_join_queue_started_by_find = true;

	// Try and join the next lobby.
	Set_State(OnlineMatching_State::Joining_Lobby);
	Join_Next_Lobby_In_Queue();
	
	// Clear callback.
	m_lobby_match_request = k_uAPICallInvalid;
	m_lobby_match_list_callback.Cancel();
}

void Steamworks_OnlineMatching::Callback_LobbyKicked(LobbyKicked_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved LobbyKicked_t callback.");	
	Set_Error(OnlineMatching_Error::Kicked);
}

void Steamworks_OnlineMatching::Callback_LobbyCreated(LobbyCreated_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved LobbyCreated_t callback. Result=%i, LobbyID=%llu", param->m_eResult, param->m_ulSteamIDLobby);
	switch (param->m_eResult)
	{
	case k_EResultOK:				
		{
			m_lobby_id = param->m_ulSteamIDLobby;

			Store_Lobby_Settings();
		}
		break;
	case k_EResultNoConnection:
		{
			Set_Error(OnlineMatching_Error::Offline);
		}
		break;
	case k_EResultTimeout:
		{
			Set_Error(OnlineMatching_Error::Timeout);
		}
		break;
	case k_EResultFail:
		{
			Set_Error(OnlineMatching_Error::Unknown);
		}
		break;
	case k_EResultAccessDenied:
		{
			Set_Error(OnlineMatching_Error::Access_Denied);
		}
		break;
	case k_EResultLimitExceeded:
		{
			Set_Error(OnlineMatching_Error::Limit_Exceeded);
		}
		break;
	}
}

void Steamworks_OnlineMatching::Callback_LobbyJoinRequested(GameLobbyJoinRequested_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved GameLobbyJoinRequested_t callback (lobby id = %llu).", param->m_steamIDLobby.ConvertToUint64());

	if (m_state == OnlineMatching_State::In_Lobby &&
		m_lobby_id == param->m_steamIDLobby)
	{
		DBG_LOG("[Steam Matchmaking] Ignored join request, we are already in lobby.");
		return;
	}

	m_invite_pending	= true;
	m_invite_index++;
	m_invite_lobby_id	= param->m_steamIDLobby;
}

void Steamworks_OnlineMatching::Callback_GameJoinRequested(GameRichPresenceJoinRequested_t* param)
{
	DBG_LOG("[Steam Matchmaking] Recieved GameRichPresenceJoinRequested_t callback (friend-id = %llu, connect=%s).", param->m_steamIDFriend.ConvertToUint64(), param->m_rgchConnect);

	std::string full_string = param->m_rgchConnect;
	std::string prefix = "+connect_lobby ";
	if (full_string.substr(0, prefix.size()) != prefix)
	{
		DBG_LOG("[Steam Matchmaking] Ignored join request, invalid lobby connect string.");
		return;
	}

	std::string lobby_id = full_string.substr(prefix.size());
	CSteamID steam_id = StringHelper::To_U64(lobby_id);

	if (m_state == OnlineMatching_State::In_Lobby &&
		m_lobby_id == steam_id)
	{
		DBG_LOG("[Steam Matchmaking] Ignored join request, we are already in lobby.");
		return;
	}

	m_invite_pending	= true;
	m_invite_index++;
	m_invite_lobby_id	= steam_id;
}

Steamworks_OnlineUser* Steamworks_OnlineMatching::Get_Lobby_Member_By_SteamID(CSteamID id)
{
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_lobby_users.begin(); iter != m_lobby_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (user->Get_SteamID() == id)
		{
			return user;
		}
	}
	return NULL;
}

void Steamworks_OnlineMatching::Lobby_User_Left(Steamworks_OnlineUser* user)
{
	std::vector<Steamworks_OnlineUser*>::iterator iter = std::find(m_lobby_users.begin(), m_lobby_users.end(), user);
	if (iter != m_lobby_users.end())
	{
		m_lobby_users.erase(iter);

		// Let everyone know he's left!
		OnlineMatching_LobbyMemberLeftEventData param;
		param.User = user;
		On_Lobby_Member_Left.Fire(&param);

		// Enfore lobby member limit.
		Enforce_Lobby_Member_Limit();
	}
}

void Steamworks_OnlineMatching::Lobby_User_Joined(Steamworks_OnlineUser* user)
{
	std::vector<Steamworks_OnlineUser*>::iterator iter = std::find(m_lobby_users.begin(), m_lobby_users.end(), user);
	if (iter == m_lobby_users.end())
	{
		m_lobby_users.push_back(user);

		// Retrieve user information.
		if (!m_lobby_settings.IsOffline)
		{
			Retrieve_Member_Lobby_Settings(user);
		}

		// Let everyone know he's joined!
		OnlineMatching_LobbyMemberJoinedEventData param;
		param.User = user;
		On_Lobby_Member_Joined.Fire(&param);

		// Enfore lobby member limit.
		if (!m_lobby_settings.IsOffline)
		{
			Enforce_Lobby_Member_Limit();
		}
	}
}

void Steamworks_OnlineMatching::Enforce_Lobby_Member_Limit()
{
	// Only lobby host should do thsi.
	if (!Is_Lobby_Host())
	{
		return;
	}

	// This function basically enforces the lobbies member limit by kicking excess members
	// if we go over the limit, and updating the steamworks member limit dynamically.
	// This is required because we have the concept of "real" members as well as "guest" members,
	// that only exist etherially. 
	int total_members = 0;
	
	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_lobby_users.begin(); iter != m_lobby_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		total_members += 1 + user->Get_Guest_Count();
	}

	int slots_remaining = (m_lobby_settings.MaxPlayers - total_members);

	// If slots remaining are about 0, set the lobby limit.
	if (slots_remaining >= 0)
	{
		SteamMatchmaking()->SetLobbyMemberLimit(m_lobby_id, SteamMatchmaking()->GetNumLobbyMembers(m_lobby_id) + slots_remaining);
	}

	// If slots remaining is below 0, kick excess members.
	else if (slots_remaining < 0)
	{
		for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_lobby_users.end() - 1; iter != m_lobby_users.begin(); iter--)
		{
			Steamworks_OnlineUser* user = *iter;
			
			// Cannot kick ourselves :(.
			if (user->Is_Local())
			{
				continue;
			}

			// Kick out this user.
			Kick_Lobby_Member(user);

			// Increase slot counter.
			slots_remaining += 1 + user->Get_Guest_Count();
			if (slots_remaining >= 0)
			{
				break;
			}
		}
	}
}

OnlineMatching_State::Type Steamworks_OnlineMatching::Get_State()
{
	return m_state;
}

OnlineMatching_Error::Type Steamworks_OnlineMatching::Get_Error()
{
	OnlineMatching_Error::Type error = m_error;
	m_error = OnlineMatching_Error::NONE;
	Set_State(OnlineMatching_State::Idle);
	return error;
}

void Steamworks_OnlineMatching::Set_State(OnlineMatching_State::Type state)
{
	DBG_LOG("[Steam Matchmaking] Changing state to '%i'.", state);
	m_state = state;
}

void Steamworks_OnlineMatching::Set_Error(OnlineMatching_Error::Type error)
{
	DBG_LOG("[Steam Matchmaking] Changing error to '%i'.", error);
	m_error = error;
	Set_State(OnlineMatching_State::Error);
}

std::string Steamworks_OnlineMatching::Get_Error_Message(OnlineMatching_Error::Type error)
{
	switch (error)
	{
		case OnlineMatching_Error::Unknown:						return "#matching_error_internal_error";
		case OnlineMatching_Error::Invalid_State:				return "#matching_error_invalid_state";
		case OnlineMatching_Error::Offline:						return "#matching_error_offline";
		case OnlineMatching_Error::Timeout:						return "#matching_error_timeout";
		case OnlineMatching_Error::Access_Denied:				return "#matching_error_access_denied";
		case OnlineMatching_Error::Limit_Exceeded:				return "#matching_error_limit_exceeded";
		case OnlineMatching_Error::Kicked:						return "#matching_error_kicked";
		case OnlineMatching_Error::No_Lobby_Matches:			return "#matching_error_no_lobby_matches";
		case OnlineMatching_Error::Dosent_Exist:				return "#matching_error_dosent_exist";
		case OnlineMatching_Error::No_Permission:				return "#matching_error_no_permission";
		case OnlineMatching_Error::Full:						return "#matching_error_full";
		case OnlineMatching_Error::Banned:						return "#matching_error_banned";
		case OnlineMatching_Error::No_Permissions:				return "#matching_error_no_permissions";
		case OnlineMatching_Error::Clan_Disabled:				return "#matching_error_clan_disabled";
		case OnlineMatching_Error::Community_Lock:				return "#matching_error_community_lock";
		case OnlineMatching_Error::Blocked_By_Member:			return "#matching_error_blocked_by_member";
		case OnlineMatching_Error::Contains_Blocked_Member:		return "#matching_error_contains_blocked_member";
	}	

	DBG_ASSERT(false);
	return "matching_error_internal_error";
}

void Steamworks_OnlineMatching::Create_Lobby(OnlineMatching_LobbySettings settings)
{
	DBG_LOG("[Steam Matchmaking] Creating lobby - offline=%i, map_guid=%s, workshop-id=%llu, max_players=%i, is_private=%i, is_secure=%i, local=%i", settings.IsOffline, settings.MapGUID.c_str(), settings.MapWorkshopID, settings.MaxPlayers, settings.Private, settings.Secure, settings.Local);

	if (m_state != OnlineMatching_State::Idle)
	{
		DBG_LOG("[Steam Matchmaking] Unable to create lobby, in invalid state '%i'.", m_state);
		Set_Error(OnlineMatching_Error::Invalid_State);
		return;
	}

	settings.InGame = false;

	m_lobby_offline_connected = false;
	m_lobby_settings = settings;

	if (settings.IsOffline)
	{
		// Clear old lobby list.
		m_lobby_users.clear();

		std::vector<OnlineUser*> users = OnlinePlatform::Get()->Get_Local_Users();

		for (std::vector<OnlineUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
		{
			Steamworks_OnlineUser* sw_user = static_cast<Steamworks_OnlineUser*>(*iter);
			Lobby_User_Joined(sw_user);
		}

		Set_State(OnlineMatching_State::In_Lobby);
	}
	else
	{
		if (settings.Private == true)
		{
			SteamMatchmaking()->CreateLobby(k_ELobbyTypePrivate, settings.MaxPlayers);
		}
		else
		{
			SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, settings.MaxPlayers);
		}

		Set_State(OnlineMatching_State::Creating_Lobby);
	}
}

void Steamworks_OnlineMatching::Find_Lobby(std::string map_name, bool join_lobbies, bool join_games)
{
	DBG_LOG("[Steam Matchmaking] Finding lobby - mapguid=%s, join_lobbies=%i, join_games=%i", map_name.c_str(), join_lobbies, join_games);

	// Filter by map?
	if (map_name != "")
	{
		SteamMatchmaking()->AddRequestLobbyListStringFilter("map_guid", map_name.c_str(), k_ELobbyComparisonEqual);
	}

	// Filter by version.
	std::string version_string = StringHelper::Format("protocol_%i", NetManager::Get()->Get_Net_Version());
	SteamMatchmaking()->AddRequestLobbyListStringFilter("version", version_string.c_str(), k_ELobbyComparisonEqual);

	// Do we need to filter lobby types?
	if (join_lobbies == false || join_games == false)
	{
		if (join_lobbies == true)
		{	
			SteamMatchmaking()->AddRequestLobbyListStringFilter("in_game", "0", k_ELobbyComparisonEqual);
		}
		else
		{
			SteamMatchmaking()->AddRequestLobbyListStringFilter("in_game", "1", k_ELobbyComparisonEqual);
		}
	}

	// Find nearby lobbies.
	SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterDefault);

	// Nead a lobby with enough space for all users!
	SteamMatchmaking()->AddRequestLobbyListFilterSlotsAvailable(OnlinePlatform::Get()->Get_Local_User_Count());

	// Right! Lets request the lobby and register a callback. Lets hope for some nice matches :D
	m_lobby_match_request = SteamMatchmaking()->RequestLobbyList();
	m_lobby_match_list_callback.Set(m_lobby_match_request, this, &Steamworks_OnlineMatching::Callback_LobbyMatchList);

	Set_State(OnlineMatching_State::Finding_Lobby);
}

void Steamworks_OnlineMatching::Leave_Lobby()
{
	DBG_LOG("[Steam Matchmaking] Leaving game.");
	
	if (m_state == OnlineMatching_State::Finding_Lobby)
	{
		DBG_LOG("[Steam Matchmaking] Not in lobby, but trying to find lobby, canceling matching request.");
		if (m_lobby_match_request != k_uAPICallInvalid)
		{
			m_lobby_match_request = k_uAPICallInvalid;		
			m_lobby_match_list_callback.Cancel();
		}

		Set_State(OnlineMatching_State::Idle);
		return;
	}
	else if (m_state != OnlineMatching_State::In_Lobby)
	{
		DBG_LOG("[Steam Matchmaking] Unable to leave lobby, in invalid state '%i'.", m_state);
		//Set_Error(OnlineMatching_Error::Invalid_State); // Don't error, we should always assume this worked if possible.
		return;
	}

	if (!m_lobby_settings.IsOffline)
	{
		SteamMatchmaking()->LeaveLobby(m_lobby_id);
	}

	Set_State(OnlineMatching_State::Idle);
}

void Steamworks_OnlineMatching::Join_Invite_Lobby()
{
	DBG_LOG("[Steam Matchmaking] Attempting to join invite lobby '%llu'.", m_invite_lobby_id.ConvertToUint64());

	m_invite_pending = false;
	
	m_lobby_join_queue.clear();
	m_lobby_join_queue.push_back(m_invite_lobby_id);

	m_lobby_join_queue_started_by_find = false;

	Set_State(OnlineMatching_State::Joining_Lobby);
	Join_Next_Lobby_In_Queue();
}

bool Steamworks_OnlineMatching::Invite_Pending()
{
	return m_invite_pending;
}

int Steamworks_OnlineMatching::Invite_Index()
{
	return m_invite_index;
}

OnlineMatching_LobbySettings Steamworks_OnlineMatching::Get_Lobby_Settings()
{
	return m_lobby_settings;
}

void Steamworks_OnlineMatching::Set_Lobby_Settings(OnlineMatching_LobbySettings settings)
{	
	DBG_LOG("[Steam Matchmaking] Changing lobby settings.");

	m_lobby_settings = settings;

	if (!m_lobby_settings.IsOffline)
	{
		Store_Lobby_Settings();
	}
}

bool Steamworks_OnlineMatching::Get_Lobby_Settings_Changed()
{
	return m_lobby_settings_changed;
}

void Steamworks_OnlineMatching::Send_Lobby_Chat(std::string chat, bool system)
{
	if (m_state != OnlineMatching_State::In_Lobby)
	{
		return;
	}

	if (system == true)
	{
		DBG_LOG("[Steam Matchmaking] Sending system lobby chat message '%s'.", chat.c_str());
	}
	else
	{
		DBG_LOG("[Steam Matchmaking] Sending lobby chat message '%s'.", chat.c_str());
	}
	
	Steamworks_OnlineMatchingChatPacket packet;

	if (system == true)
	{
		packet.Type	= Steamworks_OnlineMatchingChatPacketType::System_Chat;
	}
	else
	{
		packet.Type	= Steamworks_OnlineMatchingChatPacketType::Chat;
	}

	int size = Min(Steamworks_OnlineMatchingChatPacket::MAX_DATA_SIZE - 1, chat.size());
	strncpy(packet.Data, chat.c_str(), size);
	packet.Data[size] = '\0';

	if (m_lobby_settings.IsOffline)
	{
		Process_Packet(&packet, static_cast<Steamworks_OnlineUser*>(OnlinePlatform::Get()->Get_Initial_User()));
		return;
	}

	SteamMatchmaking()->SendLobbyChatMsg(m_lobby_id, &packet, sizeof(packet));
}

std::vector<OnlineUser*> Steamworks_OnlineMatching::Get_Lobby_Members()
{
	std::vector<OnlineUser*> result;

	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_lobby_users.begin(); iter != m_lobby_users.end(); iter++)
	{
		result.push_back(*iter);
	}

	return result;
}

CSteamID Steamworks_OnlineMatching::Get_Lobby_SteamID()
{
	return m_lobby_id;
}

u64 Steamworks_OnlineMatching::Get_Lobby_ID()
{
	return m_lobby_id.ConvertToUint64();
}

void Steamworks_OnlineMatching::Kick_Lobby_Member(OnlineUser* user)
{
	if (m_state != OnlineMatching_State::In_Lobby)
	{
		return;
	}

	Steamworks_OnlineUser* sw_user = static_cast<Steamworks_OnlineUser*>(user);

	DBG_LOG("[Steam Matchmaking] Kicking lobby member '%s'.", user->Get_Username().c_str());
	
	// Kick out this user.
	Steamworks_OnlineMatchingChatPacket packet;
	packet.Type		= Steamworks_OnlineMatchingChatPacketType::Kicked;
	packet.SteamID	= sw_user->Get_SteamID();
	SteamMatchmaking()->SendLobbyChatMsg(m_lobby_id, &packet, sizeof(packet));
}

void Steamworks_OnlineMatching::Connect_Lobby_To_Server(OnlineMatching_Server server)
{
	if (m_state != OnlineMatching_State::In_Lobby)
	{
		return;
	}

	if (m_lobby_settings.IsOffline)
	{
		m_lobby_offline_connected = true;
		return;
	}
	
	Steamworks_OnlineUser* sw_user = static_cast<Steamworks_OnlineUser*>(server.User);

	DBG_LOG("[Steam Matchmaking] Connecting lobby to server '%llu'.", sw_user->Get_SteamID().ConvertToUint64());

	SteamMatchmaking()->SetLobbyGameServer(m_lobby_id, 0, 0, sw_user->Get_SteamID());
}

OnlineMatching_Server* Steamworks_OnlineMatching::Get_Lobby_Server()
{
	if (m_state != OnlineMatching_State::In_Lobby)
	{
		return NULL;
	}

	uint32 ip = 0;
	uint16 port = 0;
	CSteamID server_id;

	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());

	if (m_lobby_offline_connected && m_lobby_settings.IsOffline)
	{
		m_lobby_server.Name = "Remote Server";
		m_lobby_server.User = static_cast<Steamworks_OnlineUser*>(platform->Get_Initial_User());
		
		return &m_lobby_server;
	}

	if (!SteamMatchmaking()->GetLobbyGameServer(m_lobby_id, &ip, &port, &server_id))
	{
		return NULL;
	}
	
	Steamworks_OnlineUser* sw_user = platform->Get_User_By_SteamID(server_id);
	if (sw_user == NULL)
	{
		sw_user = platform->Register_Remote_User(server_id);
	}

	m_lobby_server.Name = "Remote Server";
	m_lobby_server.User = sw_user;
	
	return &m_lobby_server;
}

OnlineMatching_Server Steamworks_OnlineMatching::Get_Local_Server()
{
	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());
	Steamworks_OnlineServer* server = static_cast<Steamworks_OnlineServer*>(platform->Server());
	CSteamID server_id = server->Get_Server_SteamID();//static_cast<Steamworks_OnlineUser*>(platform->Get_Initial_User())->Get_SteamID();//server->Get_Server_SteamID();

	Steamworks_OnlineUser* sw_user = platform->Get_User_By_SteamID(server_id);
	if (sw_user == NULL)
	{
		sw_user = platform->Register_Remote_User(server_id);
	}

	OnlineMatching_Server ls;
	ls.Name = "Local Server";
	ls.User = sw_user;

	return ls;
}

void Steamworks_OnlineMatching::Send_Lobby_Packet(NetPacket* packet)
{	
	NetConnection* connection = NetManager::Get()->Client()->Get_Connection();

	for (std::vector<Steamworks_OnlineUser*>::iterator iter = m_lobby_users.begin(); iter != m_lobby_users.end(); iter++)
	{
		Steamworks_OnlineUser* user = *iter;
		if (!user->Is_Local())
		{
			connection->SendTo(packet, NET_CONNECTION_CHANNEL_LOBBY, true, user);
		}
	}
}

void Steamworks_OnlineMatching::Process_Packet(Steamworks_OnlineMatchingChatPacket* packet, Steamworks_OnlineUser* from)
{
	switch (packet->Type)
	{
	case Steamworks_OnlineMatchingChatPacketType::Chat:
		{
			DBG_LOG("[Steam Matchmaking] Recieved chat message from user '%s': %s", from->Get_Username().c_str(), packet->Data);

			OnlineMatching_LobbyChatEventData param;
			param.User = from;
			param.Text = packet->Data;

			On_Lobby_Chat.Fire(&param);

			break;
		}
	case Steamworks_OnlineMatchingChatPacketType::System_Chat:
		{
			DBG_LOG("[Steam Matchmaking] Recieved system chat message from user '%s': %s", from->Get_Username().c_str(), packet->Data);

			OnlineMatching_LobbyChatEventData param;
			param.User = NULL;
			param.Text = packet->Data;

			On_Lobby_Chat.Fire(&param);

			break;
		}
	case Steamworks_OnlineMatchingChatPacketType::Kicked:
		{
			Steamworks_OnlineUser* lobby_user = Get_Lobby_Member_By_SteamID(packet->SteamID);
			if (lobby_user != NULL)
			{
				DBG_LOG("[Steam Matchmaking] User '%s' was kicked.", from->Get_Username().c_str());
				Lobby_User_Left(lobby_user);	

				if (lobby_user->Is_Local() == true)
				{	
					SteamMatchmaking()->LeaveLobby(m_lobby_id);
					Set_Error(OnlineMatching_Error::Kicked);
				}
			}
			else
			{
				DBG_LOG("[Steam Matchmaking] Recieved kick message for user that no longer exists '%llu'.", packet->SteamID.ConvertToUint64());
			}

			break;
		}
	case Steamworks_OnlineMatchingChatPacketType::Voice_Chat:
		{
			Steamworks_OnlineUser* lobby_user = Get_Lobby_Member_By_SteamID(packet->SteamID);
			if (lobby_user != NULL)
			{
				OnlineVoiceChat::Get()->Recieve_Voice_Data(lobby_user, packet->Data, packet->DataSize, packet->OutputVolume);
			}
			else
			{
				DBG_LOG("[Steam Matchmaking] Recieved voice data for user that no longer exists '%llu'.", packet->SteamID.ConvertToUint64());
			}
			break;
		}
	}
}