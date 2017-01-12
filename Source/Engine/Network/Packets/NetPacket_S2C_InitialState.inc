// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_S2C_InitialState, false)
	PACKET_ARRAY_FIELD(u16,	MAX_LOCAL_PLAYERS, Net_IDs);
END_PACKET()