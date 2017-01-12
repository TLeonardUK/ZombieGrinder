// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/MonitorServer.h"
#include "Game/Monitor/Packets/RequestActiveScriptsPacket.h"
#include "Game/Monitor/Packets/ResponseActiveScriptsPacket.h"

#include "Game/Scripts/GameVirtualMachine.h"
#include "Game/Scripts/ScriptEventListener.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Generic/Helper/StringHelper.h"

RequestActiveScriptsPacket::RequestActiveScriptsPacket()
	: BaseMonitorPacket("RequestActiveScripts")
{
}

void RequestActiveScriptsPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	BaseMonitorPacket::Serialize(value, bEncoding);
}

void RequestActiveScriptsPacket::Process(MonitorServerConnection& connection)
{
	ResponseActiveScriptsPacket response;
	response.RequestID = this->ID;

	for (std::vector<ScriptEventListener*>::iterator iter = ScriptEventListener::g_script_event_listeners.begin(); iter != ScriptEventListener::g_script_event_listeners.end(); iter++)
	{
		ScriptEventListener* listener = *iter;

		CVMObject* object = listener->Get_Object();
		CVMLinkedSymbol* symbol = object->Get_Symbol();

		ActiveScriptState state;
		state.ClassName = symbol->symbol->name;
		state.Name = StringHelper::Format("%i", listener->m_id);

		response.States.push_back(state);
	}

	connection.Send(&response);
}
