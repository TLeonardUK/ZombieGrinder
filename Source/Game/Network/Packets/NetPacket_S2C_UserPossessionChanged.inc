// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_S2C_UserPossessionChanged, false)

	PACKET_FIELD(s32,	User_ID);
	PACKET_FIELD(s32,	Actor_ID);

END_PACKET()