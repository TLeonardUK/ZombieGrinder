// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(GameNetServerState, false)

	// Name of the server.
	PACKET_FIELD(std::string, Name)

	// Name of the "host" of the server. Usually an IP (for dedi) or a players name (for local).
	PACKET_FIELD(std::string, Host_Name)
	
	// Time since the map started. We actually only sync this the first time a map is changed
	// from then on the client just increments it manually.
	PACKET_FIELD(float,		  Elapsed_Map_Time)

	// In editor mode?
	PACKET_FIELD(bool,		  In_Editor_Mode);

	// Network ID of the active game-mode object.
	PACKET_FIELD(u32,		  Game_Mode_Net_ID);

END_PACKET()

START_PACKET(NetPacket_S2C_ServerStateChanged, false)

	PACKET_SERIALIZABLE_FIELD(GameNetServerState, State);

END_PACKET()