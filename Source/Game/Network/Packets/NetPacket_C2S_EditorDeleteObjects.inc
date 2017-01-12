// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(EditorDeleteObjectID, false)
	PACKET_FIELD(s32, ID);
END_PACKET()

START_PACKET(NetPacket_C2S_EditorDeleteObjects, false)
	PACKET_VECTOR_FIELD(EditorDeleteObjectID, IDs);
END_PACKET()