// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_COMPILED_LANGUAGE_
#define _ENGINE_RESOURCES_COMPILED_LANGUAGE_

struct CompiledLanguageString
{
	u32			name_hash;
	const char* name;
	const char* value;
};

struct CompiledLanguageHeader
{
	u32 string_count;
	u32 short_name_string_index;
	u32 long_name_string_index;
	u32 glyphs_string_index;
	CompiledLanguageString* strings;
};

#endif