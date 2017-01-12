/* *****************************************************************

		CRuntime_Math.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Math.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include <cstdlib>
#include <cmath>  

float CRuntime_Math::AbsF(CVirtualMachine* vm, float v)
{
	return fabs(v);
}

int CRuntime_Math::AbsI(CVirtualMachine* vm, int v)
{
	return abs(v);
}

float CRuntime_Math::Acos(CVirtualMachine* vm, float v)
{
	return acos(v);
}

float CRuntime_Math::Asin(CVirtualMachine* vm, float v)
{
	return asin(v);
}

float CRuntime_Math::Atan(CVirtualMachine* vm, float v)
{
	return atan(v);
}

float CRuntime_Math::Atan2(CVirtualMachine* vm, float x, float y)
{
	return atan2(x,y);
}

float CRuntime_Math::Ceiling(CVirtualMachine* vm, float v)
{
	return ceilf(v);
}

float CRuntime_Math::Floor(CVirtualMachine* vm, float v)
{
	return floorf(v);
}

float CRuntime_Math::Cos(CVirtualMachine* vm, float v)
{
	return cos(v);
}

float CRuntime_Math::Cosh(CVirtualMachine* vm, float v)
{
	return cosh(v);
}

float CRuntime_Math::Exp(CVirtualMachine* vm, float v)
{
	return exp(v);
}

float CRuntime_Math::Log(CVirtualMachine* vm, float v)
{
	return log(v);
}

float CRuntime_Math::Log10(CVirtualMachine* vm, float v)
{
	return log10(v);
}

float CRuntime_Math::Pow(CVirtualMachine* vm, float x, float y)
{
	return pow(x,y);
}

float CRuntime_Math::Round(CVirtualMachine* vm, float v)
{
	return FastRound(v);
}

float CRuntime_Math::Sin(CVirtualMachine* vm, float v)
{
	return sin(v);
}

float CRuntime_Math::Sinh(CVirtualMachine* vm, float v)
{
	return sinh(v);
}

float CRuntime_Math::Sqrt(CVirtualMachine* vm, float v)
{
	return sqrt(v);
}

float CRuntime_Math::Tan(CVirtualMachine* vm, float v)
{
	return tan(v);
}

float CRuntime_Math::Tanh(CVirtualMachine* vm, float v)
{
	return tanh(v);
}

float CRuntime_Math::Truncate(CVirtualMachine* vm, float v)
{
	return RoundToZero(v);
}

int CRuntime_Math::IsInf(CVirtualMachine* vm, float v)
{
	return !IsNan(vm, v) && IsNan(vm, v - v);
}

int CRuntime_Math::IsNan(CVirtualMachine* vm, float v)
{
	return (v != v) ? 1 : 0;
}

int CRuntime_Math::SignF(CVirtualMachine* vm, float v)
{
	return (v == 0.0f ? 0 : (v < 0.0f ? -1 : 1));
}

int CRuntime_Math::SignI(CVirtualMachine* vm, int v)
{
	return (v == 0 ? 0 : (v < 0 ? -1 : 1));
}

float CRuntime_Math::MaxF(CVirtualMachine* vm, float x, float y)
{
	return x > y ? x : y;
}

int CRuntime_Math::MaxI(CVirtualMachine* vm, int x, int y)
{
	return x > y ? x : y;
}

float CRuntime_Math::MinF(CVirtualMachine* vm, float x, float y)
{
	return x < y ? x : y;
}

int CRuntime_Math::MinI(CVirtualMachine* vm, int x, int y)
{
	return x < y ? x : y;
}

int CRuntime_Math::ClampI(CVirtualMachine* vm, int x, int min, int max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

float CRuntime_Math::ClampF(CVirtualMachine* vm, float x, float min, float max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

float CRuntime_Math::DegToRad_(CVirtualMachine* vm, float v)
{
	return DegToRad(v);
}

float CRuntime_Math::RadToDeg_(CVirtualMachine* vm, float v)
{
	return RadToDeg(v);
}

float CRuntime_Math::Lerp(CVirtualMachine* vm, float s, float e, float d)
{
	return Math::Lerp(s, e, d);
}

CVMObjectHandle CRuntime_Math::Lerp_Vec2(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d)
{
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = Math::Lerp(s.Get()->Get_Slot(0).float_value, e.Get()->Get_Slot(0).float_value, d);
	obj_result->Get_Slot(1).float_value = Math::Lerp(s.Get()->Get_Slot(1).float_value, e.Get()->Get_Slot(1).float_value, d);

	return result;
}

CVMObjectHandle CRuntime_Math::Lerp_Vec3(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d)
{
	CVMLinkedSymbol* vec3_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec3_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = Math::Lerp(s.Get()->Get_Slot(0).float_value, e.Get()->Get_Slot(0).float_value, d);
	obj_result->Get_Slot(1).float_value = Math::Lerp(s.Get()->Get_Slot(1).float_value, e.Get()->Get_Slot(1).float_value, d);
	obj_result->Get_Slot(2).float_value = Math::Lerp(s.Get()->Get_Slot(2).float_value, e.Get()->Get_Slot(2).float_value, d);

	return result;
}

CVMObjectHandle CRuntime_Math::Lerp_Vec4(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d)
{
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = Math::Lerp(s.Get()->Get_Slot(0).float_value, e.Get()->Get_Slot(0).float_value, d);
	obj_result->Get_Slot(1).float_value = Math::Lerp(s.Get()->Get_Slot(1).float_value, e.Get()->Get_Slot(1).float_value, d);
	obj_result->Get_Slot(2).float_value = Math::Lerp(s.Get()->Get_Slot(2).float_value, e.Get()->Get_Slot(2).float_value, d);
	obj_result->Get_Slot(3).float_value = Math::Lerp(s.Get()->Get_Slot(3).float_value, e.Get()->Get_Slot(3).float_value, d);

	return result;
}

float CRuntime_Math::SmoothStep(CVirtualMachine* vm, float s, float e, float d)
{
	return Math::SmoothStep(s, e, d);
}

CVMObjectHandle CRuntime_Math::SmoothStep_Vec2(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d)
{
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = Math::SmoothStep(s.Get()->Get_Slot(0).float_value, e.Get()->Get_Slot(0).float_value, d);
	obj_result->Get_Slot(1).float_value = Math::SmoothStep(s.Get()->Get_Slot(1).float_value, e.Get()->Get_Slot(1).float_value, d);

	return result;
}

CVMObjectHandle CRuntime_Math::SmoothStep_Vec3(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d)
{
	CVMLinkedSymbol* vec3_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec3_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(3);
	obj_result->Get_Slot(0).float_value = Math::SmoothStep(s.Get()->Get_Slot(0).float_value, e.Get()->Get_Slot(0).float_value, d);
	obj_result->Get_Slot(1).float_value = Math::SmoothStep(s.Get()->Get_Slot(1).float_value, e.Get()->Get_Slot(1).float_value, d);
	obj_result->Get_Slot(2).float_value = Math::SmoothStep(s.Get()->Get_Slot(2).float_value, e.Get()->Get_Slot(2).float_value, d);

	return result;
}

CVMObjectHandle CRuntime_Math::SmoothStep_Vec4(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d)
{
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = Math::SmoothStep(s.Get()->Get_Slot(0).float_value, e.Get()->Get_Slot(0).float_value, d);
	obj_result->Get_Slot(1).float_value = Math::SmoothStep(s.Get()->Get_Slot(1).float_value, e.Get()->Get_Slot(1).float_value, d);
	obj_result->Get_Slot(2).float_value = Math::SmoothStep(s.Get()->Get_Slot(2).float_value, e.Get()->Get_Slot(2).float_value, d);
	obj_result->Get_Slot(3).float_value = Math::SmoothStep(s.Get()->Get_Slot(3).float_value, e.Get()->Get_Slot(3).float_value, d);

	return result;
}


int CRuntime_Math::RandI(CVirtualMachine* vm, int min, int max)
{
	return Random::Static_Next(min, max);
}

float CRuntime_Math::RandF(CVirtualMachine* vm, float min, float max)
{
	float val = (float)Random::Static_Next_Double(min, max);
	return (float)val;
}

float CRuntime_Math::CurveLerp(CVirtualMachine* vm, float start_x, float mid_x, float end_x, float mid_point, float delta)
{
	if (delta < mid_point)
	{
		float real_delta = delta / mid_point;
		return start_x + ((mid_x - start_x) * real_delta);
	}
	else
	{
		float real_delta = (delta - mid_point) / (1.0f - mid_point);
		return mid_x + ((end_x - mid_x) * real_delta);
	}
}

float GetBezierCurvePoint(float n1, float n2, float perc)
{
	float diff = n2 - n1;
	return n1 + (diff * perc);
}

float CRuntime_Math::BezierCurveLerp(CVirtualMachine* vm, float start_x, float mid_x, float end_x, float mid_point, float delta)
{
	float xa = GetBezierCurvePoint(start_x, mid_x, delta);
//	float ya = GetBezierCurvePoint(start_y, mid_y, delta);
	float xb = GetBezierCurvePoint(mid_x, end_x, delta);
//	float yb = GetBezierCurvePoint(mid_y, end_y, delta);

	float x = GetBezierCurvePoint(xa, xb, delta);
//	float y = GetBezierCurvePoint(ya, yb, delta);

	return x;
}

void CRuntime_Math::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "AbsF",		&AbsF);
	vm->Get_Bindings()->Bind_Function<int,int>						("Math", "AbsI",		&AbsI);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Acos",		&Acos);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Asin",		&Asin);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Atan",		&Atan);
	vm->Get_Bindings()->Bind_Function<float,float,float>			("Math", "Atan2",		&Atan2);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Ceiling",	&Ceiling);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Floor",		&Floor);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Cos",		&Cos);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Cosh",		&Cosh);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Exp",		&Exp);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Log",		&Log);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Log10",		&Log10);
	vm->Get_Bindings()->Bind_Function<float,float,float>			("Math", "Pow",		&Pow);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Round",		&Round);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Sin",		&Sin);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Sinh",		&Sinh);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Sqrt",		&Sqrt);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Tan",		&Tan);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Tanh",		&Tanh);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "Truncate",	&Truncate);
	vm->Get_Bindings()->Bind_Function<int,float>					("Math", "IsInf",		&IsInf);
	vm->Get_Bindings()->Bind_Function<int,float>					("Math", "isNan",		&IsNan);
	vm->Get_Bindings()->Bind_Function<int,float>					("Math", "SignF",		&SignF);
	vm->Get_Bindings()->Bind_Function<int,int>						("Math", "SignI",		&SignI);
	vm->Get_Bindings()->Bind_Function<float,float,float>			("Math", "MaxF",		&MaxF);
	vm->Get_Bindings()->Bind_Function<int,int,int>					("Math", "MaxI",		&MaxI);
	vm->Get_Bindings()->Bind_Function<float,float,float>			("Math", "MinF",		&MinF);
	vm->Get_Bindings()->Bind_Function<int,int,int>					("Math", "MinI",		&MinI);
	vm->Get_Bindings()->Bind_Function<int,int,int,int>				("Math", "ClampI",		&ClampI);
	vm->Get_Bindings()->Bind_Function<float,float,float,float>		("Math", "ClampF",		&ClampF);
	
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "DegToRad",	&DegToRad_);
	vm->Get_Bindings()->Bind_Function<float,float>					("Math", "RadToDeg",	&RadToDeg_);

	vm->Get_Bindings()->Bind_Function<float,float,float,float>		("Math", "Lerp",		&Lerp);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle,float>		("Math", "Lerp_Vec2",		&Lerp_Vec2);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle,float>		("Math", "Lerp_Vec3",		&Lerp_Vec3);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle,float>		("Math", "Lerp_Vec4",		&Lerp_Vec4);

	vm->Get_Bindings()->Bind_Function<float,float,float,float>		("Math", "SmoothStep",	&SmoothStep);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle,float>		("Math", "SmoothStep_Vec2",		&SmoothStep_Vec2);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle,float>		("Math", "SmoothStep_Vec3",		&SmoothStep_Vec3);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle,float>		("Math", "SmoothStep_Vec4",		&SmoothStep_Vec4);

	vm->Get_Bindings()->Bind_Function<int,int,int>					("Math", "RandI",		&RandI);
	vm->Get_Bindings()->Bind_Function<float,float,float>			("Math", "RandF",		&RandF);

	vm->Get_Bindings()->Bind_Function<float,float,float,float,float,float>			("Math", "CurveLerp",		&CurveLerp);
	vm->Get_Bindings()->Bind_Function<float, float, float, float, float, float>("Math", "BezierCurveLerp", &BezierCurveLerp);
}