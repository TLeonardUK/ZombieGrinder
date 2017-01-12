// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#define MAX_AUTH_TICKET_LENGTH 512

START_PACKET(NetPacket_EnforcedMod, false)
	PACKET_FIELD(std::string,	GUID);
	PACKET_FIELD(std::string,	Workshop_Title);
	PACKET_FIELD(u64,			Workshop_ID);
END_PACKET()

START_PACKET(NetPacket_C2S_Connect, false)
	PACKET_FIELD		(unsigned int,							Version);
	PACKET_FIELD		(unsigned char,							Local_User_Count);
	PACKET_FIELD		(std::string,							Short_Country_Name)		
	PACKET_FIELD		(u64,									Lobby_ID);
#ifdef OPT_PREMIUM_ACCOUNTS
	PACKET_FIELD		(bool,									Premium_Account);
#endif
	PACKET_ARRAY_FIELD	(std::string,	MAX_LOCAL_PLAYERS,		Usernames);

	PACKET_VECTOR_FIELD	(NetPacket_EnforcedMod,	Enforced_Mods);
 
	// This is pretty much steam-specific.
	PACKET_ARRAY_FIELD	(char,			MAX_AUTH_TICKET_LENGTH,	Auth_Ticket);
	PACKET_FIELD		(u32,									Auth_Ticket_Length);
END_PACKET()