// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#include "Game/Network/GameNetUserState.h"

START_PACKET(UserPacketState, false)

	// ID of user this state belongs to.
	PACKET_FIELD(u16, User_ID)

	// Serialized state data.
	PACKET_FIELD(int, State_Data_Version);
	PACKET_FIELD(DataBuffer, State_Data);

	// Users current ping-time.
	PACKET_FIELD(u8, Ping)

	// Team user belongs to.
	PACKET_FIELD(u32, Team_Index);

	// Level for scoreboard.
	PACKET_FIELD(u16, Level);
	
	// Coins!
	PACKET_FIELD(u32, Coins);

	// Game state.
	PACKET_FIELD(u8, Game_State)

END_PACKET()

START_PACKET(NetPacket_S2C_UserStateUpdate, false)
	
	PACKET_VECTOR_FIELD(UserPacketState, States);

END_PACKET()