// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATH_NOISE_SAMPLER3D_
#define _GENERIC_MATH_NOISE_SAMPLER3D_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/IntVector3.h"

class Noise;

class NoiseSampler3D
{
	MEMORY_ALLOCATOR(NoiseSampler3D, "Generic");

private:
	Noise* m_noise;
	float* m_samples;

	Vector3 m_step;
	Vector3 m_position;
	IntVector3 m_size;

	Vector3 m_scale_factor;

	float m_octaves;
	float m_persistence;
	float m_scale;

	int Flatten_Index(int x, int y, int z);
	void Generate_Samples();

public:
	NoiseSampler3D(Noise* noise, Vector3 position, IntVector3 size, Vector3 scale_factor, float octaves, float persistence, float scale);
	~NoiseSampler3D();

	void Resposition(Vector3 position);

	float Sample(int x, int y, int z);

};

#endif