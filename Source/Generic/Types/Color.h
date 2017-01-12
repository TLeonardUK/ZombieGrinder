// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_COLOR_
#define _GENERIC_COLOR_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/Vector4.h"

struct Color
{
	//MEMORY_ALLOCATOR(Color, "Data Types");

public:
	// Some commonly used colors.
	static Color White;
	static Color Black;
	static Color Red;
	static Color Green;
	static Color Blue;
	static Color Yellow;
	static Color Orange;
	static Color Magenta;
	static Color Gray;
	static Color Aqua;

public:
	unsigned char R, G, B, A;

	Color();
	Color(int r, int g, int b, int a);
	Color(float r, float g, float b, float a);

	Vector3 To_Vector3();
	Vector4 To_Vector4();

	int To_ARGB();
	
	Color Saturate(float fraction);
	
	// Operators.
	bool operator==(Color& rhs) const;
	bool operator!=(Color& rhs) const;
	Color operator* (const Color a) const;
	
	std::string To_String();
	static bool Parse(const char* value, Color& output);

};

#endif