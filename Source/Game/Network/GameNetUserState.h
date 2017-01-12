// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NET_USERSTATE_
#define _GAME_NET_USERSTATE_

#include "Game/Profile/Profile.h"

#include <vector>
#include <string>

class UserPacketState;
class GameNetUser;

// This state is replicated between server and clients, it dictates the users
// game state at a given point in time. Game specific variables should be
// replicated within this.

#define MAX_SCOREBOARD_COLUMNS 4

struct GameNetUserState
{
	MEMORY_ALLOCATOR(GameNetUserState, "Network");

private:
	GameNetUser* m_user;

public:
	GameNetUserState(GameNetUser* user);

	void Serialize(UserPacketState* state);
	void Deserialize(UserPacketState* state);

public:
	// Ping from client->server in MS.
	int Ping;

	// Team state.
	int Team_Index;

	// Coins!
	int Coins;

	// Game-Mode specific scoreboard values.
	//int Scoreboard_Values[MAX_SCOREBOARD_COLUMNS];

};

#endif

