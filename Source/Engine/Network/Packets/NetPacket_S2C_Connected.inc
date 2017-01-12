// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_S2C_Connected, false)
	PACKET_ENUM_FIELD(NetConnection_Error, Result);
	PACKET_VECTOR_FIELD(NetPacket_EnforcedMod, Missing_Server_Mods);
	PACKET_VECTOR_FIELD(NetPacket_EnforcedMod, Missing_Client_Mods);
END_PACKET()