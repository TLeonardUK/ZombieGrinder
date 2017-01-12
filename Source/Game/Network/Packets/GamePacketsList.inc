// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// Client-2-Server
#include "Game/Network/Packets/NetPacket_C2S_GameStateChanged.inc"
#include "Game/Network/Packets/NetPacket_C2S_ChatMessage.inc"
#include "Game/Network/Packets/NetPacket_C2S_CallVote.inc"
#include "Game/Network/Packets/NetPacket_C2S_Vote.inc"
#include "Game/Network/Packets/NetPacket_C2S_ProfileUpdate.inc"
#include "Game/Network/Packets/NetPacket_C2S_ClientUserStateUpdate.inc"
#include "Game/Network/Packets/NetPacket_C2S_RequestTeamChange.inc"

// Editor stuff.
#include "Game/Network/Packets/NetPacket_C2S_ModifyTilemap.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorRequestUndo.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorRequestRedo.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorRepositionObjects.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorDeleteObjects.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorCreateObjects.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorChangeObjectState.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorChangeMapState.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorResetMap.inc"
#include "Game/Network/Packets/NetPacket_C2S_EditorOpenMap.inc"

// Server-2-Client
#include "Game/Network/Packets/NetPacket_S2C_ServerStateChanged.inc"
#include "Game/Network/Packets/NetPacket_S2C_UserStateUpdate.inc"
#include "Game/Network/Packets/NetPacket_S2C_UpdateVote.inc"
#include "Game/Network/Packets/NetPacket_S2C_Replication.inc"
#include "Game/Network/Packets/NetPacket_S2C_UserPossessionChanged.inc"


