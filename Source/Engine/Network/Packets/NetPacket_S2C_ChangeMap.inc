// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

START_PACKET(NetPacket_S2C_ChangeMap, false)
	PACKET_FIELD(std::string, Map_GUID);
	PACKET_FIELD(u64,		  Map_Workshop_ID);
	PACKET_FIELD(int,		  Map_Seed);
	PACKET_FIELD(int,		  Map_Load_Index);
	PACKET_FIELD(int,	      Map_Dungeon_Level_Index);
END_PACKET()