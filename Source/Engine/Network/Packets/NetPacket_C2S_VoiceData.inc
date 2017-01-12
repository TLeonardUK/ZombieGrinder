// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_VoiceData, true)
	PACKET_FIELD(u64,		 User_ID);
	PACKET_FIELD(float,		 Output_Volume);
	PACKET_FIELD(DataBuffer, Data);
END_PACKET()