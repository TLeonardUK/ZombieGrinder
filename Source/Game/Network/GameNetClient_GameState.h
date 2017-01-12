// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NETWORK_GAME_NET_CLIENT_GAME_STATE_
#define _GAME_NETWORK_GAME_NET_CLIENT_GAME_STATE_

struct GameNetClient_GameState
{
	enum Type
	{
		Idle,
		Loading,
		SendingClientInfo,
		SyncingWithServer,
		WaitingForStart,
		InGame,
		InCutscene,
		LeftGame
	};
};

#endif