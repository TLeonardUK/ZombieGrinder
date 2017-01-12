// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/Packets/GameStateMonitorPacket.h"

GameStateMonitorPacket::GameStateMonitorPacket()
	: BaseMonitorPacket("GameState")
{
}

void GameStateMonitorPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	BaseMonitorPacket::Serialize(value, bEncoding);

	SERIALIZE_STRING(Username, "Username");
	SERIALIZE_STRING(Map, "Map");
	SERIALIZE_STRING(Host, "Host");
}

void GameStateMonitorPacket::Process(MonitorServerConnection& connection)
{
}
