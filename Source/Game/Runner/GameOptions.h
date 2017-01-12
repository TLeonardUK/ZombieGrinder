// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_OPTIONS_
#define _GAME_OPTIONS_

#include "Engine/Options/Types/BoolOption.h"
#include "Engine/Options/Types/FloatOption.h"
#include "Engine/Options/Types/IntOption.h"
#include "Engine/Options/Types/StringOption.h"
#include "Engine/Options/Types/StructListOption.h"

#include "Engine/Input/InputBindings.h"
#include "Engine/Input/OutputBindings.h"

struct InputBindingConfig
{
	MEMORY_ALLOCATOR(InputBindingConfig, "Game");

public:
	InputBindings::Type input;
	OutputBindings::Type output;

	InputBindingConfig()
		: input(InputBindings::INVALID)
		, output(OutputBindings::INVALID)
	{
	}

	InputBindingConfig(InputBindings::Type in, OutputBindings::Type out)
		: input(in)
		, output(out)
	{
	}
	
	bool Serialize(BinaryStream& stream)
	{
		stream.Write<unsigned short>(input);
		stream.Write<unsigned short>(output);
		return true;
	}

	bool Deserialize(BinaryStream& stream, unsigned int version)
	{
		input  = (InputBindings::Type)stream.Read<unsigned short>();
		output = (OutputBindings::Type)stream.Read<unsigned short>();
		return true;
	}

};

class GameOptions
{
	MEMORY_ALLOCATOR(GameOptions, "Game");

public:
	static std::vector<InputBindingConfig> g_default_input_bindings;

private:
	static void Register_Default_Input_Bindings();

public:
	#define OPTION(type, name, flags, default_value, description) \
		static type name;
	#include "Game/Runner/GameOptions.inc"
	#undef OPTION

	static void Register_Options();
};

#endif

