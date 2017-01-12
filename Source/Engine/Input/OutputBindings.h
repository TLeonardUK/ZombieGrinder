// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_OUTPUTBINDINGS_
#define _ENGINE_INPUT_OUTPUTBINDINGS_

#include <vector>
#include <cstring>
#include <cstdlib>

struct OutputBindings
{
	enum Type
	{
#define BINDING(x, description) x,
#include "Engine/Input/OutputBindings.inc"
#undef BINDING
	};

	static bool Parse(const char* name, OutputBindings::Type& result)
	{
#define BINDING(x, description) if (stricmp(name, #x) == 0) { result = x; return true; }
#include "Engine/Input/OutputBindings.inc"
#undef BINDING
		return false;
	}

	static bool ToString(OutputBindings::Type& type, const char*& result)
	{
#define BINDING(x, description) if (x == type) { result = #x; return true; }
#include "Engine/Input/OutputBindings.inc"
#undef BINDING
		return false;
	}
	static bool ToDescription(OutputBindings::Type& type, const char*& result)
	{
#define BINDING(x, description) if (x == type) { result = description; return true; }
#include "Engine/Input/OutputBindings.inc"
#undef BINDING
		return false;
	}
};

#endif