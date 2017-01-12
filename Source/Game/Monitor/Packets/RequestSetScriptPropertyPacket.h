// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_REQUEST_SET_SCRIPT_PROPERTY_PACKET_
#define _GAME_MONITOR_REQUEST_SET_SCRIPT_PROPERTY_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"

struct ScriptProperty;

class RequestSetScriptPropertyPacket : public BaseMonitorPacket
{
public:
	std::string InstanceName;
	std::string NewValue;
	std::string PropertyPath;
	std::string PropertyType;

public:
	RequestSetScriptPropertyPacket();

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif