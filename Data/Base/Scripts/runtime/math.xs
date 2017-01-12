// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	This class is used to perform several common math operations.
// -----------------------------------------------------------------------------
public static native("Math") class Math
{
	public const float QuarterPI	= 0.78539816339744;
	public const float HalfPI		= 1.57079632679489;
	public const float PI			= 3.14159265358979;
	public const float PI2			= 6.28318530717958;
	public const float E			= 2.71828182845904;
	
	public static native("AbsF") 		float Abs		(float v);
	public static native("AbsI") 		int   Abs		(int v);
	public static native("Acos") 		float Acos		(float v);
	public static native("Asin") 		float Asin		(float v);
	public static native("Atan") 		float Atan		(float v);
	public static native("Atan2") 		float Atan2		(float y, float x);
	public static native("Ceiling") 	float Ceiling	(float v);
	public static native("Floor") 		float Floor		(float v);
	public static native("Cos") 		float Cos		(float v);
	public static native("Cosh") 		float Cosh		(float v);
	public static native("Exp") 		float Exp		(float v);
	public static native("Log") 		float Log		(float v);
	public static native("Log10") 		float Log10		(float v);
	public static native("Pow") 		float Pow		(float v1, float v2);
	public static native("Round") 		float Round		(float v);
	public static native("Sin") 		float Sin		(float v);
	public static native("Sinh") 		float Sinh		(float v);
	public static native("Sqrt") 		float Sqrt		(float v);
	public static native("Tan") 		float Tan		(float v);
	public static native("Tanh") 		float Tanh		(float v);
	public static native("Truncate") 	float Truncate	(float v);
	public static native("IsInf") 		bool  IsInf		(float v);
	public static native("isNan") 		bool  IsNAN		(float v);
	public static native("SignF")  		int 	Sign	(float v);
	public static native("SignI")  		int 	Sign	(int v);
	public static native("MaxF")  		float 	Max		(float v1, float v2);
	public static native("MaxI")  		int 	Max		(int v1, int v2);
	public static native("MinF")  		float 	Min		(float v1, float v2);
	public static native("MinI")  		int 	Min		(int v1, int v2);
	public static native("ClampI")  	int 	Clamp	(int v1, int min, int max);
	public static native("ClampF")  	float 	Clamp	(float v1, float min, float max);
	
	public static native("DegToRad")  	float 	DegToRad(float v);
	public static native("RadToDeg")  	float 	RadToDeg(float v);

	public static native("Lerp")	  	float 	Lerp		(float x, float y, float delta);
	public static native("Lerp_Vec2")	Vec2 	Lerp		(Vec2 x, Vec2 y, float delta);
	public static native("Lerp_Vec3")	Vec3 	Lerp		(Vec3 x, Vec3 y, float delta);
	public static native("Lerp_Vec4")	Vec4 	Lerp		(Vec4 x, Vec4 y, float delta);
	
	public static native("SmoothStep")		float 	SmoothStep	(float x, float y, float delta);
	public static native("SmoothStep_Vec2")	Vec2 	SmoothStep	(Vec2 x, Vec2 y, float delta);
	public static native("SmoothStep_Vec3")	Vec3 	SmoothStep	(Vec3 x, Vec3 y, float delta);
	public static native("SmoothStep_Vec4")	Vec4 	SmoothStep	(Vec4 x, Vec4 y, float delta);
    
	public static native("CurveLerp")		    float 	CurveLerp	(float start_x, float mid_x, float end_x, float mid_point, float delta);
	public static native("BezierCurveLerp")		float 	BezierCurveLerp	(float start_x, float mid_x, float end_x, float mid_point, float delta);

	public static native("RandI")		int		Rand	(int min, int max);
	public static native("RandF")		float	Rand	(float min, float max);

    public static float FixedVelocityLerp(float current, float next, float velocity)
    {
        float distance = (next - current);
        if (distance < 0.0f)
        {
            distance = Math.Max(distance, -velocity);
        }
        else 
        {
            distance = Math.Min(distance, velocity);
        }
        return current + distance;
    }
}
