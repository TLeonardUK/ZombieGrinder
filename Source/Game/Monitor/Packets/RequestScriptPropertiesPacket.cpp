// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/MonitorServer.h"
#include "Game/Monitor/Packets/RequestScriptPropertiesPacket.h"
#include "Game/Monitor/Packets/ResponseScriptPropertiesPacket.h"

#include "Game/Scripts/GameVirtualMachine.h"
#include "Game/Scripts/ScriptEventListener.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"

#include "Generic/Helper/StringHelper.h"

RequestScriptPropertiesPacket::RequestScriptPropertiesPacket()
	: BaseMonitorPacket("RequestScriptProperties")
{
}

void RequestScriptPropertiesPacket::Serialize(MonitorPacketSerializer& value, bool bEncoding)
{
	BaseMonitorPacket::Serialize(value, bEncoding);

	SERIALIZE_STRING(InstanceName, "InstanceName");
}

void RequestScriptPropertiesPacket::WriteObject(ScriptProperty& parent, CVMValue value, int depth)
{
	CVMObject* obj = value.object_value.Get();
	CVMLinkedSymbol* symbol = obj->Get_Symbol();
	
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	//while (symbol->symbol->type != SymbolType::Root)
	{
		for (int i = 0; i < symbol->symbol->children_count; i++)
		{
			CVMLinkedSymbol* child = vm->Get_Symbol_Table_Entry(symbol->symbol->children_indexes[i]);
			
			// Ignore inspection disabled ones, they are usually things like help properties (XY swizzle on vector etc).
			if (child->symbol->Get_Meta_Data<bool>("DisableInspection", false) == true)
			{
				continue;
			}

			if (child->symbol->type == SymbolType::ClassField)
			{
				CVMValue childValue = value.object_value.Get()->Get_Slot(child->symbol->field_data->offset);
				if (child->symbol->field_data->is_static)
				{
					childValue = symbol->static_data[child->symbol->field_data->offset];
				}

				WriteValue(parent, child, childValue, depth);
			}
			else if (child->symbol->type == SymbolType::ClassProperty)
			{
				CVMLinkedSymbol* get_method = vm->Get_Symbol_Table_Entry(child->symbol->property_data->get_property_symbol);
				vm->Invoke(get_method, value, false, false);

				CVMValue subvalue;
				vm->Get_Return_Value(subvalue);

				WriteValue(parent, child, subvalue, depth);
			}
		}

		//symbol = vm->Get_Symbol_Table_Entry(symbol->symbol->class_data->super_class_index);
	}
}

void RequestScriptPropertiesPacket::WriteArray(ScriptProperty& parent, CVMValue value, CVMDataType* elementType, int depth)
{
	CVMObject* obj = value.object_value.Get();
	CVMLinkedSymbol* symbol = obj->Get_Symbol();
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	CVMLinkedSymbol* elementClass = vm->Get_Symbol_Table_Entry(elementType->class_index);

	for (int i = 0; i < obj->Slot_Count(); i++)
	{
		CVMValue elementValue = obj->Get_Slot(i);
		WriteValue(parent, elementClass, elementValue, depth);
	}
}

void RequestScriptPropertiesPacket::WriteValue(ScriptProperty& parent, CVMLinkedSymbol* symbol, CVMValue value, int depth)
{
	ScriptProperty prop;
	prop.Name = symbol->symbol->name;
	prop.bReadOnly = false;

	CVMDataType* dt = NULL;
	if (symbol->symbol->type == SymbolType::Class)
	{
		dt = symbol->symbol->class_data->data_type;
	}
	else if (symbol->symbol->type == SymbolType::ClassProperty)
	{
		dt = symbol->symbol->property_data->data_type;
		prop.bReadOnly = !(symbol->symbol->property_data->set_property_symbol > 0);
	}
	else
	{
		dt = symbol->symbol->field_data->data_type;
		prop.bReadOnly = !!(symbol->symbol->field_data->is_static);
	}

	CVMBaseDataType::Type base_dt = dt->type;
	if (base_dt == CVMBaseDataType::Object)
	{
		if (prop.Name == "float")
		{
			base_dt = CVMBaseDataType::Float;
		}
		else if (prop.Name == "bool")
		{
			base_dt = CVMBaseDataType::Bool;
		}
		else if (prop.Name == "int")
		{
			base_dt = CVMBaseDataType::Int;
		}
		else if (prop.Name == "string")
		{
			base_dt = CVMBaseDataType::String;
		}
	}

	switch (base_dt)
	{
	case CVMBaseDataType::Array:
		{
			prop.TypeName = "Array";
			prop.CurrentValue = "Array";
			if (value.object_value.Get() != NULL)
			{
				WriteArray(prop, value, dt->element_type, depth);
			}
			else
			{
				prop.CurrentValue = "Null";
			}
			break;
		}
	case CVMBaseDataType::Bool:
		{
			prop.TypeName = "Bool";
			prop.CurrentValue = StringHelper::Format("%i", value.int_value);
			break;
		}
	case CVMBaseDataType::Float:
		{
			prop.TypeName = "Float";
			prop.CurrentValue = StringHelper::Format("%f", value.float_value);
			break;
		}
	case CVMBaseDataType::Int:
		{
			prop.TypeName = "Int";
			prop.CurrentValue = StringHelper::Format("%i", value.int_value);
			break;
		}
	case CVMBaseDataType::Object:
		{
			prop.TypeName = "Object";
			prop.CurrentValue = "Object";
			if (value.object_value.Get() != NULL && depth < 3)
			{
				WriteObject(prop, value, depth + 1);
			}
			else
			{
				prop.CurrentValue = "Null";
			}
			break;
		}
	case CVMBaseDataType::String:
		{
			prop.TypeName = "String";
			prop.CurrentValue = value.string_value.C_Str();
			break;
		}
	}

	if (prop.TypeName != "")
	{
		parent.Children.push_back(prop);
	}
}

void RequestScriptPropertiesPacket::Process(MonitorServerConnection& connection)
{
	ResponseScriptPropertiesPacket response;
	response.RequestID = this->ID;
	response.bScriptExists = false;

	DBG_LOG("Recieved request for properties of script #%s", InstanceName.c_str());

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
			response.bScriptExists = true;

			response.Root.Name			= "Root";
			response.Root.CurrentValue	= "";
			response.Root.TypeName		= "Object";

			WriteObject(response.Root, object, 0);

			break;
		}
	}

	connection.Send(&response);
}
