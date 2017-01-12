// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_REQUEST_SET_LOCALISATION_PACKET_
#define _GAME_MONITOR_REQUEST_SET_LOCALISATION_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"

class RequestSetLocalisationPacket : public BaseMonitorPacket
{
public:
	std::string Name;
	std::string SourceLanguage;
	std::string Value;

public:
	RequestSetLocalisationPacket();

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif