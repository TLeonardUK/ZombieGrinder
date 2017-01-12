// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NETWORK_ENGINE_PACKETS_
#define _ENGINE_NETWORK_ENGINE_PACKETS_

#include "Engine/Network/NetConnectionTypes.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Generic/Types/DataBuffer.h"

class Stream;

#define PACKET_LIST_FILE "Engine/Network/Packets/EnginePacketList.inc"
#define PACKET_TYPE_REGISTER_FUNCTION Register_Engine_Packets	
#include "Engine/Network/Packets/PacketDeclarer.inc"
#undef PACKET_TYPE_REGISTER_FUNCTION
#undef PACKET_LIST_FILE

#endif