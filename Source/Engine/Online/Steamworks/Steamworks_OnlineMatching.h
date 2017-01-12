// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_MATCHING_
#define _ENGINE_STEAMWORKS_ONLINE_MATCHING_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Online/OnlineMatching.h"

#include "Engine/Network/NetPacket.h"
#include "Engine/Network/Packets/PacketDispatcher.h"

#include "public/steam/steam_api.h"

#include <string>
#include <vector>

class Steamworks_OnlineUser;

class MapFileHandle;
class NetPacket;

struct Steamworks_OnlineMatchingChatPacketType
{
	enum Type
	{
		Chat,
		System_Chat,
		Kicked,
		Voice_Chat
	};
};

struct Steamworks_OnlineMatchingChatPacket
{
	enum
	{
		MAX_DATA_SIZE = 1024 * 2
	};

	Steamworks_OnlineMatchingChatPacketType::Type	Type;
	CSteamID										SteamID;
	float											OutputVolume;
	int												DataSize;
	char											Data[MAX_DATA_SIZE];
};

class Steamworks_OnlineMatching : public OnlineMatching,
								  public ISteamMatchmakingServerListResponse
{
	MEMORY_ALLOCATOR(Steamworks_OnlineMatching, "Network");

private:
	int									m_appid;

	HServerListRequest					m_server_list_requests[OnlineMatching_ServerSource::COUNT];
	bool								m_server_list_updated[OnlineMatching_ServerSource::COUNT];

	OnlineMatching_State::Type			m_state;
	OnlineMatching_Error::Type			m_error;

	CSteamID							m_lobby_id;
	OnlineMatching_LobbySettings		m_lobby_settings;
	bool								m_lobby_settings_changed;
	SteamAPICall_t						m_lobby_match_request;	
	OnlineMatching_Server				m_lobby_server;

	std::vector<CSteamID>				m_lobby_join_queue;
	bool								m_lobby_join_queue_started_by_find;
	uint32								m_last_lobby_join_error;

	std::vector<Steamworks_OnlineUser*>	m_lobby_users;
	
	bool								m_invite_pending;
	CSteamID							m_invite_lobby_id;
	int									m_invite_index;

	bool								m_lobby_offline_connected;

protected:
	// General callbacks.
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyInvite,			LobbyInvite_t,				m_lobby_invite_callback);
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyEnter,			LobbyEnter_t,				m_lobby_enter_callback);
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyDataUpdate,		LobbyDataUpdate_t,			m_lobby_data_update_callback);
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyChatUpdate,		LobbyChatUpdate_t,			m_lobby_chat_update_callback);
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyChatMessage,		LobbyChatMsg_t,				m_lobby_chat_message_callback);
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyGameCreated,		LobbyGameCreated_t,			m_lobby_game_created_callback);
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyKicked,			LobbyKicked_t,				m_lobby_kicked_callback);
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyCreated,			LobbyCreated_t,				m_lobby_created_callback);
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_LobbyJoinRequested,	GameLobbyJoinRequested_t,	m_lobby_join_requested_callback);	
	STEAM_CALLBACK(Steamworks_OnlineMatching,			Callback_GameJoinRequested,		GameRichPresenceJoinRequested_t,	m_game_join_requested_callback);	

	

	// Call-specific callbacks (ones we want the ability to register/cancel to specific requests).
	CCallResult<Steamworks_OnlineMatching, LobbyMatchList_t> m_lobby_match_list_callback;
	void Callback_LobbyMatchList(LobbyMatchList_t*, bool ioFailure);

	// Converts a HServerListRequest into a OnlineMatching_ServerSource
	OnlineMatching_ServerSource::Type Get_Source_From_Request(HServerListRequest hRequest);

	// Callbacks for server-lists.
	void ServerResponded(HServerListRequest hRequest, int iServer); 
	void ServerFailedToRespond(HServerListRequest hRequest, int iServer); 
	void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response); 

	// Change state.
	void Set_State(OnlineMatching_State::Type state);

	// Change error.
	void Set_Error(OnlineMatching_Error::Type error);

	// Lobby stuff
	void Store_Lobby_Settings();
	void Retrieve_Lobby_Settings();
	void Store_Member_Lobby_Settings();
	void Retrieve_Member_Lobby_Settings(Steamworks_OnlineUser* user);
	void Enforce_Lobby_Member_Limit();

	// Joining lobbies.
	void Join_Next_Lobby_In_Queue();

	// Lobby state-changes.
	Steamworks_OnlineUser* Get_Lobby_Member_By_SteamID(CSteamID id);
	void Lobby_User_Left(Steamworks_OnlineUser* user);
	void Lobby_User_Joined(Steamworks_OnlineUser* user);

public:
	
	// Saves some type-casting.
	static Steamworks_OnlineMatching* Get()
	{
		return static_cast<Steamworks_OnlineMatching*>(OnlineMatching::Get());
	}
	
	Steamworks_OnlineMatching();
	~Steamworks_OnlineMatching();
	
	// Connects to online platform.
	bool Initialize();
	
	// Tick tock tick tock
	void Tick(const FrameTime& time);

	// Server listing
	void Refresh_Server_List(OnlineMatching_ServerSource::Type source);
	void Cancel_Server_List_Refresh(OnlineMatching_ServerSource::Type source);
	std::vector<OnlineMatching_Server> Get_Server_List(OnlineMatching_ServerSource::Type source);
	bool Has_Server_List_Updated(OnlineMatching_ServerSource::Type source);
	bool Is_Server_List_Refreshing(OnlineMatching_ServerSource::Type source);

	// State settings.
	OnlineMatching_State::Type Get_State();
	OnlineMatching_Error::Type Get_Error();
	std::string Get_Error_Message(OnlineMatching_Error::Type error);

	// Creating server.
	void Create_Lobby(OnlineMatching_LobbySettings settings);
	void Find_Lobby(std::string map_name, bool join_lobbies, bool join_games);

	void Join_Invite_Lobby();
	bool Invite_Pending();
	int Invite_Index();

	void Leave_Lobby();
	OnlineMatching_LobbySettings Get_Lobby_Settings();
	void Set_Lobby_Settings(OnlineMatching_LobbySettings settings);
	bool Get_Lobby_Settings_Changed();
	bool Is_Lobby_Host();
	OnlineUser* Get_Lobby_Host();
	void Send_Lobby_Chat(std::string chat, bool system);
	std::vector<OnlineUser*> Get_Lobby_Members();
	void Kick_Lobby_Member(OnlineUser* user);

	CSteamID Get_Lobby_SteamID();
	u64 Get_Lobby_ID();
	
	void Connect_Lobby_To_Server(OnlineMatching_Server server);
	OnlineMatching_Server Get_Local_Server();
	OnlineMatching_Server* Get_Lobby_Server();

	//void Send_Voice_Data(const char* data, int data_size, float output_volume);
	
	void Send_Lobby_Packet(NetPacket* packet);

	void Process_Packet(Steamworks_OnlineMatchingChatPacket* packet, Steamworks_OnlineUser* from);

};

#endif

