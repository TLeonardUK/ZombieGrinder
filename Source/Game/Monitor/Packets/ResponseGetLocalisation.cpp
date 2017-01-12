// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/MonitorServer.h"
#include "Game/Monitor/Packets/RequestGetLocalisation.h"
#include "Game/Monitor/Packets/ResponseGetLocalisation.h"

#include "Game/Scripts/GameVirtualMachine.h"
#include "Game/Scripts/ScriptEventListener.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Generic/Helper/StringHelper.h"

ResponseGetLocalisationPacket::ResponseGetLocalisationPacket()
	: BaseMonitorPacket("ResponseActiveScripts")
{
}

void ResponseGetLocalisationPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	BaseMonitorPacket::Serialize(value, bEncoding);
}

void ResponseGetLocalisationPacket::Process(MonitorServerConnection& connection)
{
}
