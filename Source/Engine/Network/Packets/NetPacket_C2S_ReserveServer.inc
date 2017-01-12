// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_C2S_ReserveServer, false)

	// TODO: Blaaaaaah, this is kinda steam-specific? Needs abstracting.
	PACKET_FIELD(u64,			LobbyID); 

	PACKET_FIELD(std::string,	MapGUID);
	PACKET_FIELD(u64,			MapWorkshopID); 
	PACKET_FIELD(int,			MaxPlayers);
	PACKET_FIELD(bool,			Private); 

END_PACKET()