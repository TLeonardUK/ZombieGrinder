// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(EditorRepositionObjectsState, false)	
	PACKET_FIELD		(s32,   Index);

	PACKET_FIELD		(float, X);
	PACKET_FIELD		(float, Y);
	PACKET_FIELD		(float, Angle);
	PACKET_FIELD		(Rect2D, Bounding_Box);
END_PACKET()

START_PACKET(NetPacket_C2S_EditorRepositionObjects, false)	
	PACKET_VECTOR_FIELD(EditorRepositionObjectsState, States);
	PACKET_VECTOR_FIELD(EditorRepositionObjectsState, Old_States);
END_PACKET()