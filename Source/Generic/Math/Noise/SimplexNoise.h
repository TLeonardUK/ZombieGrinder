// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATH_SIMPLEXNOISE_
#define _GENERIC_MATH_SIMPLEXNOISE_

#include "Generic/Math/Random.h"

#include "Generic/Math/Noise/Noise.h"

class SimplexNoise : public Noise
{
	MEMORY_ALLOCATOR(SimplexNoise, "Generic");

private:
	Random		m_random;
	int			m_seed;
	int			m_permutations[512];

	static int	m_gradiant3D[12][3];
	static int	m_gradiant4D[32][4];
	static int	m_simplex[64][4];

	static float Dot(const int* g, const float x, const float y);
	static float Dot(const int* g, const float x, const float y, const float z);
	static float Dot(const int* g, const float x, const float y, const float z, const float w);

public:
	SimplexNoise(int seed);

	float Raw_Sample_2D(float x, float y);
	float Raw_Sample_3D(float x, float y, float z);
	float Raw_Sample_4D(float x, float y, float z, float w);

};

#endif