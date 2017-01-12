// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATH_RANDOM_
#define _GENERIC_MATH_RANDOM_

#include <vector>

struct RandomWeight
{
	int value;
	int probability;

	RandomWeight(int InValue, int InProbability)
		: value(InValue)
		, probability(InProbability)
	{
	}
};

class Random
{
	MEMORY_ALLOCATOR(Random, "Generic");

private:
	static Random m_static;

	int m_seed;
	int m_max;

public:
	Random(int seed);

	int Next();
	int Next(int loBound, int hiBound);
	double Next_Double();
	double Next_Double(float loBound, float hiBound);

	int Next_Weighted(std::vector<RandomWeight>& weight);
	
	static void Seed_Static(int seed);
	static int Get_Static_Seed() { return m_static.m_seed; }
	static int Static_Next();
	static int Static_Next(int loBound, int hiBound);
	static double Static_Next_Double();
	static double Static_Next_Double(int loBound, int hiBound);
	static double Static_Next_Double(double loBound, double hiBound);

};

#endif