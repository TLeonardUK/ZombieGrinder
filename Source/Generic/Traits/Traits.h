// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_TRAITS_TRAITS_
#define _GENERIC_TRAITS_TRAITS_

struct FalseTraitType
{
};

struct TrueTraitType
{
};

struct FalseTrait
{
	static const bool Value;
	static const FalseTraitType Type;
};

struct TrueTrait
{
	static const bool Value;
	static const TrueTraitType Type;
};

#endif
