/* *****************************************************************

		CCollectionHelper.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCOLLECTIONHELPER_H_
#define _CCOLLECTIONHELPER_H_

#include "Generic/Types/String.h"
#include <vector>

// =================================================================
//	Class contains several collection helper functions.
// =================================================================
class CCollectionHelper
{
private:
	CCollectionHelper();

public:

	template <typename T>
	static int VectorIndexOf(std::vector<T>& vector, T object)
	{
		int index = 0;
		for (typename std::vector<T>::iterator iter = vector.begin(); iter != vector.end(); iter++, index++)
		{
			if (*iter == object)
			{
				return index;
			}
		}
		return -1;
	}
	
	template <typename T>
	static bool VectorAddIfNotExists(std::vector<T>& vector, T object)
	{
		int index = 0;
		for (typename std::vector<T>::iterator iter = vector.begin(); iter != vector.end(); iter++, index++)
		{
			if (*iter == object)
			{
				return false;
			}
		}

		vector.push_back(object);
		return true;
	}

};

#endif