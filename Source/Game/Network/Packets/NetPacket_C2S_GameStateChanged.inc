// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_GameStateChanged, false)
	PACKET_ENUM_FIELD(GameNetClient_GameState, New_State);
	PACKET_FIELD(int, Map_Load_Index);
END_PACKET()