// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_REQUEST_SCRIPT_PROPERTIES_PACKET_
#define _GAME_MONITOR_REQUEST_SCRIPT_PROPERTIES_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"

struct ScriptProperty;

class RequestScriptPropertiesPacket : public BaseMonitorPacket
{
public:
	std::string InstanceName;

public:
	RequestScriptPropertiesPacket();

	void WriteObject(ScriptProperty& parent, CVMValue value, int depth);
	void WriteArray(ScriptProperty& parent, CVMValue value, CVMDataType* elementType, int depth);
	void WriteValue(ScriptProperty& parent, CVMLinkedSymbol* symbol, CVMValue value, int depth);

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif