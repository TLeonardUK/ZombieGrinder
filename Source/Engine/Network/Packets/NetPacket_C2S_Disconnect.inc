// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_Disconnect, false)
	PACKET_ENUM_FIELD(NetConnection_Error, Result);
END_PACKET()