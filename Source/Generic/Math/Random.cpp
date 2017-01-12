// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Math/Random.h"

#include <cstdlib>

Random Random::m_static(rand());

Random::Random(int seed)
{
	m_seed = seed;
	m_max  = 0x7FFFFFFF;//2147483647;
}

int Random::Next()
{
	// IBM's fastrand implementation.
	m_seed = (214013 * m_seed + 2531011); 
	return m_seed & 0x7FFFFFFF;
}

int Random::Next(int loBound, int hiBound)
{
	int range = (hiBound - loBound);
	if (range == 0)
	{
		return loBound;
	}
	return loBound + (Next() % range);
//	return Next() % (hiBound + loBound - 1) + loBound;
}

double Random::Next_Double()
{
	return (float)Next() / (float)m_max;
}

double Random::Next_Double(float loBound, float hiBound)
{
	float range = (hiBound + loBound);
	return loBound + ((float)Next_Double() * (hiBound - loBound));
}

int Random::Next_Weighted(std::vector<RandomWeight>& weights)
{
	int total = 0;
	for (std::vector<RandomWeight>::iterator iter = weights.begin(); iter != weights.end(); iter++)
	{
		RandomWeight& weight = *iter;
		total += weight.probability;
	}

	int value = Next(0, total);

	total = 0;
	for (std::vector<RandomWeight>::iterator iter = weights.begin(); iter != weights.end(); iter++)
	{
		RandomWeight& weight = *iter;
		int range_start = total;
		int range_end = total + weight.probability;

		if (value >= range_start && value < range_end)
		{
			return weight.value;
		}

		total += weight.probability;
	}

	DBG_ASSERT(false);
	return 0;
}

int Random::Static_Next()
{
	return m_static.Next();
}

int Random::Static_Next(int loBound, int hiBound)
{
	int range = (hiBound - loBound);
	if (range == 0)
	{
		return loBound;
	}
	return loBound + (range == 0 ? 0 : Static_Next() % range);
}

double Random::Static_Next_Double()
{
	return (float)Static_Next() / (float)m_static.m_max;
}

double Random::Static_Next_Double(int loBound, int hiBound)
{
	return loBound + (Static_Next_Double() * (hiBound - loBound));
}

double Random::Static_Next_Double(double loBound, double hiBound)
{
	return loBound + (Static_Next_Double() * (hiBound - loBound));
}

void Random::Seed_Static(int seed)
{
	m_static = Random(seed);
}
