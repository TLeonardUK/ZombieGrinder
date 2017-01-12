// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_REQUEST_GET_LOCALISATION_PACKET_
#define _GAME_MONITOR_REQUEST_GET_LOCALISATION_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"

class RequestGetLocalisationPacket : public BaseMonitorPacket
{
public:

public:
	RequestGetLocalisationPacket();

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif