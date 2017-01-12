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

RequestGetLocalisationPacket::RequestGetLocalisationPacket()
	: BaseMonitorPacket("RequestActiveScripts")
{
}

void RequestGetLocalisationPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	BaseMonitorPacket::Serialize(value, bEncoding);
}

void RequestGetLocalisationPacket::Process(MonitorServerConnection& connection)
{
	ResponseGetLocalisationPacket response;
	/*
	std::vector<LanguageHandle*> languages = LanguageFactory::Get_Languages();
	for (std::vector<LanguageHandle*>::iterator iter = languages.begin(); iter != languages.end(); iter++)
	{
		Language* handle = iter->Get();
		for (ThreadSafeHashTable<const char*, unsigned int>::Iterator iter = handle->Strings.Begin(); iter != handle->Strings.End(); iter++)
		{
			LocalisationStringState state;
			state.Name = iter->Get_Value();

			response.States.push_back(state);
		}
	}
	*/
	connection.Send(&response);
}
