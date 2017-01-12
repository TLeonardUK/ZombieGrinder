// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Engine/EngineOptions.h"

#include "Engine/Options/OptionRegistry.h"

#define OPTION(type, name, flags, default_value, description) \
	type EngineOptions::name(default_value, #name, description, flags);
#include "Engine/Engine/EngineOptions.inc"
#undef OPTION

void EngineOptions::Register_Options()
{
	#define OPTION(type, name, flags, default_value, description) \
		OptionRegistry::Get()->Register(&name);
	#include "Engine/Engine/EngineOptions.inc"
	#undef OPTION
}

