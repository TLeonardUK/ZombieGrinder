// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_MAP_MAPFILE_
#define _ENGINE_SCENE_MAP_MAPFILE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/IO/BinaryStream.h"

#include "Engine/Config/ConfigFile.h"

#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"
#include "Engine/Scene/Map/Blocks/MapFileBlock.h"

#include <vector>

class MapFileFactory;
class TextureHandle;
class SoundHandle;
class Pixelmap;
class Texture;
class PackageFile;
class OnlineSubscribedMod;

#define MAP_FILE_SIGNATURE			"ZGMAP2"

class MapFile
{
	MEMORY_ALLOCATOR(MapFile, "Scene");

private:	
	std::vector<MapFileBlock*> m_blocks;
	MapVersion::Type m_version;

	bool m_internal;
	bool m_mod;

	MapFileHeaderBlock* m_header_block;
	Pixelmap* m_preview_pixelmap;
	Texture* m_preview_texture;

	PackageFile* m_package;
	OnlineSubscribedMod* m_online_mod;

protected:
	friend class MapFileFactory;

	static MapFile* Load(const char* url);
	static MapFile* Load(Stream* stream);

public:
	MapFile();
	~MapFile();


	void Set_Package(PackageFile* package)
	{
		m_package = package;
	}
	PackageFile* Get_Package()
	{
		return m_package;
	}
	void Set_Online_Mod(OnlineSubscribedMod* package)
	{
		m_online_mod = package;
	}
	OnlineSubscribedMod* Get_Online_Mod()
	{
		return m_online_mod;
	}

	bool Load_Internal(Stream* stream);
	bool Save(const char* url);
	bool Save(Stream* stream);

	void Set_Internal(bool value);
	bool Is_Internal();

	void Set_Mod(bool value);
	bool Is_Mod();

	Pixelmap* Get_Preview_Pixmap();
	Texture* Get_Preview_Image();

	MapFileHeaderBlock* Get_Header();

	template <typename T>
	T* Get_Block()
	{
		for (std::vector<MapFileBlock*>::iterator iter = m_blocks.begin(); iter != m_blocks.end(); iter++)
		{
			T* block = dynamic_cast<T*>(*iter);
			if (block != NULL)
			{
				return block;
			}
		}

		return NULL;
	}

	template <typename T>
	std::vector<T*> Get_Blocks()
	{
		std::vector<T*> blocks;
	
		for (std::vector<MapFileBlock*>::iterator iter = m_blocks.begin(); iter != m_blocks.end(); iter++)
		{
			T* block = dynamic_cast<T*>(*iter);
			if (block != NULL)
			{
				blocks.push_back(block);
			}
		}

		return blocks;
	}
	
	template <typename T>
	T* New_Block()
	{
		T* block = new T();
		m_blocks.push_back(block);
		return block;
	}

	MapFileBlock* New_Block(u32 id);
};

#endif

