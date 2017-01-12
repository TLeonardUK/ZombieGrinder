// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_NameChanged, false)
	PACKET_FIELD		(u16,			Net_ID)
	PACKET_FIELD		(std::string,	Username);
END_PACKET()