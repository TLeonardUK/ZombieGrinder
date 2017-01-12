/* *****************************************************************

		CRuntime_Vectors.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Vector3.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Engine/Platform/Platform.h"

#include <float.h>

CVMString Vec3_ToString(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;
	float z = obj->Get_Slot(2).float_value;

	return StringHelper::Format("Vec3(%f,%f,%f)", x, y, z).c_str();
}

CVMObjectHandle Vec3_Create(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(3);
	obj->Get_Slot(0).float_value = 0.0f;
	obj->Get_Slot(1).float_value = 0.0f;
	obj->Get_Slot(2).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec3_Create_X(CVirtualMachine* vm, CVMValue self, float x)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(3);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = 0.0f;
	obj->Get_Slot(2).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec3_Create_XY(CVirtualMachine* vm, CVMValue self, float x, float y)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(3);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = y;
	obj->Get_Slot(2).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec3_Create_XYZ(CVirtualMachine* vm, CVMValue self, float x, float y, float z)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(3);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = y;
	obj->Get_Slot(2).float_value = z;

	return obj;
}

CVMObjectHandle Vec3_Create_V2Z(CVirtualMachine* vm, CVMValue self, CVMObjectHandle xy, float z)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* xy_obj = xy.Get();
	vm->Assert(xy_obj != NULL);

	obj->Resize(3);
	obj->Get_Slot(0).float_value = xy_obj->Get_Slot(0).float_value;
	obj->Get_Slot(1).float_value = xy_obj->Get_Slot(1).float_value;
	obj->Get_Slot(2).float_value = z;

	return obj;
}

bool Vec3_Operator_EQ(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		fabs(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) < FLT_EPSILON &&		
		fabs(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) < FLT_EPSILON &&
		fabs(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) < FLT_EPSILON
	);
}

bool Vec3_Operator_NE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	return !Vec3_Operator_EQ(vm, self, rvalue);
}

bool Vec3_Operator_G(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) > 0.0f &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) > 0.0f &&
		(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) > 0.0f
	);
}

bool Vec3_Operator_GE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) >= 0.0f &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) >= 0.0f &&
		(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) >= 0.0f
	);
}

bool Vec3_Operator_L(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) < 0.0f &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) < 0.0f &&
		(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) < 0.0f
	);
}

bool Vec3_Operator_LE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) <= 0.0f &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) <= 0.0f &&
		(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) <= 0.0f
	);
}

CVMObjectHandle Vec3_Operator_SubV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value;	
	
	return result;
}

CVMObjectHandle Vec3_Operator_SubF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value - value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value - value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value - value;	

	return result;
}

CVMObjectHandle Vec3_Operator_Neg(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = -obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = -obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = -obj->Get_Slot(2).float_value;	

	return result;
}

CVMObjectHandle Vec3_Operator_AddV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value + r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value + r_obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value + r_obj->Get_Slot(2).float_value;	
	
	return result;
}

CVMObjectHandle Vec3_Operator_AddF(CVirtualMachine* vm, CVMValue self, float value)
{	
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value + value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value + value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value + value;	

	return result;
}

CVMObjectHandle Vec3_Operator_MulV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value * r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value * r_obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value * r_obj->Get_Slot(2).float_value;	

	return result;
}

CVMObjectHandle Vec3_Operator_MulF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value * value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value * value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value * value;	

	return result;
}

CVMObjectHandle Vec3_Operator_DivV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / r_obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value / r_obj->Get_Slot(2).float_value;	

	return result;
}

CVMObjectHandle Vec3_Operator_DivF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value / value;	
	
	return result;
}

void Vec3_Operator_SetIndex(CVirtualMachine* vm, CVMValue self, int index, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	vm->Assert(index >= 0 && index < 3);
	obj->Get_Slot(index).float_value = value;
}

float Vec3_Operator_GetIndex(CVirtualMachine* vm, CVMValue self, int index)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	vm->Assert(index >= 0 && index < 3);
	return obj->Get_Slot(index).float_value;
}

float Vec3_Get_X(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(0).float_value;
}

void Vec3_Set_X(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(0).float_value = value;
}

float Vec3_Get_Y(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(1).float_value;
}

void Vec3_Set_Y(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(1).float_value = value;
}

float Vec3_Get_Z(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(2).float_value;
}

void Vec3_Set_Z(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(2).float_value = value;
}

CVMObjectHandle Vec3_Get_XY(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value;

	return result;
}

void Vec3_Set_XY(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = other.Get();
	vm->Assert(r_obj != NULL);

	obj->Get_Slot(0).float_value = r_obj->Get_Slot(0).float_value;	
	obj->Get_Slot(1).float_value = r_obj->Get_Slot(1).float_value;	
}	

float Vec3_Get_Length(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;
	float z = obj->Get_Slot(2).float_value;
	
	return sqrt(x * x + y * y + z * z);
}

CVMObjectHandle Vec3_Unit(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;
	float z = obj->Get_Slot(2).float_value;
	float length = sqrt(x * x + y * y + z * z);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);	

	if (length == 0)
	{
		obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value;	
		obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value;	
		obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value;	
	}
	else
	{
		obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / length;	
		obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / length;	
		obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value / length;
	}

	return result;
}

float Vec3_Dot(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* other_obj = other.Get();
	vm->Assert(other_obj != NULL);

	float x1 = obj->Get_Slot(0).float_value;
	float y1 = obj->Get_Slot(1).float_value;
	float z1 = obj->Get_Slot(2).float_value;
	float x2 = other_obj->Get_Slot(0).float_value;
	float y2 = other_obj->Get_Slot(1).float_value;
	float z2 = other_obj->Get_Slot(2).float_value;

	return (x1 * x2) + (y1 * y2) + (z1 * z2);
}

CVMObjectHandle Vec3_Offset(CVirtualMachine* vm, CVMValue self, float dir, float len)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);	
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value + (cos(dir - HALFPI) * len);
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value + (sin(dir - HALFPI) * len);
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value;

	return result;
}

CVMObjectHandle Vec3_Rand(CVirtualMachine* vm, float min, float max)
{
	float angle = (float)Random::Static_Next_Double(0.0f, PI2);
	float power = (float)Random::Static_Next_Double(min, max);

	CVMObjectHandle handle = vm->Create_Vec3
	(
		sinf(angle) * power,
		cosf(angle) * power,
		-cosf(angle) * power
	);

	return handle;
}

void CRuntime_Vector3::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMString>										("Vec3", "ToString",			&Vec3_ToString);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec3", "Create",				&Vec3_Create);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec3", "Create_X",			&Vec3_Create_X);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float>					("Vec3", "Create_XY",			&Vec3_Create_XY);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float,float>				("Vec3", "Create_XYZ",			&Vec3_Create_XYZ);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle,float>			("Vec3", "Create_V2Z",			&Vec3_Create_V2Z);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec3", "Operator_EQ",		&Vec3_Operator_EQ);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec3", "Operator_NE",		&Vec3_Operator_NE);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec3", "Operator_G",			&Vec3_Operator_G);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec3", "Operator_GE",		&Vec3_Operator_GE);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec3", "Operator_L",			&Vec3_Operator_L);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec3", "Operator_LE",		&Vec3_Operator_LE);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec3", "Operator_SubV",		&Vec3_Operator_SubV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec3", "Operator_SubF",		&Vec3_Operator_SubF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec3", "Operator_Neg",		&Vec3_Operator_Neg);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec3", "Operator_AddV",		&Vec3_Operator_AddV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec3", "Operator_AddF",		&Vec3_Operator_AddF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec3", "Operator_MulV",		&Vec3_Operator_MulV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec3", "Operator_MulF",		&Vec3_Operator_MulF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec3", "Operator_DivV",		&Vec3_Operator_DivV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec3", "Operator_DivF",		&Vec3_Operator_DivF);	
	vm->Get_Bindings()->Bind_Method<void,int,float>									("Vec3", "Operator_SetIndex",	&Vec3_Operator_SetIndex);
	vm->Get_Bindings()->Bind_Method<float,int>										("Vec3", "Operator_GetIndex",	&Vec3_Operator_GetIndex);
	vm->Get_Bindings()->Bind_Method<float>											("Vec3", "Length",				&Vec3_Get_Length);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float>					("Vec3", "Offset",				&Vec3_Offset);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec3", "Unit",				&Vec3_Unit);
	vm->Get_Bindings()->Bind_Method<float,CVMObjectHandle>							("Vec3", "Dot",					&Vec3_Dot);
	vm->Get_Bindings()->Bind_Method<float>											("Vec3", "Get_X",				&Vec3_Get_X);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec3", "Set_X",				&Vec3_Set_X);
	vm->Get_Bindings()->Bind_Method<float>											("Vec3", "Get_Y",				&Vec3_Get_Y);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec3", "Set_Y",				&Vec3_Set_Y);
	vm->Get_Bindings()->Bind_Method<float>											("Vec3", "Get_Z",				&Vec3_Get_Z);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec3", "Set_Z",				&Vec3_Set_Z);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec3", "Get_XY",				&Vec3_Get_XY);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>							("Vec3", "Set_XY",				&Vec3_Set_XY);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,float,float>					("Vec3", "Rand",				&Vec3_Rand);
}