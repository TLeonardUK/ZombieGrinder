// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NETWORK_GAME_PACKETS_
#define _GAME_NETWORK_GAME_PACKETS_

#include "Engine/Network/NetConnection.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Generic/Types/DataBuffer.h"

#include "Game/Network/GameNetClient_GameState.h"
#include "Game/Network/GameNetManager.h"

#include "Generic/Types/Color.h"
#include "Generic/Types/Rect2D.h"

class Stream;

#define PACKET_LIST_FILE "Game/Network/Packets/GamePacketsList.inc"
#define PACKET_TYPE_REGISTER_FUNCTION Register_Game_Packets	
#include "Engine/Network/Packets/PacketDeclarer.inc"
#undef PACKET_TYPE_REGISTER_FUNCTION
#undef PACKET_LIST_FILE

#endif