// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_GAME_STATE_PACKET_
#define _GAME_MONITOR_GAME_STATE_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

class GameStateMonitorPacket : public BaseMonitorPacket
{
public:
	std::string Username;
	std::string Map;
	std::string Host;

	GameStateMonitorPacket();

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif