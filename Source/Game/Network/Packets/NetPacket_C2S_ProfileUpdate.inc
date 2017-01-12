// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_ProfileUpdate, false)

	// ID of user who is having profile updated.
	PACKET_FIELD(u32, User_ID);

	// Profile data for user.
	PACKET_BLOB_FIELD(Profile_Data);

END_PACKET()