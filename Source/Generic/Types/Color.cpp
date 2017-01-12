// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Color.h"
#include "Generic/Helper/StringHelper.h"

#include "Generic/Math/Math.h"

#include <cstdio>

Color Color::White(255, 255, 255, 255);
Color Color::Black(0, 0, 0, 255);
Color Color::Red(255, 0, 0, 255);
Color Color::Green(0, 255, 0, 255);
Color Color::Blue(0, 0, 255, 255);
Color Color::Yellow(255, 255, 0, 255);
Color Color::Orange(255, 153, 0, 255);
Color Color::Magenta(255, 0, 255, 255);
Color Color::Gray(32, 32, 32, 255);
Color Color::Aqua(0, 212, 255, 255);

Color::Color()
	: R(0)
	, G(0)
	, B(0)
	, A(0)
{
}

Color::Color(int r, int g, int b, int a)
	: R(r)
	, G(g)
	, B(b)
	, A(a)
{
}

Color::Color(float r, float g, float b, float a)
	: R((unsigned char)r)
	, G((unsigned char)g)
	, B((unsigned char)b)
	, A((unsigned char)a)
{
}

int Color::To_ARGB()
{
	return ((A << 24) | (R << 16) | (G << 8) | (B));
}

Vector3 Color::To_Vector3()
{
	static float inv = 1.0f / 255.0f;
	return Vector3(R * inv, G * inv, B * inv);
}

Vector4 Color::To_Vector4()
{
	static float inv = 1.0f / 255.0f;
	return Vector4(R * inv, G * inv, B * inv, A * inv);
}

Color Color::Saturate(float fraction)
{
	return Color(
		Clamp(R * fraction, 0.0f, 255.0f),
		Clamp(G * fraction, 0.0f, 255.0f),
		Clamp(B * fraction, 0.0f, 255.0f),
		Clamp(A * fraction, 0.0f, 255.0f)
	);
}

bool Color::operator==(Color& rhs) const
{
	return (R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A);
}

bool Color::operator!=(Color& rhs) const
{
	return !operator==(rhs);
}

Color Color::operator* (const Color a) const
{
	return Color
	(
		R * (a.R/255.0f), 
		G * (a.G/255.0f), 
		B * (a.B/255.0f), 
		A * (a.A/255.0f)
	);
}

std::string Color::To_String()
{
	return StringHelper::Format("%d,%d,%d,%d", R, G, B, A);
}

bool Color::Parse(const char* value, Color& output)
{
	int r = 255, g = 255, b = 255, a = 255;
	int count = sscanf(value, "%d,%d,%d,%d", &r, &g, &b, &a);
	
	if (count >= 3)
	{
		output = Color(r, g, b, a);
		return true;
	}

	return false;
}
