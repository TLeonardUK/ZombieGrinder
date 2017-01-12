// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_S2C_UpdateVote, false)

	PACKET_FIELD(bool,			Active);
	PACKET_FIELD(bool,			Outcome);
	
	PACKET_FIELD(u32,			Vote_Type);
	PACKET_FIELD(u16,			User_ID);
	PACKET_FIELD(std::string,	Extra);

	PACKET_FIELD(u8,			Total_Votes);
	PACKET_FIELD(u8,			Pass_Votes);
	PACKET_FIELD(u8,			Fail_Votes);

END_PACKET()