// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NETWORK_NET_CONNECTION_TYPES_
#define _ENGINE_NETWORK_NET_CONNECTION_TYPES_

#define MAX_NET_CONNECTION_CHANNELS					6

#define NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE		0	// Reliable system-level messages (connect/disconnect/etc)
#define NET_CONNECTION_CHANNEL_VOICE_UNRELIABLE		1	// Unreliable voice-chat messages.
#define NET_CONNECTION_CHANNEL_PING_UNRELIABLE		2	// Unreliable ping-pong messages.
#define NET_CONNECTION_CHANNEL_LOBBY				3	// Lobby messages (mainly voice-chat).
#define NET_CONNECTION_CHANNEL_STATE_UNRELIABLE		4	// Unreliable world-state messages.
#define NET_CONNECTION_CHANNEL_FILE_TRANSFER		5	// Reliable file transfers.

struct NetConnection_State
{
	enum Type
	{
		Idle,

		Connecting,
		Connected,
		Listening,
		Disconnecting,
		Disconnected,

		Error
	};
};

struct NetConnection_Error
{
	enum Type
	{
		NONE,
		Unknown,
		Unavailable,
		Timeout,

		Wrong_Version,		
		Server_Full,
#ifdef OPT_PREMIUM_ACCOUNTS
		Server_Premium,
#endif
		Banned,
		Kicked,
		Lost_Connection,
		Duplicate_Connection,
		Transmission_Error,
		Server_Reserved,
		Invalid_Ticket,
		Client_Mod_Missmatch,
		Server_Mod_Missmatch
	};
};

#endif

