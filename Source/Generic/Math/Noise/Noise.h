// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATH_NOISE_
#define _GENERIC_MATH_NOISE_

class Noise
{
	MEMORY_ALLOCATOR(Noise, "Generic");

private:

public:
	virtual float Raw_Sample_2D(float x, float y) = 0;
	virtual float Raw_Sample_3D(float x, float y, float z) = 0;
	virtual float Raw_Sample_4D(float x, float y, float z, float w) = 0;

	float Sample_2D(float octaves, float persistence, float scale, float x, float y);
	float Sample_3D(float octaves, float persistence, float scale, float x, float y, float z);
	float Sample_4D(float octaves, float persistence, float scale, float x, float y, float z, float w);

	float Sample_2D_In_Range(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y);
	float Sample_3D_In_Range(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y, float z);
	float Sample_4D_In_Range(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y, float z, float w);

	float Raw_Sample_2D_In_Range(float loBound, float hiBound, float x, float y);
	float Raw_Sample_3D_In_Range(float loBound, float hiBound, float x, float y, float z);
	float Raw_Sample_4D_In_Range(float loBound, float hiBound, float x, float y, float z, float w);

};

#endif