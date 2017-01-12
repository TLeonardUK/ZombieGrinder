// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Main.h"
#include "Builder/Builder/AssetBuilder.h"

#include "Engine/IO/BinaryStream.h"
#include "Engine/IO/CompressedStream.h"

// Maximum CPU memory usage for the builder.
int max_cpu_memory = 512 * 1024 * 1024;

void Print_Game_Version()
{
	DBG_LOG(" Asset Builder");
}

void Game_Entry_Point(const char* command_line)
{
	// Always show log information in master versions of the builder.
	Platform_Enable_Log_File();

	AssetBuilder* builder = new AssetBuilder(); 
	builder->Run(command_line);
}
