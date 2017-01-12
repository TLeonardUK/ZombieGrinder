// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_RESPONSE_ACTIVE_SCRIPTS_PACKET_
#define _GAME_MONITOR_RESPONSE_ACTIVE_SCRIPTS_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

#include <vector>
#include <string>

struct ActiveScriptState
{
	std::string Name;
	std::string ClassName;
};

class ResponseActiveScriptsPacket : public BaseMonitorPacket
{
public:
	std::vector<ActiveScriptState> States;

	ResponseActiveScriptsPacket();

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif