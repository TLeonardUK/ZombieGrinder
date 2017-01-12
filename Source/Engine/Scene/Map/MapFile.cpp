// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Map/MapFileFactory.h"
#include "Engine/Scene/Map/MapFileHandle.h"
#include "Engine/Scene/Map/MapFile.h"

#include "Engine/Scene/Map/Blocks/MapFileBlock.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIElement.h"

#include "Engine/IO/StreamFactory.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Config/ConfigFile.h"

#include "Engine/Renderer/Textures/TextureFactory.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/Scene/Map/Blocks/MapFilePreviewBlock.h"

#include "Generic/Data/Hashes/CRC32DataTransformer.h"

#include "Generic/Helper/StringHelper.h"

#include <typeinfo>

MapFile::MapFile()
	: m_header_block(NULL)
	, m_version(MapVersion::Current_Version)
	, m_internal(false)
	, m_preview_texture(NULL)
	, m_preview_pixelmap(NULL)
	, m_mod(false)
	, m_online_mod(NULL)
	, m_package(NULL)
{
}

MapFile::~MapFile()
{
	if (m_preview_pixelmap != NULL)
	{
		SAFE_DELETE(m_preview_pixelmap);
	}
	if (m_preview_texture != NULL)
	{
		SAFE_DELETE(m_preview_texture);
	}
	for (std::vector<MapFileBlock*>::iterator iter = m_blocks.begin(); iter != m_blocks.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_blocks.clear();
}

void MapFile::Set_Internal(bool value)
{
	m_internal = value;
}

bool MapFile::Is_Internal()
{
	return m_internal;
}

void MapFile::Set_Mod(bool value)
{
	m_mod = value;
}

bool MapFile::Is_Mod()
{
	return m_mod;
}

Pixelmap* MapFile::Get_Preview_Pixmap()
{
	return m_preview_pixelmap;
}

Texture* MapFile::Get_Preview_Image()
{
	return m_preview_texture;
}

bool MapFile::Load_Internal(Stream* stream)
{
	// Clear old data.
	m_blocks.clear();

	// Check length for obvious issues.
	int siglen = strlen(MAP_FILE_SIGNATURE);
	if ((int)stream->Length() < siglen)
	{
		DBG_LOG("Failed to load map, map file has an invalid header.");
		return false;
	}

	// Check signature.
	char* signature = new char[siglen + 1];
	stream->ReadBuffer(signature, 0, siglen);
	signature[siglen] = '\0';

	if (strcmp(signature, MAP_FILE_SIGNATURE) != 0)
	{
		SAFE_DELETE_ARRAY(signature);
		DBG_LOG("Failed to load map, map file has an invalid header.");
		return false;
	}

	SAFE_DELETE_ARRAY(signature);

	// Check version.
	m_version = (MapVersion::Type)stream->Read<u32>();
	DBG_LOG("Map file is version %i.", (int)m_version);

	// Check the expected checksum.
	u32 checksum = stream->Read<u32>();
	u32 checksum_position = stream->Position();

	// Calculate the current checksum.
	int crc_bytes = stream->Bytes_Remaining();
	char* buffer = new char[crc_bytes];
	stream->ReadBuffer(buffer, 0, crc_bytes);

	CRC32DataTransformer crc_transformer;
	u32 real_checksum = crc_transformer.Calculate<u32>((void*)buffer, crc_bytes);

	/*if (real_checksum != checksum)
	{
		DBG_LOG("Failed to load map, map file has an invalid checksum (0x%08x expected 0x%08x).", real_checksum, checkum);
		SAFE_DELETE_ARRAY(buffer);
		return false;
	}*/

	stream->Seek(checksum_position);
	SAFE_DELETE_ARRAY(buffer);

	//DBG_LOG("==== READING BLOCKS ====");

	// Start reading blocks.
	while (stream->Bytes_Remaining() > 0)
	{
		// Read block header.
		u32 id = stream->Read<u32>();
		u32 length = stream->Read<u32>();

		//DBG_LOG("Block 0x%08x %i", id, length);

		// SHIT
		/*
		if (length > 1024 * 1024 * 3)
		{
			DBG_LOG("Map file looks corrupt. Shit.");
			break;
		}
		*/

		unsigned int offset = (unsigned int)stream->Position();
		unsigned int total_length = (unsigned int)stream->Length();

		//DBG_LOG("Block %c%c%c%c (length=%u, offset=%u/%u)", id & 0xFF, (id >> 8) & 0xFF, (id >> 16) & 0xFF, (id >> 24) & 0xFF, length, offset, total_length);

		// Read block data.
		char* data = new char[length];
		stream->ReadBuffer(data, 0, length);

		BinaryStream blockstream(data, length);

		MapFileBlock* block = New_Block(id);
		if (block != NULL)
		{
			bool ret = block->Decode(m_version, &blockstream);
			if (!ret)
			{
				DBG_LOG("Failed to load map, could not decode block 0x%08x.", id);
				return false;
			}
		}
		else
		{
			DBG_LOG("File contains unknown block %c%c%c%c (length=%u, offset=%u/%u). Possibly from a new file format version?", id & 0xFF, (id >> 8) & 0xFF, (id >> 16) & 0xFF, (id >> 24) & 0xFF, length, offset, total_length);
		}

		SAFE_DELETE_ARRAY(data);
	}

	// Ensure we have a header chunk.
	m_header_block = Get_Block<MapFileHeaderBlock>();
	if (m_header_block == NULL)
	{
		DBG_LOG("Failed to load map, map file does not appear to have a header chunk.");
		return false;
	}

	// Grab the preview image
	MapFilePreviewBlock* preview_block = Get_Block<MapFilePreviewBlock>();
	if (preview_block != NULL && preview_block->Preview != NULL)
	{
		m_preview_pixelmap = preview_block->Preview;
		m_preview_texture = Renderer::Get()->Create_Texture(m_preview_pixelmap, TextureFlags::PersistSourceData);
	}
	else
	{
		m_preview_pixelmap = NULL;
		m_preview_texture = NULL;
	}

	return true;
}

MapFile* MapFile::Load(Stream* stream)
{
	// Create the layout.
	MapFile* map = new MapFile();
	if (map->Load_Internal(stream))
	{
		return map;
	}

	SAFE_DELETE(map);
	return NULL;
}

MapFile* MapFile::Load(const char* url)
{
	Platform* platform = Platform::Get();

	DBG_LOG("Loading map resource '%s'.", url);
	
	// Open dat stream.
	Stream* stream = NULL;
	if (ResourceFactory::Try_Get() != NULL)
	{
		stream = ResourceFactory::Get()->Open(url, StreamMode::Read);
	}
	if (stream == NULL)
	{
		stream = StreamFactory::Open(url, StreamMode::Read);
	}

	if (stream == NULL)
	{
		DBG_LOG("Failed to load map, map file could not be opened: '%s'", url);
		return NULL;
	}	

	MapFile* file = Load(stream);

	SAFE_DELETE(stream);
	return file;
}

bool MapFile::Save(Stream* stream)
{
	// Check signature.
	stream->WriteBuffer(MAP_FILE_SIGNATURE, 0, strlen(MAP_FILE_SIGNATURE));
	stream->Write<u32>((u32)MapVersion::Current_Version);

	// Save all blocks into a memory stream so we can calc checksum.
	BinaryStream mem_stream; 

	for (std::vector<MapFileBlock*>::iterator iter = m_blocks.begin(); iter != m_blocks.end(); iter++)
	{
		MapFileBlock* block = *iter;

		BinaryStream block_stream;

		bool ret = block->Encode(MapVersion::Current_Version, &block_stream);
		if (ret == false)
		{
			DBG_LOG("Failed to save map, block 0x%08x failed to encode.", block->Get_ID());
			SAFE_DELETE(stream);
			return false;
		}

		mem_stream.Write<u32>(block->Get_ID());
		mem_stream.Write<u32>(block_stream.Length());
		mem_stream.WriteBuffer(block_stream.Data(), 0, block_stream.Length());
	}

	// Write out checksum	
	CRC32DataTransformer crc_transformer;
	u32 real_checksum = crc_transformer.Calculate<u32>((void*)mem_stream.Data(), mem_stream.Length());
	stream->Write<u32>(real_checksum);

	// Write block data into main stream.
	stream->WriteBuffer(mem_stream.Data(), 0, mem_stream.Length());

	return true;
}

bool MapFile::Save(const char* url)
{
	Platform* platform = Platform::Get();

	DBG_LOG("Saving map resource '%s'.", url);
	
	// Open dat stream.
	Stream* stream = StreamFactory::Open(url, (StreamMode::Type)(StreamMode::Write | StreamMode::Truncate));
	if (stream == NULL)
	{
		DBG_LOG("Failed to save map, map file could not be opened: '%s'", url);
		return false;
	}

	Save(stream);

	// Done!
	DBG_LOG("Finished saving map: '%s'", url);

	stream->Flush();
	stream->Close();
	SAFE_DELETE(stream);

	return true;
}

MapFileHeaderBlock* MapFile::Get_Header()
{
	return m_header_block;
}

MapFileBlock* MapFile::New_Block(u32 id)
{
	MapFileBlock* block = MapFileBlockFactoryBase::New_Block(id);
	if (block != NULL)
	{
		m_blocks.push_back(block);
	}
	return block;
}
