// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_RESPONSE_GET_LOCALISATION_PACKET_
#define _GAME_MONITOR_RESPONSE_GET_LOCALISATION_PACKET_

#include "Game/Monitor/Packets/BaseMonitorPacket.h"

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"

struct LocalisationStringState
{
	std::string Name;
	std::string SourceLanguage;
	std::string Value;
};

class ResponseGetLocalisationPacket : public BaseMonitorPacket
{
public:
	std::vector<LocalisationStringState> States;

public:
	ResponseGetLocalisationPacket();

	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection);

};

#endif