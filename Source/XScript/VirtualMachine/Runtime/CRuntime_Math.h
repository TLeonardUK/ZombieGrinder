/* *****************************************************************

		CRuntime_Math.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CRUNTIME_MATH_H_
#define _CRUNTIME_MATH_H_

class CVirtualMachine;

class CRuntime_Math
{
public:
	static float AbsF(CVirtualMachine* vm, float v);
	static int AbsI(CVirtualMachine* vm, int v);
	static float Acos(CVirtualMachine* vm, float v);
	static float Asin(CVirtualMachine* vm, float v);
	static float Atan(CVirtualMachine* vm, float v);
	static float Atan2(CVirtualMachine* vm, float x, float y);
	static float Ceiling(CVirtualMachine* vm, float v);
	static float Floor(CVirtualMachine* vm, float v);
	static float Cos(CVirtualMachine* vm, float v);
	static float Cosh(CVirtualMachine* vm, float v);
	static float Exp(CVirtualMachine* vm, float v);
	static float Log(CVirtualMachine* vm, float v);
	static float Log10(CVirtualMachine* vm, float v);
	static float Pow(CVirtualMachine* vm, float x, float y);
	static float Round(CVirtualMachine* vm, float v);
	static float Sin(CVirtualMachine* vm, float v);
	static float Sinh(CVirtualMachine* vm, float v);
	static float Sqrt(CVirtualMachine* vm, float v);
	static float Tan(CVirtualMachine* vm, float v);
	static float Tanh(CVirtualMachine* vm, float v);
	static float Truncate(CVirtualMachine* vm, float v);
	static int IsInf(CVirtualMachine* vm, float v);
	static int IsNan(CVirtualMachine* vm, float v);
	static int SignF(CVirtualMachine* vm, float v);
	static int SignI(CVirtualMachine* vm, int v);
	static float MaxF(CVirtualMachine* vm, float x, float y);
	static int MaxI(CVirtualMachine* vm, int x, int y);
	static float MinF(CVirtualMachine* vm, float x, float y);
	static int MinI(CVirtualMachine* vm, int x, int y);
	static int ClampI(CVirtualMachine* vm, int x, int min, int max);
	static float ClampF(CVirtualMachine* vm, float x, float min, float max); 	
	static float DegToRad_(CVirtualMachine* vm, float v);
	static float RadToDeg_(CVirtualMachine* vm, float v);

	static float Lerp(CVirtualMachine* vm, float s, float e, float d);
	static CVMObjectHandle Lerp_Vec2(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d);
	static CVMObjectHandle Lerp_Vec3(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d);
	static CVMObjectHandle Lerp_Vec4(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d);

	static float SmoothStep(CVirtualMachine* vm, float s, float e, float d);
	static CVMObjectHandle SmoothStep_Vec2(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d);
	static CVMObjectHandle SmoothStep_Vec3(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d);
	static CVMObjectHandle SmoothStep_Vec4(CVirtualMachine* vm, CVMObjectHandle s, CVMObjectHandle e, float d);

	static float CurveLerp(CVirtualMachine* vm, float start_x, float mid_x, float end_x, float mid_point, float delta);
	static float BezierCurveLerp(CVirtualMachine* vm, float start_x, float mid_x, float end_x, float mid_point, float delta);

	static int RandI(CVirtualMachine* vm, int min, int max);
	static float RandF(CVirtualMachine* vm, float min, float max);

	static void Bind(CVirtualMachine* machine);

};

#endif