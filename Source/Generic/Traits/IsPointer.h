// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_TRAITS_ISPOINTER_
#define _GENERIC_TRAITS_ISPOINTER_

#include "Generic/Traits/Traits.h"

// Used with template metaprogrammer to determine if a
// type argument is a pointer!

// Example to call different templates if pointer/not;
//
//	template<T>
//	Template_Func(T derp)
//	{
//		Internal_Func<T>(IsPointer<T>::Type, derp);
//	}
//
//	template<T>
//	Internal_Func(TrueTraitType, T derp)
//	{
//		Do pointer shenanigans
//	}
//
//	template<T>
//	Internal_Func(FalseTraitType, T derp)
//	{
//		Do non-pointer shenanigans
//	}

template<typename T>
struct IsPointer : FalseTrait
{
};

template<typename T>
struct IsPointer<T*> : TrueTrait
{
};

#endif
