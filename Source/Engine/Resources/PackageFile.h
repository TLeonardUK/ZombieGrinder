// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_PACKAGEFILE_
#define _ENGINE_RESOURCES_PACKAGEFILE_

#include <vector>
#include <string>

#include "Generic/Types/HashTable.h"

#include "Engine/Crypto/CryptoStream.h"

class PackageFileStream;
class Mutex;
class Stream;

struct PackageFileChunkType
{
	enum Type
	{
		Atlas,
		Sound,
		Font,
		Language,
		Layout,
		Map,
		Raw,
		Script,
		Shader,
		Source,
		ParticleFX,
		Video
	};
};

struct PackageFileChunk
{
public:
	PackageFileChunkType::Type type;

	unsigned int name_offset;
	unsigned int offset;
	unsigned int checksum;
	unsigned int priority_offset;
	unsigned int compressed;
	unsigned int compressed_size;
	unsigned int uncompressed_size;
};

struct PackageFileString
{
public:
	unsigned int hash;
	// Null-termianted string follows this struct.
};

struct PackageFileHeader
{
public:
	enum
	{
		MAGIC_NUMBER = 0xAB98CD89,
		CURRENT_VERSION = 1,
		SIGNATURE_LENGTH = 128,
		MAX_GUID_LENGTH = 64,
		MAX_WORKSHOP_NAME = 256,
	};
	 
	unsigned int magic;
	unsigned int version;
	char signature[SIGNATURE_LENGTH];
	char guid[MAX_GUID_LENGTH];
	u64  workshop_id;
	char workshop_name[MAX_WORKSHOP_NAME];

	unsigned int chunk_count;
	unsigned int chunk_strings_size;
	unsigned int chunk_data_size;

	char* chunk_strings;				// Variable length structures of PackageFileString where *name is a pointer to itself.
	PackageFileChunk* chunks;

	unsigned int chunk_data_offset;
};

struct PackageFileBaseStream
{
public:
	Stream* base_stream;
	bool	in_use;
};

class PackageFile
{
	MEMORY_ALLOCATOR(PackageFile, "Engine");

private:
	friend class PackageFileStream;

	PackageFileHeader m_header;
	std::vector<std::string> m_chunk_source_files;

	Mutex* m_package_streams_mutex;
	std::vector<PackageFileBaseStream*> m_package_streams;

	std::string m_package_path;

	bool m_use_compression;
	std::string m_tmp_dir;

	HashTable<PackageFileChunk*, unsigned int> m_chunk_hash_table;

	bool m_is_signed;
	bool m_is_whitelisted;

	CryptoKey m_public_key;
	CryptoKey m_private_key;

	enum
	{
		max_package_streams = 16,
		compression_chunk_size = 256 * 1024
	};
	
	void Read_Header();
	unsigned int Add_String(const char* value);
	unsigned int Get_File_Checksum(const char* url);
	void Add_File_Data(const char* source, unsigned int& offset, unsigned int& uncompressed_size, unsigned int& compressed_size, bool should_compress, bool has_changed);
	std::string Normalize_Chunk_Path(const char* path);

public:

	// Used to set package priorities when loading assets.
	int Priority;

	// Constructors
	PackageFile();	
	virtual ~PackageFile();	

	// Access operations.
	void Open(const char* path);
	void Close();

	PackageFileStream* Open_Chunk_Stream(PackageFileChunk* chunk);
	PackageFileStream* Open_Chunk_Stream(const char* url);

	// Signing.
	void Set_Keys(CryptoKey private_key, CryptoKey public_key);
	bool Is_Signed();
	PackageFileHeader* Get_Header();

	// Whitelisting.
	bool Is_Whitelisted();
	void Whitelist();

	// General.
	std::string Get_Name();

	// Chunk iteration.
	int Get_Chunk_Count();
	PackageFileChunk* Get_Chunk(int index);
	const char* Get_String(int offset);
	unsigned int Get_String_Hash(int offset);

	// Creations operations.
	void Set_Tmp_Folder(const char* url);
	void Use_Compression(bool value);
	void Add_Chunk(const char* source_file, const char* package_location, PackageFileChunkType::Type type, bool force_uncompressed, const char* priority, bool has_changed);
	void Write_Patched_Header(const char* path);
	void Write_To_Disk(const char* path);
	void Write_Manifest_To_Disk(const char* path);

};

#endif

