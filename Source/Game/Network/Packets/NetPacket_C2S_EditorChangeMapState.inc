// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_EditorChangeMapState, false)	
	PACKET_FIELD(DataBuffer,	Map_Data);
	PACKET_FIELD(DataBuffer,	Old_Map_Data);
END_PACKET()