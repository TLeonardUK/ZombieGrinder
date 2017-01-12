/* *****************************************************************

		CRuntime_Vectors.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Vector2.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Engine/Platform/Platform.h"

#include <float.h>

CVMString Vec2_ToString(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;

	return StringHelper::Format("Vec2(%f,%f)", x, y).c_str();
}

CVMObjectHandle Vec2_Create(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(2);
	obj->Get_Slot(0).float_value = 0.0f;
	obj->Get_Slot(1).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec2_Create_X(CVirtualMachine* vm, CVMValue self, float x)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(2);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = 0.0f;

	return obj;
}

CVMObjectHandle Vec2_Create_XY(CVirtualMachine* vm, CVMValue self, float x, float y)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Resize(2);
	obj->Get_Slot(0).float_value = x;
	obj->Get_Slot(1).float_value = y;

	return obj;
}

bool Vec2_Operator_EQ(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		fabs(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) < FLT_EPSILON &&		
		fabs(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) < FLT_EPSILON 
	);
}

bool Vec2_Operator_NE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	return !Vec2_Operator_EQ(vm, self, rvalue);
}

bool Vec2_Operator_G(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) > FLT_EPSILON &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) > FLT_EPSILON 
	);
}

bool Vec2_Operator_GE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) >= 0.0f &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) >= 0.0f 
	);
}

bool Vec2_Operator_L(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) < -FLT_EPSILON &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) < -FLT_EPSILON 
	);
}

bool Vec2_Operator_LE(CVirtualMachine* vm, CVMValue self, CVMObjectHandle rvalue)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = rvalue.Get();
	vm->Assert(r_obj != NULL);

	return 
	(
		(obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value) <= 0.0f &&
		(obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value) <= 0.0f 
	);
}

CVMObjectHandle Vec2_Operator_SubV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value - r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value - r_obj->Get_Slot(1).float_value;	

	return result;
}

CVMObjectHandle Vec2_Operator_SubF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value - value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value - value;	

	return result;
}

CVMObjectHandle Vec2_Operator_Neg(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = -obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = -obj->Get_Slot(1).float_value;	

	return result;
}

CVMObjectHandle Vec2_Operator_AddV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value + r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value + r_obj->Get_Slot(1).float_value;	

	return result;
}

CVMObjectHandle Vec2_Operator_AddF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value + value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value + value;	

	return result;
}

CVMObjectHandle Vec2_Operator_MulV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value * r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value * r_obj->Get_Slot(1).float_value;	

	return result;
}

CVMObjectHandle Vec2_Operator_MulF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value * value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value * value;	

	return result;
}

CVMObjectHandle Vec2_Operator_DivV(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObject* r_obj = value.Get();
	vm->Assert(r_obj != NULL);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / r_obj->Get_Slot(0).float_value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / r_obj->Get_Slot(1).float_value;	

	return result;
}

CVMObjectHandle Vec2_Operator_DivF(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / value;	
	obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / value;	

	return result;
}

void Vec2_Operator_SetIndex(CVirtualMachine* vm, CVMValue self, int index, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	vm->Assert(index >= 0 && index < 2);
	obj->Get_Slot(index).float_value = value;
}

float Vec2_Operator_GetIndex(CVirtualMachine* vm, CVMValue self, int index)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);
	
	vm->Assert(index >= 0 && index < 2);
	return obj->Get_Slot(index).float_value;
}

float Vec2_Get_X(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(0).float_value;
}

void Vec2_Set_X(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(0).float_value = value;
}

float Vec2_Get_Y(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	return obj->Get_Slot(1).float_value;
}

void Vec2_Set_Y(CVirtualMachine* vm, CVMValue self, float value)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	obj->Get_Slot(1).float_value = value;
}
	
float Vec2_Get_Length(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;
	
	return sqrt(x * x + y * y);
}

float Vec2_Get_Direction(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;

	return atan2(y, x);
}

CVMObjectHandle Vec2_Unit(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	float x = obj->Get_Slot(0).float_value;
	float y = obj->Get_Slot(1).float_value;
	float length = sqrt(x * x + y * y);

	CVMObjectHandle result = vm->New_Object(obj->Get_Symbol(), false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);

	if (length == 0)
	{
		obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value;	
		obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value;	
	}
	else
	{
		obj_result->Get_Slot(0).float_value = obj->Get_Slot(0).float_value / length;	
		obj_result->Get_Slot(1).float_value = obj->Get_Slot(1).float_value / length;	
	}

	return result;
}

float Vec2_Dot(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other)
{
	CVMObject* obj = self.object_value.Get();
	vm->Assert(obj != NULL);

	CVMObject* other_obj = other.Get();
	vm->Assert(other_obj != NULL);

	float x1 = obj->Get_Slot(0).float_value;
	float y1 = obj->Get_Slot(1).float_value;
	float x2 = other_obj->Get_Slot(0).float_value;
	float y2 = other_obj->Get_Slot(1).float_value;

	return (x1 * x2) + (y1 * y2);
}

CVMObjectHandle Vec2_Rand(CVirtualMachine* vm, float min, float max)
{
	float angle = (float)Random::Static_Next_Double(0.0f, PI2);
	float power = (float)Random::Static_Next_Double(min, max);

	CVMObjectHandle handle = vm->Create_Vec2
	(
		sinf(angle) * power,
		cosf(angle) * power
	);

	return handle;
}

void CRuntime_Vector2::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMString>										("Vec2", "ToString",			&Vec2_ToString);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec2", "Create",				&Vec2_Create);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec2", "Create_X",			&Vec2_Create_X);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float,float>					("Vec2", "Create_XY",			&Vec2_Create_XY);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec2", "Operator_EQ",		&Vec2_Operator_EQ);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec2", "Operator_NE",		&Vec2_Operator_NE);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec2", "Operator_G",			&Vec2_Operator_G);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec2", "Operator_GE",		&Vec2_Operator_GE);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec2", "Operator_L",			&Vec2_Operator_L);
	vm->Get_Bindings()->Bind_Method<bool,CVMObjectHandle>							("Vec2", "Operator_LE",		&Vec2_Operator_LE);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec2", "Operator_SubV",		&Vec2_Operator_SubV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec2", "Operator_SubF",		&Vec2_Operator_SubF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec2", "Operator_Neg",		&Vec2_Operator_Neg);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec2", "Operator_AddV",		&Vec2_Operator_AddV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec2", "Operator_AddF",		&Vec2_Operator_AddF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec2", "Operator_MulV",		&Vec2_Operator_MulV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec2", "Operator_MulF",		&Vec2_Operator_MulF);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>				("Vec2", "Operator_DivV",		&Vec2_Operator_DivV);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,float>							("Vec2", "Operator_DivF",		&Vec2_Operator_DivF);	
	vm->Get_Bindings()->Bind_Method<void,int,float>									("Vec2", "Operator_SetIndex",	&Vec2_Operator_SetIndex);
	vm->Get_Bindings()->Bind_Method<float,int>										("Vec2", "Operator_GetIndex",	&Vec2_Operator_GetIndex);
	vm->Get_Bindings()->Bind_Method<float>											("Vec2", "Length",				&Vec2_Get_Length);
	vm->Get_Bindings()->Bind_Method<float>											("Vec2", "Direction",			&Vec2_Get_Direction);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>								("Vec2", "Unit",				&Vec2_Unit);
	vm->Get_Bindings()->Bind_Method<float,CVMObjectHandle>							("Vec2", "Dot",					&Vec2_Dot);
	vm->Get_Bindings()->Bind_Method<float>											("Vec2", "Get_X",				&Vec2_Get_X);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec2", "Set_X",				&Vec2_Set_X);
	vm->Get_Bindings()->Bind_Method<float>											("Vec2", "Get_Y",				&Vec2_Get_Y);
	vm->Get_Bindings()->Bind_Method<void,float>										("Vec2", "Set_Y",				&Vec2_Set_Y);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,float,float>					("Vec2", "Rand",				&Vec2_Rand);
}