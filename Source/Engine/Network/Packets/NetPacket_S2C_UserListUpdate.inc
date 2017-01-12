// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET_ENUM(UserListUpdateType)
	PACKET_ENUM_ELEMENT(Join)		// Players joined
	PACKET_ENUM_ELEMENT(Leave)		// Players left
	PACKET_ENUM_ELEMENT(Full)		// Full player list update
END_PACKET_ENUM()

START_PACKET(UserListState, false)
	PACKET_FIELD		(u64,							User_ID);
	PACKET_FIELD		(u16,							Net_ID);
	PACKET_FIELD		(u16,							Local_Index);
	PACKET_FIELD		(std::string,					Username);
	PACKET_FIELD		(std::string,					Short_Country_Name);
#ifdef OPT_PREMIUM_ACCOUNTS
	PACKET_FIELD		(bool,							Premium);
#endif
END_PACKET()

START_PACKET(NetPacket_S2C_UserListUpdate, false)
	PACKET_ENUM_FIELD	(UserListUpdateType,			Update_Type);
	PACKET_VECTOR_FIELD	(UserListState,					States);
END_PACKET()