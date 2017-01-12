// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Network/Packets/GamePackets.h"

#define PACKET_LIST_FILE "Game/Network/Packets/GamePacketsList.inc"
#define PACKET_TYPE_REGISTER_FUNCTION Register_Game_Packets	
#include "Engine/Network/Packets/PacketDefiner.inc"
#undef PACKET_TYPE_REGISTER_FUNCTION
#undef PACKET_LIST_FILE