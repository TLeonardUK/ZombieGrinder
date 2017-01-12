// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// Client-2-Server
#include "Engine/Network/Packets/NetPacket_C2S_Disconnect.inc"
#include "Engine/Network/Packets/NetPacket_C2S_Connect.inc"
#include "Engine/Network/Packets/NetPacket_C2S_Pong.inc"
#include "Engine/Network/Packets/NetPacket_C2S_ReserveServer.inc"
#include "Engine/Network/Packets/NetPacket_C2S_VoiceData.inc"
#include "Engine/Network/Packets/NetPacket_C2S_NameChanged.inc"

#include "Engine/Network/Packets/NetPacket_C2S_FileTransferStart.inc"
#include "Engine/Network/Packets/NetPacket_C2S_FileTransferChunk.inc"
#include "Engine/Network/Packets/NetPacket_C2S_FileTransferFinish.inc"
#include "Engine/Network/Packets/NetPacket_C2S_FileTransferFinishAck.inc"

// Server-2-Client
#include "Engine/Network/Packets/NetPacket_S2C_Connected.inc"
#include "Engine/Network/Packets/NetPacket_S2C_Ping.inc"
#include "Engine/Network/Packets/NetPacket_S2C_ReservationResult.inc"
#include "Engine/Network/Packets/NetPacket_S2C_UserListUpdate.inc"
#include "Engine/Network/Packets/NetPacket_S2C_InitialState.inc"
#include "Engine/Network/Packets/NetPacket_S2C_ChangeMap.inc"
#include "Engine/Network/Packets/NetPacket_S2C_Kick.inc"

