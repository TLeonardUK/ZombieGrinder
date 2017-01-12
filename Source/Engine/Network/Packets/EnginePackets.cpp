// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/Packets/EnginePackets.h"

#define PACKET_LIST_FILE "Engine/Network/Packets/EnginePacketList.inc"
#define PACKET_TYPE_REGISTER_FUNCTION Register_Engine_Packets	
#include "Engine/Network/Packets/PacketDefiner.inc"
#undef PACKET_TYPE_REGISTER_FUNCTION
#undef PACKET_LIST_FILE