// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Math/Math.h"
#include <cstring>

double Math::Lerp(double v0, double v1, double d)
{
	return v0 + (v1 - v0) * d;
}

float Math::Lerp(float v0, float v1, float d)
{
	return v0 + (v1 - v0) * d;
}

float Math::Lerp(float v00, float v10, float v01, float v11, float dx, float dy)
{
	float u = Lerp(v00, v10, dx);
	float v = Lerp(v01, v11, dx);
	return Lerp(u, v, dy);
}

float Math::Lerp(float v000, float v100, float v010, float v110, float v001, float v101, float v011, float v111, float dx, float dy, float dz)
{	
	float u = Lerp(v000, v100, v010, v110, dx, dy);
	float v = Lerp(v001, v101, v011, v111, dx, dy);
	return Lerp(u, v, dz);
}

float Math::SmoothStep(float v0, float v1, float delta)
{
	if (v0 == v1)
	{
		return v0;
	}

	float offset = v0 + ((v1 - v0) * delta);	
	float p = Clamp((offset - v0) / (v1 - v0), 0.0f, 1.0f);
	float final_d = p*p*p*(p*(p*6 - 15) + 10);

	return v0 + ((v1 - v0) * final_d);
}

float Math::SmootherStep(float edge0, float edge1, float x)
{
	if (edge0 == edge1)
	{
		return edge0;
	}
	x = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return x*x*x*(x*(x * 6.0f - 15.0f) + 10.0f);
}

float Math::FadeInFadeOut(float start, float end, float delta, float fade_interval)
{
	float alpha = 0.0f;

	// Fade In
	if (delta <= fade_interval)
	{
		alpha = (delta / fade_interval);
	}
	// Standard
	else if (delta < 1.0f - fade_interval)
	{
		alpha = 1.0f;
	}
	// Fade Out
	else if (delta < 1.0f)
	{
		alpha = 1.0f - ((delta - (1.0f - fade_interval)) / fade_interval);
	}
	// Next tip.
	else if (delta >= 1.0f)
	{
		alpha = 0.0f;
	}

	return start + ((end - start) * alpha);
}

s16 Math::Float_To_Half(float x)
{
	u16 i16;
	u32 i32;

	memcpy(&i32, &x, sizeof(float));

	i16 = ((i32 & 0x7FFFFFFF) >> 13) - (0x38000000 >> 13);
	i16 |= ((i32 & 0x80000000) >> 16);

	return i16;
}

float Math::Half_To_Float(s16 x)
{
	int i32 = ((x & 0x8000) << 16);
	i32 |= ((x & 0x7FFF) << 13) + 0x38000000;

	float float32;
	memcpy(&float32, &i32, sizeof(float));
	return float32;
}
