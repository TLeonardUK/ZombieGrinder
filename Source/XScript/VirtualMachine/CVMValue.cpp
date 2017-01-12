/* *****************************************************************

		CVMValue.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"

bool CVMValue::Equal_To(CVMValue& other, CVMDataType* type)
{
	switch (type->type)
	{
	case CVMBaseDataType::Array:	
	case CVMBaseDataType::Object:
		return (other.object_value.Get() == object_value.Get());
	case CVMBaseDataType::Bool:
	case CVMBaseDataType::Int:
		return (other.int_value == int_value);
	case CVMBaseDataType::Float:
		return (other.float_value == float_value);
	case CVMBaseDataType::String:
		return (other.string_value == string_value);
	case CVMBaseDataType::Void:
		break;
	}
	return false;
}