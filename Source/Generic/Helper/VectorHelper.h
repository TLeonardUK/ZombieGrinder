// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_HELPER_VECTORHELPER_
#define _GENERIC_HELPER_VECTORHELPER_

#include <vector>

class VectorHelper
{
private:
	VectorHelper(){} // Static class!

public:

	template <typename To, typename From>
	static std::vector<To> Cast(typename std::vector<From> original)
	{
		std::vector<To> result;

		for (typename std::vector<From>::iterator iter = original.begin(); iter != original.end(); iter++)
		{
			result.push_back(static_cast<To>(*iter));
		}

		return result;
	}
		
};

#endif