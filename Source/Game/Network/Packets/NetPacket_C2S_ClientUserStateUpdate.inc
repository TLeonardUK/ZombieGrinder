// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_ClientUserStateUpdate, false)
	
	PACKET_FIELD(int, User_ID);
	PACKET_FIELD(Rect2D, Viewport);
	PACKET_FIELD(int, Level);

END_PACKET()