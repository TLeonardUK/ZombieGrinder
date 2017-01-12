/* *****************************************************************

		CRuntime_Vectors.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Vector4.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/Types/Rect2D.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Engine/Platform/Platform.h"

#include <float.h>

CVMString Vec4_ToString(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;
	float z = obj->Get_Slot(2).float_value;
	float w = obj->Get_Slot(3).float_value;

	return StringHelper::Format("Vec4(%f,%f,%f,%f)", x, y, z, w).c_str();
}

CVMObjectHandle Vec4_Create(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(4);
	obj->Get_Slot(0).float_value = 0.0f;
	obj->Get_Slot(1).float_value = 0.0f;
	obj->Get_Slot(2).float_value = 0.0f;
	obj->Get_Slot(3).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec4_Create_X(CVirtualMachine* vm, CVMValue self, float x)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(4);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = 0.0f;
	obj->Get_Slot(2).float_value = 0.0f;
	obj->Get_Slot(3).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec4_Create_XY(CVirtualMachine* vm, CVMValue self, float x, float y)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(4);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = y;
	obj->Get_Slot(2).float_value = 0.0f;
	obj->Get_Slot(3).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec4_Create_XYZ(CVirtualMachine* vm, CVMValue self, float x, float y, float z)
{	
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(4);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = y;
	obj->Get_Slot(2).float_value = z;
	obj->Get_Slot(3).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec4_Create_XYZW(CVirtualMachine* vm, CVMValue self, float x, float y, float z, float w)
{	
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(4);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = y;
	obj->Get_Slot(2).float_value = z;
	obj->Get_Slot(3).float_value = w;

	return obj;
}

CVMObjectHandle Vec4_Create_V3W(CVirtualMachine* vm, CVMValue self, CVMObjectHandle xyz, float w)
{	
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* xyz_obj = xyz.Get();
	vm->Assert(xyz_obj != NULL);

	obj->Resize(4);
	obj->Get_Slot(0).float_value = xyz_obj->Get_Slot(0).float_value;
	obj->Get_Slot(1).float_value = xyz_obj->Get_Slot(1).float_value;
	obj->Get_Slot(2).float_value = xyz_obj->Get_Slot(2).float_value;
	obj->Get_Slot(3).float_value = w;

	return obj;
}

CVMObjectHandle Vec4_Create_V2ZW(CVirtualMachine* vm, CVMValue self, CVMObjectHandle xy, float z, float w)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* xy_obj = xy.Get();
	vm->Assert(xy_obj != NULL);
	

	obj->Resize(4);
	obj->Get_Slot(0).float_value = xy_obj->Get_Slot(0).float_value;
	obj->Get_Slot(1).float_value = xy_obj->Get_Slot(1).float_value;
	obj->Get_Slot(2).float_value = z;
	obj->Get_Slot(3).float_value = w;

	return obj;
}

CVMObjectHandle Vec4_Create_V2V2(CVirtualMachine* vm, CVMValue self, CVMObjectHandle xy, CVMObjectHandle zw)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* xy_obj = xy.Get();
	vm->Assert(xy_obj != NULL);
	
	CVMObject* zw_obj = zw.Get();
	vm->Assert(zw_obj != NULL);

	obj->Resize(4);
	obj->Get_Slot(0).float_value = xy_obj->Get_Slot(0).float_value;
	obj->Get_Slot(1).float_value = xy_obj->Get_Slot(1).float_value;
	obj->Get_Slot(2).float_value = zw_obj->Get_Slot(0).float_value;
	obj->Get_Slot(3).float_value = zw_obj->Get_Slot(1).float_value;

	return obj;
}

bool Vec4_Operator_EQ(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		fabs(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) < FLT_EPSILON &&		
		fabs(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) < FLT_EPSILON &&
		fabs(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) < FLT_EPSILON &&
		fabs(obj->Get_Slot(3).float_value - r_obj->Get_Slot(3).float_value) < FLT_EPSILON
	);
}

bool Vec4_Operator_NE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{	
	return !Vec4_Operator_EQ(vm, self, rvalue);
}

bool Vec4_Operator_G(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{	
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) > FLT_EPSILON &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) > FLT_EPSILON &&
		(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) > FLT_EPSILON &&
		(obj->Get_Slot(3).float_value - r_obj->Get_Slot(3).float_value) > FLT_EPSILON
	);
}

bool Vec4_Operator_GE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) >= 0.0f &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) >= 0.0f &&
		(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) >= 0.0f &&
		(obj->Get_Slot(3).float_value - r_obj->Get_Slot(3).float_value) >= 0.0f
	);
}

bool Vec4_Operator_L(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) < -FLT_EPSILON &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) < -FLT_EPSILON &&
		(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) < -FLT_EPSILON &&
		(obj->Get_Slot(3).float_value - r_obj->Get_Slot(3).float_value) < -FLT_EPSILON
	);
}

bool Vec4_Operator_LE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) <= 0.0f &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) <= 0.0f &&
		(obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value) <= 0.0f &&
		(obj->Get_Slot(3).float_value - r_obj->Get_Slot(3).float_value) <= 0.0f
	);
}

CVMObjectHandle Vec4_Operator_SubV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value - r_obj->Get_Slot(2).float_value;	
	obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value - r_obj->Get_Slot(3).float_value;

	return result;
}

CVMObjectHandle Vec4_Operator_SubF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value - value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value - value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value - value;	
	obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value - value;

	return result;
}

CVMObjectHandle Vec4_Operator_Neg(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = -obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = -obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = -obj->Get_Slot(2).float_value;	
	obj_result->Get_Slot(3).float_value = -obj->Get_Slot(3).float_value;

	return result;
}

CVMObjectHandle Vec4_Operator_AddV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value + r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value + r_obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value + r_obj->Get_Slot(2).float_value;	
	obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value + r_obj->Get_Slot(3).float_value;

	return result;
}

CVMObjectHandle Vec4_Operator_AddF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value + value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value + value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value + value;	
	obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value + value;

	return result;
}

CVMObjectHandle Vec4_Operator_MulV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value * r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value * r_obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value * r_obj->Get_Slot(2).float_value;	
	obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value * r_obj->Get_Slot(3).float_value;

	return result;
}

CVMObjectHandle Vec4_Operator_MulF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value * value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value * value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value * value;	
	obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value * value;

	return result;
}

CVMObjectHandle Vec4_Operator_DivV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / r_obj->Get_Slot(1).float_value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value / r_obj->Get_Slot(2).float_value;	
	obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value / r_obj->Get_Slot(3).float_value;

	return result;
}

CVMObjectHandle Vec4_Operator_DivF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / value;	
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value / value;	
	obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value / value;

	return result;
}

void Vec4_Operator_SetIndex(CVirtualMachine* vm, CVMValue self, int index, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	vm->Assert(index >= 0 && index < 4);
	obj->Get_Slot(index).float_value = value;
}

float Vec4_Operator_GetIndex(CVirtualMachine* vm, CVMValue self, int index)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	vm->Assert(index >= 0 && index < 4);
	return obj->Get_Slot(index).float_value;
}

float Vec4_Get_X(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(0).float_value;
}

void Vec4_Set_X(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(0).float_value = value;
}

float Vec4_Get_Y(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(1).float_value;
}

void Vec4_Set_Y(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(1).float_value = value;
}

float Vec4_Get_Z(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(2).float_value;
}

void Vec4_Set_Z(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(2).float_value = value;
}

float Vec4_Get_W(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(3).float_value;
}

void Vec4_Set_W(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(3).float_value = value;
}

CVMObjectHandle Vec4_Get_XY(CVirtualMachine* vm, CVMValue self)
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

void Vec4_Set_XY(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = other.Get();
	vm->Assert(r_obj != NULL);

	obj->Get_Slot(0).float_value = r_obj->Get_Slot(0).float_value;	
	obj->Get_Slot(1).float_value = r_obj->Get_Slot(1).float_value;	
}

CVMObjectHandle Vec4_Get_ZW(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(2).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(3).float_value;

	return result;
}

void Vec4_Set_ZW(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = other.Get();
	vm->Assert(r_obj != NULL);

	obj->Get_Slot(2).float_value = r_obj->Get_Slot(0).float_value;	
	obj->Get_Slot(3).float_value = r_obj->Get_Slot(1).float_value;
}

CVMObjectHandle Vec4_Get_XYZ(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMLinkedSymbol* vec3_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec3_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value;
	obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value;

	return result;
}

void Vec4_Set_XYZ(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = other.Get();
	vm->Assert(r_obj != NULL);

	obj->Get_Slot(0).float_value = r_obj->Get_Slot(0).float_value;	
	obj->Get_Slot(1).float_value = r_obj->Get_Slot(1).float_value;	
	obj->Get_Slot(2).float_value = r_obj->Get_Slot(2).float_value;	
}

float Vec4_Get_Length(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;
	float z = obj->Get_Slot(2).float_value;
	float w = obj->Get_Slot(3).float_value;
	
	return sqrt(x * x + y * y + z * z + w * w);
}

CVMObjectHandle Vec4_Center(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value + (obj->Get_Slot(2).float_value * 0.5f); 	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value + (obj->Get_Slot(3).float_value * 0.5f); 

	return result;
}

CVMObjectHandle Vec4_Constrain(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* other_obj = other.Get();
	vm->Assert(other_obj != NULL);
	
	CVMLinkedSymbol* vec4_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Rect2D main = Rect2D(obj->Get_Slot(0).float_value,
						obj->Get_Slot(1).float_value,
						obj->Get_Slot(2).float_value,
						obj->Get_Slot(3).float_value);

	Rect2D bounds = Rect2D(other_obj->Get_Slot(0).float_value,
						other_obj->Get_Slot(1).float_value,
						other_obj->Get_Slot(2).float_value,
						other_obj->Get_Slot(3).float_value);

	main = main.ConstrainInside(bounds);
	
	CVMObjectHandle result = vm->New_Object(vec4_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = main.X;
	obj_result->Get_Slot(1).float_value = main.Y;
	obj_result->Get_Slot(2).float_value = main.Width;
	obj_result->Get_Slot(3).float_value = main.Height;

	return result;
}

int Vec4_Intersects(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* other_obj = other.Get();
	vm->Assert(other_obj != NULL);

	CVMLinkedSymbol* vec4_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Rect2D main = Rect2D(obj->Get_Slot(0).float_value,
		obj->Get_Slot(1).float_value,
		obj->Get_Slot(2).float_value,
		obj->Get_Slot(3).float_value);

	Rect2D bounds = Rect2D(other_obj->Get_Slot(0).float_value,
		other_obj->Get_Slot(1).float_value,
		other_obj->Get_Slot(2).float_value,
		other_obj->Get_Slot(3).float_value);

	return main.Intersects(bounds) ? 1 : 0;
}

CVMObjectHandle Vec4_Unit(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;
	float z = obj->Get_Slot(2).float_value;
	float w = obj->Get_Slot(3).float_value;
	float length = sqrt(x * x + y * y + z * z + w * w);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);	

	if (length == 0)
	{
		obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value;	
		obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value;	
		obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value;	
		obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value;	
	}
	else
	{
		obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / length;	
		obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / length;	
		obj_result->Get_Slot(2).float_value = obj->Get_Slot(2).float_value / length;
		obj_result->Get_Slot(3).float_value = obj->Get_Slot(3).float_value / length;
	}

	return result;
}

float Vec4_Dot(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* other_obj = other.Get();
	vm->Assert(other_obj != NULL);

	float x1 = obj->Get_Slot(0).float_value;
	float y1 = obj->Get_Slot(1).float_value;
	float z1 = obj->Get_Slot(2).float_value;
	float w1 = obj->Get_Slot(3).float_value;
	float x2 = other_obj->Get_Slot(0).float_value;
	float y2 = other_obj->Get_Slot(1).float_value;
	float z2 = other_obj->Get_Slot(2).float_value;
	float w2 = other_obj->Get_Slot(3).float_value;

	return (x1 * x2) + (y1 * y2) + (z1 * z2) + (w1 * w2);
}

CVMObjectHandle Vec4_Rand(CVirtualMachine* vm, float min, float max)
{
	float angle = (float)Random::Static_Next_Double(0.0f, PI2);
	float power = (float)Random::Static_Next_Double(min, max);

	CVMObjectHandle handle = vm->Create_Vec4
		(
		sinf(angle) * power,
		cosf(angle) * power,
		-cosf(angle) * power,
		-sinf(angle) * power
		);

	return handle;
}

CVMObjectHandle Vec4_Shift(CVirtualMachine* vm, CVMValue self, float x, float y)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float f_x = (obj->Get_Slot(2).float_value * x);
	float f_y = (obj->Get_Slot(3).float_value * y);

	return vm->Create_Vec4
		(
		obj->Get_Slot(0).float_value + f_x,
		obj->Get_Slot(1).float_value + f_y,
		obj->Get_Slot(2).float_value,
		obj->Get_Slot(3).float_value
		);
}

CVMObjectHandle Vec4_Inflate(CVirtualMachine* vm, CVMValue self, float x, float y)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float f_x = (obj->Get_Slot(2).float_value * x) - obj->Get_Slot(2).float_value;
	float f_y = (obj->Get_Slot(3).float_value * y) - obj->Get_Slot(3).float_value;

	return vm->Create_Vec4
		(
		obj->Get_Slot(0).float_value - (f_x * 0.5f),
		obj->Get_Slot(1).float_value - (f_y * 0.5f),
		obj->Get_Slot(2).float_value + f_x,
		obj->Get_Slot(3).float_value + f_y
		);
}

CVMObjectHandle Vec4_RectUnion(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* obj2 = other.Get();
	vm->Assert(obj2 != NULL);

	Rect2D r1;
	r1.X = obj->Get_Slot(0).float_value;
	r1.Y = obj->Get_Slot(1).float_value;
	r1.Width = obj->Get_Slot(2).float_value;
	r1.Height = obj->Get_Slot(3).float_value;

	Rect2D r2;
	r2.X = obj2->Get_Slot(0).float_value;
	r2.Y = obj2->Get_Slot(1).float_value;
	r2.Width = obj2->Get_Slot(2).float_value;
	r2.Height = obj2->Get_Slot(3).float_value;
	
	Rect2D un = r1.Union(r2);

	return vm->Create_Vec4
		(
			un.X,
			un.Y,
			un.Width,
			un.Height
		);
}

// ---------------------------------------------------------------------------------------------------------------------------

void CRuntime_Vector4::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMString>										("Vec4", "ToString",			&Vec4_ToString);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec4", "Create",				&Vec4_Create);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec4", "Create_X",			&Vec4_Create_X);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float>					("Vec4", "Create_XY",			&Vec4_Create_XY);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float,float>				("Vec4", "Create_XYZ",			&Vec4_Create_XYZ);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float,float,float>		("Vec4", "Create_XYZW",			&Vec4_Create_XYZW);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle,float>			("Vec4", "Create_V3W",			&Vec4_Create_V3W);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle,float,float>	("Vec4", "Create_V2ZW",			&Vec4_Create_V2ZW);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle>("Vec4", "Create_V2V2",			&Vec4_Create_V2V2);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec4", "Operator_EQ",			&Vec4_Operator_EQ);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec4", "Operator_NE",			&Vec4_Operator_NE);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec4", "Operator_G",			&Vec4_Operator_G);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec4", "Operator_GE",			&Vec4_Operator_GE);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec4", "Operator_L",			&Vec4_Operator_L);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec4", "Operator_LE",			&Vec4_Operator_LE);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec4", "Operator_SubV",		&Vec4_Operator_SubV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec4", "Operator_SubF",		&Vec4_Operator_SubF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec4", "Operator_Neg",		&Vec4_Operator_Neg);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec4", "Operator_AddV",		&Vec4_Operator_AddV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec4", "Operator_AddF",		&Vec4_Operator_AddF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec4", "Operator_MulV",		&Vec4_Operator_MulV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec4", "Operator_MulF",		&Vec4_Operator_MulF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec4", "Operator_DivV",		&Vec4_Operator_DivV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec4", "Operator_DivF",		&Vec4_Operator_DivF);	
	vm->Get_Bindings()->Bind_Method<void,int,float>									("Vec4", "Operator_SetIndex",	&Vec4_Operator_SetIndex);
	vm->Get_Bindings()->Bind_Method<float,int>										("Vec4", "Operator_GetIndex",	&Vec4_Operator_GetIndex);
	vm->Get_Bindings()->Bind_Method<float>											("Vec4", "Length",				&Vec4_Get_Length);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec4", "Center",				&Vec4_Center);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec4", "Constrain",			&Vec4_Constrain);
	vm->Get_Bindings()->Bind_Method<int,CVMObjectHandle>							("Vec4", "Intersects",			&Vec4_Intersects);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec4", "Unit",				&Vec4_Unit);
	vm->Get_Bindings()->Bind_Method<float,CVMObjectHandle>							("Vec4", "Dot",					&Vec4_Dot);
	vm->Get_Bindings()->Bind_Method<float>											("Vec4", "Get_X",				&Vec4_Get_X);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec4", "Set_X",				&Vec4_Set_X);
	vm->Get_Bindings()->Bind_Method<float>											("Vec4", "Get_Y",				&Vec4_Get_Y);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec4", "Set_Y",				&Vec4_Set_Y);
	vm->Get_Bindings()->Bind_Method<float>											("Vec4", "Get_Z",				&Vec4_Get_Z);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec4", "Set_Z",				&Vec4_Set_Z);
	vm->Get_Bindings()->Bind_Method<float>											("Vec4", "Get_W",				&Vec4_Get_W);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec4", "Set_W",				&Vec4_Set_W);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec4", "Get_XY",				&Vec4_Get_XY);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>							("Vec4", "Set_XY",				&Vec4_Set_XY);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec4", "Get_ZW",				&Vec4_Get_ZW);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>							("Vec4", "Set_ZW",				&Vec4_Set_ZW);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec4", "Get_XYZ",				&Vec4_Get_XYZ);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>							("Vec4", "Set_XYZ",				&Vec4_Set_XYZ);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,float,float>					("Vec4", "Rand",				&Vec4_Rand);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float>					("Vec4", "Shift",				&Vec4_Shift);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float>					("Vec4", "Inflate",				&Vec4_Inflate);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle, CVMObjectHandle>				("Vec4", "RectUnion",			&Vec4_RectUnion);
}