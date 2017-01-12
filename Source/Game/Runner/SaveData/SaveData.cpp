// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Runner/SaveData/SaveData.h"

#include "Game/Game/TutorialManager.h"

#include "Game/Online/AchievementManager.h"
#include "Game/Online/ChallengeManager.h"
#include "Game/Online/RankingsManager.h"
#include "Game/Online/StatisticsManager.h"
#include "Game/Online/DLCManager.h"
#include "Engine/Options/OptionRegistry.h"

#include "Game/Profile/ProfileManager.h"

#include "Generic/Types/DataBuffer.h"

#include "Engine/IO/BinaryStream.h"
#include "Generic/Data/Hashes/CRC32DataTransformer.h"

#include "Game/Version.h"

#define SAVE_DATA_MAGIC_SIGNATURE 0x1337C0DE

bool g_save_data_enabled = false;
DataBuffer g_save_data_buffer = DataBuffer();

bool SaveData::Serialize(DataBuffer& buffer)
{
	BinaryStream stream;
	stream.Write<unsigned int>(SAVE_DATA_MAGIC_SIGNATURE);
	stream.Write<unsigned int>(0xFFFFFFFF);
	stream.Write<unsigned int>(VersionInfo::TOTAL_CHANGELISTS);

	unsigned int header_patch_offset = stream.Position();
	stream.Write<unsigned int>(0); // Checksum, patched later.

	// Serialize the options registry.
	OptionRegistryBuffer opt_buffer;
	bool result = OptionRegistry::Get()->Serialize(&opt_buffer);	
	if (!result)
	{
		return false;
	}
	stream.Write<u32>(opt_buffer.Size());
	stream.WriteBuffer(opt_buffer.Data(), 0, opt_buffer.Size());

	// Write save data.
	if (g_save_data_enabled)
	{
		Serialize_SaveData(&stream);
	}
	else if (g_save_data_buffer.Size() > 0)
	{
		stream.WriteBuffer(g_save_data_buffer.Buffer(), 0, g_save_data_buffer.Size());
	}

	// Checksum all the data.
	CRC32DataTransformer crc;
	unsigned int checksum = crc.Calculate<unsigned int>((void*)stream.Data(), stream.Length());

	// Patch the header.
	stream.Seek(header_patch_offset);
	stream.Write<unsigned int>(checksum);

	buffer.Set(stream.Data(), (int)stream.Length());

	return true;
}

bool SaveData::Deserialize(DataBuffer& buffer)
{
	BinaryStream stream(buffer.Buffer(), buffer.Size());

	if (buffer.Size() < sizeof(unsigned int) * 4)
	{
		DBG_LOG("Failed to deserialize save data, stream to small.");
		return false;
	}

	unsigned int signature = stream.Read<unsigned int>();

	if (signature != SAVE_DATA_MAGIC_SIGNATURE)
	{
		DBG_LOG("Failed to deserialize save data, magic number was incorrect, recieved 0x%08x, expected 0x%08x.", signature, SAVE_DATA_MAGIC_SIGNATURE);
		return false;
	}

	bool bIgnoreVersionCheck = false;

	unsigned int version				= stream.Read<unsigned int>();
	if (version == 0xFFFFFFFF)
	{
		version = stream.Read<unsigned int>();
	}
	else
	{
		bIgnoreVersionCheck = false;
	}
	unsigned int patched_header_offset  = stream.Position();
	unsigned int checksum				= stream.Read<unsigned int>();

	unsigned int current_version		= (unsigned int)VersionInfo::TOTAL_CHANGELISTS;

	DBG_LOG("Save data, version=%u, current=%u", version, current_version);

#ifdef MASTER_BUILD
	if (version > current_version && !bIgnoreVersionCheck)
	{
		DBG_ASSERT_STR_STRAIGHT(false, "Failed to load save data, save data is for a newer version of the game.\n\nIf you have switched from a beta branch to the main branch, please run the game with the command line '-reset_save_data 1' to clear your save data, this will allow you to run on older branches.");
		return false;
	}
#endif

	// Validate checksum.
	{
		CRC32DataTransformer crc;
		
		// We need to blank out patched header to get original checksum data.
		stream.Seek(patched_header_offset);
		stream.Write<unsigned int>(0);
		
		unsigned int real_checksum = crc.Calculate<unsigned int>((void*)stream.Data(), stream.Length());
		if (checksum != real_checksum)
		{
			DBG_LOG("Failed to deserialize save data, invalid checksum, expected 0x%08x, recieved 0x%08x.", checksum, real_checksum);
			return false;
		}
	}

	// Read options registry.
	u32 opt_size = stream.Read<u32>();
	char* opt_data = new char[opt_size];
	stream.ReadBuffer(opt_data, 0, opt_size);

	OptionRegistryBuffer opt_buffer(opt_data, opt_size);
	bool result = OptionRegistry::Get()->Deserialize(&opt_buffer);
	if (!result)
	{
		DBG_LOG("Failed to deserialize save data. Options registry could not be deserialized.");
		SAFE_DELETE_ARRAY(opt_data);
		return false;
	}

	SAFE_DELETE_ARRAY(opt_data);

	if (g_save_data_enabled)
	{
		Deserialize_SaveData(&stream);
	}
	else
	{
		g_save_data_buffer.Reserve(stream.Bytes_Remaining(), false);
		stream.ReadBuffer(g_save_data_buffer.Buffer(), 0, g_save_data_buffer.Size());
	}

	return true;
}

bool SaveData::Serialize_SaveData(BinaryStream* stream)
{
	// Serialize all profiles.
	ProfileManager::Get()->Serialize(stream);
	
	// Serialize achievements.
	AchievementManager::Get()->Serialize(stream);
	
	// Serialize rankings.
	RankingsManager::Get()->Serialize(stream);

	// Serialize statistics.
	StatisticsManager::Get()->Serialize(stream);

	// Serialize dlc.
	DLCManager::Get()->Serialize(stream);

	// Serialize achievements.
	TutorialManager::Get()->Serialize(stream);

	// Serialize challenges.
	ChallengeManager::Get()->Serialize(stream);

	return true;
}

bool SaveData::Deserialize_SaveData(BinaryStream* stream)
{
#define CHECK_BYTES() \
	if (stream->Bytes_Remaining() <= 0) \
		return true; \

	// Read profiles.
	CHECK_BYTES();
	ProfileManager::Get()->Deserialize(stream);
	
	// Read achievements.
	CHECK_BYTES();
	AchievementManager::Get()->Deserialize(stream);
	
	// Read rankings.
	CHECK_BYTES();
	RankingsManager::Get()->Deserialize(stream);

	// Read statistics.
	CHECK_BYTES();
	StatisticsManager::Get()->Deserialize(stream);

	// Read dlc.
	CHECK_BYTES();
	DLCManager::Get()->Deserialize(stream);

	// Serialize achievements.
	CHECK_BYTES();
	TutorialManager::Get()->Deserialize(stream);

	// Serialize achievements.
	CHECK_BYTES();
	ChallengeManager::Get()->Deserialize(stream);

	return true;
}

void SaveData::Enable_Save_Data()
{
	g_save_data_enabled = true;
	if (g_save_data_buffer.Size() > 0)
	{
		BinaryStream stream(g_save_data_buffer.Buffer(), g_save_data_buffer.Size());
		Deserialize_SaveData(&stream);

		StatisticsManager::Get()->Sync();
	}
}