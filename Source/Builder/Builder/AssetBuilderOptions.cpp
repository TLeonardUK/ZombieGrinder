// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Builder/AssetBuilderOptions.h"

#include "Engine/Options/OptionRegistry.h"

#define OPTION(type, name, flags, default_value, description) \
	type AssetBuilderOptions::name(default_value, #name, description, flags);
#include "Builder/Builder/AssetBuilderOptions.inc"
#undef OPTION

void AssetBuilderOptions::Register_Options()
{
#define OPTION(type, name, flags, default_value, description) \
	OptionRegistry::Get()->Register(&name);
#include "Builder/Builder/AssetBuilderOptions.inc"
#undef OPTION
}

