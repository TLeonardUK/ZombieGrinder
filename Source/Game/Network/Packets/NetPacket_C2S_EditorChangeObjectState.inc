// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_EditorChangeObjectState, false)	
	PACKET_FIELD(s32,			ID);
	PACKET_FIELD(std::string,	Class_Name);
	
	PACKET_FIELD(DataBuffer,	Serialized_Data);
	PACKET_FIELD(int,			Serialized_Data_Version);
	
	PACKET_FIELD(DataBuffer,	Old_Serialized_Data);
	PACKET_FIELD(int,			Old_Serialized_Data_Version);
END_PACKET()