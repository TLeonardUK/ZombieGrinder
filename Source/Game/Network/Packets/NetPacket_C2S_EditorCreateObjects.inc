// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(EditorCreateObjectState, false)
	PACKET_FIELD(std::string, Class_Name);
	PACKET_FIELD(DataBuffer,  Serialized_Data);
	PACKET_FIELD(int,		  Serialized_Data_Version);
END_PACKET()

START_PACKET(NetPacket_C2S_EditorCreateObjects, false)	
	PACKET_FIELD(int, Creator_ID);
	PACKET_FIELD(float, Origin_X);
	PACKET_FIELD(float, Origin_Y);
	PACKET_FIELD(int, Origin_Layer);
	PACKET_VECTOR_FIELD(EditorCreateObjectState, States);
END_PACKET()