// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_MATCHING_
#define _ENGINE_ONLINE_MATCHING_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Network/NetAddress.h"

#include "Generic/Events/Event.h"

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

class OnlineUser;
class MapFileHandle;
class NetPacket;

struct OnlineMatching_ServerSource
{
	enum Type
	{
		Internet,
		LAN,
		Friends,
		Favorites,
		History,
		Reservable_Secure,
		Reservable_None_Secure,

		Unknown,
		COUNT
	};
};

struct OnlineMatching_State
{
	enum Type
	{
		Idle,
		Error,
		
		Creating_Lobby,
		Finding_Lobby,
		Joining_Lobby,
		
		In_Lobby,
		
		COUNT
	};
};

struct OnlineMatching_Error
{
	enum Type
	{
		NONE,
		Invalid_State,
		Offline,
		Timeout,
		Access_Denied,
		Limit_Exceeded,
		Kicked,
		No_Lobby_Matches,

		Dosent_Exist,
		No_Permission,
		Full,
		Banned,
		No_Permissions,
		Clan_Disabled,
		Community_Lock,
		Blocked_By_Member,
		Contains_Blocked_Member,

		Unknown
	};
};

class OnlineMatching_LobbySettings
{
	MEMORY_ALLOCATOR(OnlineMatching_LobbySettings, "Network");

public:
	std::string	MapGUID;
	u64			MapWorkshopID;
	int			MaxPlayers;
	bool		Private;
	bool		Secure;
	bool		Local;
	bool		InGame;
	bool		IsOffline;

};

class OnlineMatching_Server
{
	MEMORY_ALLOCATOR(OnlineMatching_Server, "Network");

public:
	OnlineMatching_ServerSource::Type Source;
	int								  Ping;
	std::string						  Name;
	bool							  Private;
	bool							  Secure;
	bool							  Dedicated;
	int								  PlayerCount;
	int								  MaxPlayerCount;
	std::string						  Map;
	NetAddress						  Address;
	OnlineUser*						  User;
};

class OnlineMatching_LobbyMemberJoinedEventData
{
	MEMORY_ALLOCATOR(OnlineMatching_LobbyMemberJoinedEventData, "Network");

public:
	OnlineUser* User;
};

class OnlineMatching_LobbyMemberLeftEventData
{
	MEMORY_ALLOCATOR(OnlineMatching_LobbyMemberLeftEventData, "Network");

public:
	OnlineUser* User;
};

class OnlineMatching_LobbyChatEventData
{
	MEMORY_ALLOCATOR(OnlineMatching_LobbyChatEventData, "Network");

public:
	OnlineUser* User;
	std::string Text;
};

class OnlineMatching_LobbyMemberDataUpdateEventData
{
	MEMORY_ALLOCATOR(OnlineMatching_LobbyMemberDataUpdateEventData, "Network");

public:
	OnlineUser* User;
};

class OnlineMatching : public Singleton<OnlineMatching>
{
	MEMORY_ALLOCATOR(OnlineMatching, "Network");

private:

public:
	Event<OnlineMatching_LobbyMemberJoinedEventData>		On_Lobby_Member_Joined;
	Event<OnlineMatching_LobbyMemberLeftEventData>			On_Lobby_Member_Left;
	Event<OnlineMatching_LobbyChatEventData>				On_Lobby_Chat;
	Event<OnlineMatching_LobbyMemberDataUpdateEventData>	On_Lobby_Member_Data_Updated;
	
public:
	virtual ~OnlineMatching() {} 

	static OnlineMatching* Create();
	
	// Connects to online platform.
	virtual bool Initialize() = 0;

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time) = 0;

	// Server listing
	virtual void Refresh_Server_List(OnlineMatching_ServerSource::Type source) = 0;
	virtual void Cancel_Server_List_Refresh(OnlineMatching_ServerSource::Type source) = 0;
	virtual std::vector<OnlineMatching_Server> Get_Server_List(OnlineMatching_ServerSource::Type source) = 0;
	virtual bool Has_Server_List_Updated(OnlineMatching_ServerSource::Type source) = 0;
	virtual bool Is_Server_List_Refreshing(OnlineMatching_ServerSource::Type source) = 0;

	// State settings.
	virtual OnlineMatching_State::Type Get_State() = 0;
	virtual OnlineMatching_Error::Type Get_Error() = 0;
	virtual std::string Get_Error_Message(OnlineMatching_Error::Type error) = 0;

	// Lobby shit!
	virtual void Create_Lobby								(OnlineMatching_LobbySettings settings) = 0;
	virtual void Find_Lobby									(std::string map_name, bool join_lobbies, bool join_games) = 0;
	virtual void Leave_Lobby								() = 0;	
	virtual void Join_Invite_Lobby							() = 0;
	virtual bool Invite_Pending								() = 0;
	virtual int Invite_Index								() = 0;
	virtual OnlineMatching_LobbySettings Get_Lobby_Settings () = 0;
	virtual void						 Set_Lobby_Settings (OnlineMatching_LobbySettings settings) = 0;
	virtual bool Get_Lobby_Settings_Changed					() = 0;
	virtual bool Is_Lobby_Host								() = 0;
	virtual OnlineUser* Get_Lobby_Host						() = 0;
	virtual void Send_Lobby_Chat							(std::string chat, bool system = false) = 0;
	virtual std::vector<OnlineUser*> Get_Lobby_Members		() = 0;
	virtual void Kick_Lobby_Member							(OnlineUser*) = 0;
	virtual u64 Get_Lobby_ID								() = 0;
	virtual void Connect_Lobby_To_Server					(OnlineMatching_Server server) = 0;
	virtual OnlineMatching_Server Get_Local_Server			() = 0;
	virtual OnlineMatching_Server* Get_Lobby_Server			() = 0;	
	//virtual void Send_Voice_Data							(const char* data, int data_size, float output_volume) = 0;

	virtual void Send_Lobby_Packet							(NetPacket* packet) = 0;

};

#endif

