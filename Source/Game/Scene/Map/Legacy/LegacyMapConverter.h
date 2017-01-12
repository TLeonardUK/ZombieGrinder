// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_MAP_LEGACY_LEGACYMAPLOADER_
#define _ENGINE_MAP_LEGACY_LEGACYMAPLOADER_

#include "Generic/Types/HashTable.h"

class Map;
class Actor;
class Stream;

class CVirtualMachine;
struct CVMLinkedSymbol;

class MapFile;

class LegacyMapConverter
{
	MEMORY_ALLOCATOR(LegacyMapConverter, "Scene");

private:
	HashTable<const char*, unsigned int> m_tileset_map;
	HashTable<const char*, unsigned int> m_image_map;

	// Entity loading.
	void Load_Player_Start	(MapFile* map, int map_version, Stream* stream);
	void Load_Decal			(MapFile* map, int map_version, Stream* stream);
	void Load_Entity		(MapFile* map, int map_version, Stream* stream, bool with_size = false);

public:
	LegacyMapConverter();
	
	// Map loading
	bool Convert(const char* path, const char* to_path);

};

#endif

