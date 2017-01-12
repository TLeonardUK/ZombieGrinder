// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_CallVote, false)

	PACKET_FIELD(u32,			Vote_Type);
	PACKET_FIELD(u16,			User_ID);
	PACKET_FIELD(std::string,	Extra);

END_PACKET()