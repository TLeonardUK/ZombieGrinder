// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/Packets/BaseMonitorPacket.h"

#include "Game/Monitor/Packets/GameStateMonitorPacket.h"
#include "Game/Monitor/Packets/RequestActiveScriptsPacket.h"
#include "Game/Monitor/Packets/ResponseActiveScriptsPacket.h"
#include "Game/Monitor/Packets/RequestScriptPropertiesPacket.h"
#include "Game/Monitor/Packets/ResponseScriptPropertiesPacket.h"
#include "Game/Monitor/Packets/RequestSetScriptPropertyPacket.h"

BaseMonitorPacket::BaseMonitorPacket(std::string InClassName)
	: ClassName(InClassName)
{
	static int IDCounter = 0;

	ID			= ++IDCounter;
	RequestID	= 0;
}

void BaseMonitorPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	SERIALIZE_STRING(ClassName, "ClassName");
	SERIALIZE_INT(RequestID, "RequestID");
	SERIALIZE_INT(ID, "ID");
}

BaseMonitorPacket* BaseMonitorPacket::Create(const char* ClassName)
{
	if (strcmp(ClassName, "GameState") == 0)
		return new GameStateMonitorPacket();
	else if (strcmp(ClassName, "RequestActiveScripts") == 0)
		return new RequestActiveScriptsPacket();
	else if (strcmp(ClassName, "ResponseActiveScripts") == 0)
		return new ResponseActiveScriptsPacket();
	else if (strcmp(ClassName, "RequestScriptProperties") == 0)
		return new RequestScriptPropertiesPacket();
	else if (strcmp(ClassName, "ResponseScriptProperties") == 0)
		return new ResponseScriptPropertiesPacket();
	else if (strcmp(ClassName, "RequestSetScriptProperty") == 0)
		return new RequestSetScriptPropertyPacket();
	else
		return NULL;
}
