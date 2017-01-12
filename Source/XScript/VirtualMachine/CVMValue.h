/* *****************************************************************

		CVMValue.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMVALUE_H_
#define _CVMVALUE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/VirtualMachine/CVMString.h"
#include "XScript/VirtualMachine/CVMObject.h"

struct CVMDataType;

// =================================================================
//	Stores a value held by the virtual machine. Essentially a variant
//	container for holding int/float/string/object's.
// =================================================================
class CVMValue
{
public:
	
	union
	{
		int				int_value;
		float			float_value;
	};
	CVMString			string_value;
	CVMObjectHandle		object_value;

	CVMValue()
		: int_value(0)
	{
	}

	CVMValue(int val)
		: int_value(val)
	{
	}

	CVMValue(float val)
		: float_value(val)
	{
	}

	CVMValue(double val)
		: float_value((float)val)
	{
	}

	CVMValue(CVMString val)
		: string_value(val)
	{
	}

	CVMValue(CVMObjectHandle val)
		: object_value(val)
	{
	}

	CVMValue(CVMObject* val)
		: object_value(val)
	{
	}

	bool Equal_To(CVMValue& other, CVMDataType* type);

	INLINE void Clear()
	{
		int_value = 0;
		float_value = 0.0f;
		string_value = "";
		object_value = NULL;
	}

	INLINE void To_Primitive(int* val)
	{
		*val = int_value;
	}

	INLINE void To_Primitive(float* val)
	{
		*val = float_value;
	}

	INLINE void To_Primitive(double* val)
	{
		*val = (float)float_value;
	}

	INLINE void To_Primitive(CVMString* val)
	{
		*val = string_value;
	}

	INLINE void To_Primitive(CVMObjectHandle* val)
	{
		*val = object_value;
	}

	INLINE void To_Primitive(CVMValue* val)
	{
		*val = *this;
	}

};

#endif