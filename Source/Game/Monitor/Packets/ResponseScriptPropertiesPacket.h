// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_RESPONSE_SCRIPT_PROPERTIES_PACKET_
#define _GAME_MONITOR_RESPONSE_SCRIPT_PROPERTIES_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

#include <vector>
#include <string>

struct ScriptProperty
{
	std::string Name;
	std::string TypeName;
	std::string CurrentValue;
	bool bReadOnly;

	std::vector<ScriptProperty> Children;
};

class ResponseScriptPropertiesPacket : public BaseMonitorPacket
{
private:
	void SerializeProperty(MonitorPacketSerializer& value, bool bEncoding, ScriptProperty& prop);

public:
	bool bScriptExists;
	ScriptProperty Root;

	ResponseScriptPropertiesPacket();

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif