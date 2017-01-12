// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Options/OptionRegistry.h"

#include "Engine/IO/BinaryStream.h"

#include "Generic/Data/Hashes/CRC32DataTransformer.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/Platform/Platform.h"

#define OPTION_REGISTRY_MAGIC_SIGNATURE 0xCAADBEEF
#define OPTION_REGISTRY_VERSION			1

OptionRegistryBuffer::OptionRegistryBuffer(const char* data, int data_size)
{
	m_data = new char[data_size];
	memcpy(m_data, data, data_size);
	m_size = data_size;
}

OptionRegistryBuffer::OptionRegistryBuffer()
	: m_data(NULL)
	, m_size(0)
{
}

OptionRegistryBuffer::~OptionRegistryBuffer()
{
	SAFE_DELETE_ARRAY(m_data);
}

const char* OptionRegistryBuffer::Data()
{
	return m_data;
}

int OptionRegistryBuffer::Size()
{
	return m_size;
}

void OptionRegistryBuffer::Set_Buffer(const char* data, int data_size)
{
	SAFE_DELETE_ARRAY(m_data);

	m_data = new char[data_size];
	memcpy(m_data, data, data_size);
	m_size = data_size;
}

OptionRegistry::OptionRegistry()
	: m_state_stack_index(0)
{
	memset(m_state_stack, 0, sizeof(OptionRegistryBuffer) * MAX_OPTION_REGISTRY_STATE_STACK);
}

void OptionRegistry::Register(Option* option)
{
	m_options.push_back(option);
	m_options_map.Set(option->Get_Name_Hash(), option);
}

Option* OptionRegistry::Find(unsigned int name)
{
	Option* val = NULL;
	m_options_map.Get(name, val);
	return val;
}

Option* OptionRegistry::Find(const char* name)
{
	Option* val = NULL;
	unsigned int hash = StringHelper::Hash(name);
	m_options_map.Get(hash, val);
	return val;

	/*
	for (std::vector<Option*>::iterator iter = m_options.begin(); iter != m_options.end(); iter++)
	{
		Option* opt = *iter;
		if (stricmp(opt->Get_Name(), name) == 0 )
		{
			return opt;
		}
	}

	return NULL;
	*/
}

void OptionRegistry::Push_State()
{
	DBG_ASSERT_STR(m_state_stack_index < MAX_OPTION_REGISTRY_STATE_STACK, "Attempt to push option registry state onto full stack.");

	if (!Serialize(&m_state_stack[m_state_stack_index]))
	{
		DBG_ASSERT_STR(false, "Failed to serialize option registry.");
	}

	m_state_stack_index++;
}

void OptionRegistry::Pop_State(bool apply)
{
	DBG_ASSERT_STR(m_state_stack_index > 0, "Attempt to pop option registry state from empty stack.");

	m_state_stack_index--;
	
	if (apply == true)
	{
		if (!Deserialize(&m_state_stack[m_state_stack_index]))
		{
			DBG_ASSERT_STR(false, "Failed to deserialize option registry.");
		}
	}
}

bool OptionRegistry::Serialize(OptionRegistryBuffer* buffer)
{
	BinaryStream stream;
	stream.Write<unsigned int>(OPTION_REGISTRY_MAGIC_SIGNATURE);
	stream.Write<unsigned int>(OPTION_REGISTRY_VERSION);

	unsigned int header_patch_offset = stream.Position();
	stream.Write<unsigned int>(0); // Total options, patched later.
	stream.Write<unsigned int>(0); // Checksum, patched later.

	// Serialize all options.
	int options_saved = 0;
	for (std::vector<Option*>::iterator iter = m_options.begin(); iter != m_options.end(); iter++)
	{
		Option* opt = *iter;

		if (!opt->Flag_Set(OptionFlags::Saved))
		{
			continue;
		}

		stream.Write<unsigned int>(opt->Get_Name_Hash());
		stream.Write<unsigned int>(opt->Get_Type_ID());

		unsigned int length_offset = stream.Position();
		stream.Write<unsigned int>(0); // We patch this up below.

		if (!opt->Serialize(stream))
		{
			DBG_LOG("Failed to serialize option '%s' in option registry.", opt->Get_Name());
			return false;
		}

		int end_offset = stream.Position();

		stream.Seek(length_offset);
		stream.Write<unsigned int>(end_offset);

		stream.Seek(end_offset);

		options_saved++;
	}

	// Checksum all the data.
	CRC32DataTransformer crc;
	unsigned int checksum = crc.Calculate<unsigned int>((void*)stream.Data(), stream.Length());

	// Patch the header.
	stream.Seek(header_patch_offset);
	stream.Write<unsigned int>(options_saved);
	stream.Write<unsigned int>(checksum);

	buffer->Set_Buffer(stream.Data(), (int)stream.Length());

	return true;
}

bool OptionRegistry::Deserialize(OptionRegistryBuffer* buffer)
{
	BinaryStream stream(buffer->Data(), buffer->Size());

	if (buffer->Size() < sizeof(unsigned int) * 4)
	{
		DBG_LOG("Failed to deserialize option registry, stream to small.");
		return false;
	}

	unsigned int signature = stream.Read<unsigned int>();

	if (signature != OPTION_REGISTRY_MAGIC_SIGNATURE)
	{
		DBG_LOG("Failed to deserialize option registry, magic number was incorrect, recieved 0x%08x, expected 0x%08x.", signature, OPTION_REGISTRY_MAGIC_SIGNATURE);
		return false;
	}

	unsigned int version				= stream.Read<unsigned int>();
	unsigned int patched_header_offset  = stream.Position();
	unsigned int count					= stream.Read<unsigned int>();
	unsigned int checksum				= stream.Read<unsigned int>();

	// Validate checksum.
	{
		CRC32DataTransformer crc;
		
		// We need to blank out patched header to get original checksum data.
		stream.Seek(patched_header_offset);
		stream.Write<unsigned int>(0);
		stream.Write<unsigned int>(0);
		
		unsigned int real_checksum = crc.Calculate<unsigned int>((void*)stream.Data(), stream.Length());
		if (checksum != real_checksum)
		{
			DBG_LOG("Failed to deserialize option registry, invalid checksum, expected 0x%08x, recieved 0x%08x.", checksum, real_checksum);
			return false;
		}
	}

	for (unsigned int i = 0; i < count; i++)
	{
		unsigned int name_hash	= stream.Read<unsigned int>();
		unsigned int type_id	= stream.Read<unsigned int>();
		unsigned int end_offset = stream.Read<unsigned int>();

		Option* option = Find(name_hash);
		if (option == NULL)
		{
			DBG_LOG("Option 0x%08x no longer exists skipping.", name_hash);
			stream.Seek(end_offset);
		}
		else
		{
			if (type_id != option->Get_Type_ID())
			{
				DBG_LOG("Option 0x%08x no longer has same type id, skipping.", name_hash);
				stream.Seek(end_offset);
				continue;
			}

			if (!option->Flag_Set(OptionFlags::Saved))
			{
				DBG_LOG("Option 0x%08x is not marked as saved anymore, skipping.", name_hash);
				stream.Seek(end_offset);
				continue;
			}

			if (option->Flag_Set(OptionFlags::SetByCommandLine))
			{
				DBG_LOG("Option 0x%08x was already set by command line, skipping.", name_hash);
				stream.Seek(end_offset);
				continue;
			}

			DBG_LOG("\tOption[%i] '%s' = '%s'", i, option->Get_Name(), option->To_String().c_str());
			option->Deserialize(stream, version);

			if (stream.Position() != end_offset)
			{
				// We could skip this option here, but if we have not consumed the entire data correctly
				// chances are something is wrong with the data stream and its not safe to continue.

				DBG_LOG("Failed to deserialize option registry, option 0x%08x did not consume entire data.", name_hash);
				return false;
			}
		}
	}
	
	return true;
}
