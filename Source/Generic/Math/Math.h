// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATH_
#define _GENERIC_MATH_

#include <math.h>

// Yay PI!
#define PI					3.14159265358979f
#define PI2					(PI * 2)
#define HALFPI				(PI * 0.5F)

// Angle conversion.
#define DegToRadFactor		(PI / 180.0f)
#define RadToDegFactor		(180.0f / PI)
#define DegToRad(x)			((x) * (PI / 180.0f))
#define RadToDeg(x)			((x) * (180.0f / PI))

// Min/Max/Clamp etc
#define Clamp(x, min, max)	((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))  
#define Min(x, y)			((x) < (y) ? (x) : (y))
#define Max(x, y)			((x) > (y) ? (x) : (y))

// Speedy stuff.
#define FastRound(x)		floorf((x) + 0.5f)
#define FastSign(x)			((0 < (x)) - ((x) < 0))
#define FastFloor(x)		(((x) > 0) ? (int)(x) : (int)(x) - 1)
#define RoundToZero(x)		((x) < 0 ? ceil((x)) : floor((x)))
#define RoundToNonZero(x)	((x) >= 0 ? ceil((x)) : floor((x)))
#define Sgn(x)				(0 < (x)) - ((x) < 0) 
#define SgnF(x)				(0.0f < (x)) - ((x) < 0.0f) 

// Float error stuff
#define IsIndeterminate(x)	((x) != (x))

// General math functions.
class Math
{
private:
	Math();

public:
	static double Lerp	(double v0, double v1, double d);
	static float Lerp	(float v0, float v1, float d);
	static float Lerp	(float v00, float v10, float v01, float v11, float dx, float dy);
	static float Lerp	(float v000, float v100, float v010, float v110, float v001, float v101, float v011, float v111, float dx, float dy, float dz);

	static float SmoothStep	(float v0, float v1, float d);
	static float SmootherStep(float edge0, float edge1, float x);

	static float FadeInFadeOut(float start, float end, float delta, float fade_interval);

	static s16 Float_To_Half(float x);
	static float Half_To_Float(s16 x);

};

#endif