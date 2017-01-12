// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_REQUEST_ACTIVE_SCRIPTS_PACKET_
#define _GAME_MONITOR_REQUEST_ACTIVE_SCRIPTS_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

class RequestActiveScriptsPacket : public BaseMonitorPacket
{
public:
	RequestActiveScriptsPacket();

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif