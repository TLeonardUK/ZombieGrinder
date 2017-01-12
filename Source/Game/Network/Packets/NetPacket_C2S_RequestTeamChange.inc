// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_RequestTeamChange, false)

	PACKET_FIELD(u16,	User_ID);
	PACKET_FIELD(s32,	Team_Index);

END_PACKET()