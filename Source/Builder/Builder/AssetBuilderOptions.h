// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ASSET_BUILDER_OPTIONS_
#define _ASSET_BUILDER_OPTIONS_

#include "Engine/Options/Types/BoolOption.h"
#include "Engine/Options/Types/FloatOption.h"
#include "Engine/Options/Types/IntOption.h"
#include "Engine/Options/Types/LongOption.h"
#include "Engine/Options/Types/StringOption.h"
#include "Engine/Options/Types/StructListOption.h"

class AssetBuilderOptions
{
	MEMORY_ALLOCATOR(AssetBuilderOptions, "Engine");

private:		

public:
#define OPTION(type, name, flags, default_value, description) \
	static type name;
#include "Builder/Builder/AssetBuilderOptions.inc"
#undef OPTION

	static void Register_Options();
};

#endif

