// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
//	A lot of the code in this file is based off code provided at;
//		https://code.google.com/p/battlestar-tux/
//	Full credit to them!
// ===================================================================
#include "Generic/Math/Noise/Noise.h"

float Noise::Sample_2D(float octaves, float persistence, float scale, float x, float y)
{
	float total		= 0;
	float frequency = scale;
	float amplitude = 1;

	// We have to keep track of the largest possible amplitude,
	// because each octave adds more, and we need a value in [-1, 1].
	float maxAmplitude = 0;

	for (int i = 0; i < octaves; i++) 
	{
		total			+= Raw_Sample_2D(x * frequency, y * frequency) * amplitude;

		frequency		*= 2;
		maxAmplitude	+= amplitude;
		amplitude		*= persistence;
	}

	return total / maxAmplitude;
}

float Noise::Sample_3D(float octaves, float persistence, float scale, float x, float y, float z)
{
    float total			= 0;
    float frequency		= scale;
    float amplitude		= 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    float maxAmplitude = 0;

    for (int i = 0; i < octaves; i++) 
	{
        total			+= Raw_Sample_3D(x * frequency, y * frequency, z * frequency) * amplitude;

        frequency		*= 2;
        maxAmplitude	+= amplitude;
        amplitude		*= persistence;
    }

    return total / maxAmplitude;
}

float Noise::Sample_4D(float octaves, float persistence, float scale, float x, float y, float z, float w)
{
    float total			= 0;
    float frequency		= scale;
    float amplitude		= 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    float maxAmplitude = 0;

    for (int i = 0; i < octaves; i++) 
	{
        total			+= Raw_Sample_4D( x * frequency, y * frequency, z * frequency, w * frequency ) * amplitude;

        frequency		*= 2;
        maxAmplitude	+= amplitude;
        amplitude		*= persistence;
    }

    return total / maxAmplitude;
}

float Noise::Sample_2D_In_Range(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y)
{
    return Sample_2D(octaves, persistence, scale, x, y) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

float Noise::Sample_3D_In_Range(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y, float z)
{    
	return Sample_3D(octaves, persistence, scale, x, y, z) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

float Noise::Sample_4D_In_Range(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y, float z, float w)
{
    return Sample_4D(octaves, persistence, scale, x, y, z, w) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

float Noise::Raw_Sample_2D_In_Range(float loBound, float hiBound, float x, float y)
{
	return Raw_Sample_2D(x, y) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

float Noise::Raw_Sample_3D_In_Range(float loBound, float hiBound, float x, float y, float z)
{
	return Raw_Sample_3D(x, y, z) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}

float Noise::Raw_Sample_4D_In_Range(float loBound, float hiBound, float x, float y, float z, float w)
{
	return Raw_Sample_4D(x, y, z, w) * (hiBound - loBound) / 2 + (hiBound + loBound) / 2;
}
