// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/MonitorServer.h"
#include "Game/Monitor/Packets/RequestSetScriptPropertyPacket.h"

#include "Game/Scripts/GameVirtualMachine.h"
#include "Game/Scripts/ScriptEventListener.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"

#include "Generic/Helper/StringHelper.h"

RequestSetScriptPropertyPacket::RequestSetScriptPropertyPacket()
	: BaseMonitorPacket("RequestSetScriptProperty")
{
}

void RequestSetScriptPropertyPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	BaseMonitorPacket::Serialize(value, bEncoding);

	SERIALIZE_STRING(InstanceName, "InstanceName");
	SERIALIZE_STRING(NewValue, "NewValue");
	SERIALIZE_STRING(PropertyPath, "PropertyPath");
	SERIALIZE_STRING(PropertyType, "PropertyType");
}

void RequestSetScriptPropertyPacket::Process(MonitorServerConnection& connection)
{
	DBG_LOG("Recieved request to set script '%s' property '%s' to '%s'", InstanceName.c_str(), PropertyPath.c_str(), NewValue.c_str());

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int instanceId = atoi(InstanceName.c_str());
	for (std::vector<ScriptEventListener*>::iterator iter = ScriptEventListener::g_script_event_listeners.begin(); iter != ScriptEventListener::g_script_event_listeners.end(); iter++)
	{
		ScriptEventListener* listener = *iter;

		CVMObject* object = listener->Get_Object();
		CVMLinkedSymbol* symbol = object->Get_Symbol();

		if (listener->m_id == instanceId)
		{
			CVMValue value;

			if (PropertyType == "Float")
			{
				value.float_value = atof(NewValue.c_str());
			}
			else if (PropertyType == "Bool")
			{
				value.int_value = (NewValue == "True" || NewValue == "1");
			}
			else if (PropertyType == "Int")
			{
				value.int_value = atoi(NewValue.c_str());
			}
			else if (PropertyType == "String")
			{
				value.string_value = NewValue.c_str();
			}

			vm->Set_Value(object, PropertyPath.c_str(), value);
		}
	}
}
