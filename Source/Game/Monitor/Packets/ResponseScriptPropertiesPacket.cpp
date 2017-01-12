// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/MonitorServer.h"
#include "Game/Monitor/Packets/ResponseScriptPropertiesPacket.h"
#include "Generic/Helper/StringHelper.h"

ResponseScriptPropertiesPacket::ResponseScriptPropertiesPacket()
	: BaseMonitorPacket("ResponseScriptProperties")
{
}

void ResponseScriptPropertiesPacket::SerializeProperty(MonitorPacketSerializer& value, bool bEncoding, ScriptProperty& prop)
{
	SERIALIZE_STRING(prop.Name, "Name");
	SERIALIZE_STRING(prop.TypeName, "TypeName");
	SERIALIZE_BOOL(prop.bReadOnly, "bReadOnly");
	SERIALIZE_STRING(prop.CurrentValue, "CurrentValue");
	SERIALIZE_ARRAY_START(prop.Children, "Children", ScriptProperty, Child);
	
		SerializeProperty(value, bEncoding, Child);

	SERIALIZE_ARRAY_END(prop.Children, "Children", ScriptProperty, Child);
}

void ResponseScriptPropertiesPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	BaseMonitorPacket::Serialize(value, bEncoding);

	SERIALIZE_BOOL(bScriptExists, "bScriptExists");
	SERIALIZE_OBJECT_START(Root, "Root");
		SerializeProperty(value, bEncoding, this->Root);
	SERIALIZE_OBJECT_END(Root, "Root");
}

void ResponseScriptPropertiesPacket::Process(MonitorServerConnection& connection)
{
}
