// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Math/Noise/Noise.h"
#include "Generic/Math/Noise/NoiseSampler3D.h"

NoiseSampler3D::NoiseSampler3D(Noise* noise, Vector3 position, IntVector3 size, Vector3 scale_factor, float octaves, float persistence, float scale)
	: m_noise(noise)
	, m_position(position)
	, m_size(size)
	, m_scale_factor(scale_factor)
	, m_octaves(octaves)
	, m_persistence(persistence)
	, m_scale(scale)
{
	m_samples = new float[size.X * size.Y * size.Z];
	Generate_Samples();
}

NoiseSampler3D::~NoiseSampler3D()
{
	SAFE_DELETE_ARRAY(m_samples);
}

void NoiseSampler3D::Resposition(Vector3 position)
{
	m_position = position;
	Generate_Samples();
}

int NoiseSampler3D::Flatten_Index(int x, int y, int z)
{
	return y + m_size.Y * (z + m_size.X * x);
}

void NoiseSampler3D::Generate_Samples()
{
	// Work out the "main" points in our sample latice.
	for (int x = 0; x < m_size.X; x++)
	{
		for (int y = 0; y < m_size.Y; y++)
		{
			for (int z = 0; z < m_size.Z; z++)
			{
				float pos_x = (m_position.X + x) * m_scale_factor.X;
				float pos_y = (m_position.Y + y) * m_scale_factor.Y;
				float pos_z = (m_position.Z + z) * m_scale_factor.Z;

				m_samples[Flatten_Index(x, y, z)] = m_noise->Sample_3D(m_octaves, m_persistence, m_scale, pos_x, pos_y, pos_z);
			}
		}
	}
}

float NoiseSampler3D::Sample(int x, int y, int z)
{
	return m_samples[Flatten_Index(x, y, z)];
}
