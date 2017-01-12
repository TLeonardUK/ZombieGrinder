// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/MonitorServer.h"
#include "Game/Monitor/Packets/ResponseActiveScriptsPacket.h"
#include "Generic/Helper/StringHelper.h"

ResponseActiveScriptsPacket::ResponseActiveScriptsPacket()
	: BaseMonitorPacket("ResponseActiveScripts")
{
}

void ResponseActiveScriptsPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	BaseMonitorPacket::Serialize(value, bEncoding);

	SERIALIZE_ARRAY_START(States, "States", ActiveScriptState, Element)

		SERIALIZE_STRING(Element.Name, "Name")
		SERIALIZE_STRING(Element.ClassName, "ClassName")

	SERIALIZE_ARRAY_END(States, "States", ActiveScriptState, Element)
}

void ResponseActiveScriptsPacket::Process(MonitorServerConnection& connection)
{
}
