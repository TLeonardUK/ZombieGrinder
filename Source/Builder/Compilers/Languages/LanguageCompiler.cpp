// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Languages/LanguageCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

#include "Engine/Resources/Compiled/Languages/CompiledLanguage.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include "Generic/Types/HashTable.h"

struct StringEntry
{
	std::string ID;
	std::string Value;

	int NamePtrIndex;
	int ValuePtrIndex;
};

int LanguageCompiler::Get_Version()
{
	return 3;
}

std::vector<AssetToCompile> LanguageCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("languages"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("languages/language");

	// Load atlases.
	for (std::vector<ConfigFileNode>::iterator atlas_iter = atlases.begin(); atlas_iter != atlases.end(); atlas_iter++)
	{
		ConfigFileNode& node = *atlas_iter;

		AssetToCompile asset;
		asset.Source_Path = file->Get<const char*>("file", node, true);
		asset.Priority = "";

		if (file->Contains("priority", node, true))
		{
			asset.Priority = file->Get<const char*>("priority", node, true);
		}

		result.push_back(asset);
	}

	return result;
}

bool LanguageCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile asset)
{	
	DBG_LOG("Compiling language resource '%s'.", asset.Source_Path.c_str());

	// Calculate all dependencies.
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);

	// Load asset file.
	ConfigFile asset_file;
	asset_file.Set_Root_Element("resources");
	if (!asset_file.Load(resolved_asset_path.c_str()))
	{
		return false;
	}

	// Add each of the source-images.
	std::vector<StringEntry> strings;

	int short_name_index = -1;
	int long_name_index = -1;
	int glyphs_index = -1;

	std::vector<ConfigFileNode> nodes = asset_file.Get<std::vector<ConfigFileNode> >("string");
	int index = 0;
	for (std::vector<ConfigFileNode>::iterator iter = nodes.begin(); iter != nodes.end(); iter++, index++)
	{
		ConfigFileNode node = *iter;	

		StringEntry entry;
		entry.ID	= StringHelper::Trim(asset_file.Get<const char*>("name", node, true));	
		entry.Value = node->value();

		entry.Value = StringHelper::Replace(entry.Value.c_str(), "\\n", "\n");
		entry.Value = StringHelper::Replace(entry.Value.c_str(), "\\\"", "\"");
		entry.Value = StringHelper::Replace(entry.Value.c_str(), "\\'", "'");

		if (entry.ID == "name_short")
			short_name_index = index;
		if (entry.ID == "name_long")
			long_name_index = index;
		if (entry.ID == "glyphs")
			glyphs_index = index;

		for (std::vector<StringEntry>::iterator iter2 = strings.begin(); iter2 != strings.end(); iter2++)
		{
			StringEntry& other = *iter2;
			if (other.ID == entry.ID)
			{
				DBG_LOG("Language file contains duplicate string id '%s'.", entry.ID.c_str());
				return false;
			}
		}

		strings.push_back(entry);
	}

	// Check required strings are here.
	if (short_name_index == -1 ||
		long_name_index == -1 ||
		glyphs_index == -1)
	{
		DBG_LOG("Language file does not contain a required string (name_short/name_long/glyphs).");
		return false;
	}

	// Save compiled binary output.
	PackageFile* file = builder->Get_Package_File();
	PatchedBinaryStream* stream = new PatchedBinaryStream(compiled_asset_path.c_str(), StreamMode::Write);

	std::vector<int> string_name_pointers;

	// CompiledLanguageHeader
	stream->Write<u32>(strings.size());
	stream->Write<u32>(short_name_index);
	stream->Write<u32>(long_name_index);
	stream->Write<u32>(glyphs_index);
	int strings_ptr_index = stream->Create_Pointer();
	stream->Align_To_Pointer();

	// Write string entries.
	stream->Patch_Pointer(strings_ptr_index);
	for (unsigned int i = 0; i < strings.size(); i++)
	{
		// CompiledLanguageString
		StringEntry& string = strings.at(i);
		stream->Write<u32>(StringHelper::Hash(string.ID.c_str()));
		string.NamePtrIndex = stream->Create_Pointer();
		string.ValuePtrIndex = stream->Create_Pointer();
		stream->Align_To_Pointer();
	}
	for (unsigned int i = 0; i < strings.size(); i++)
	{
		StringEntry& string = strings.at(i);
		stream->Patch_Pointer(string.NamePtrIndex);
		stream->WriteNullTerminatedString(string.ID.c_str());
		stream->Patch_Pointer(string.ValuePtrIndex);
		stream->WriteNullTerminatedString(string.Value.c_str());
	}

	stream->Close();
	SAFE_DELETE(stream);
	
	//stream = new PatchedBinaryStream(compiled_asset_path.c_str(), StreamMode::Read);
	//CompiledLanguageHeader* header = reinterpret_cast<CompiledLanguageHeader*>(stream->Get_Data());
	//stream->Close();
	//SAFE_DELETE(stream);

	return true;
}

void LanguageCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);

	builder->Get_Package_File()->Add_Chunk(compiled_asset_path.c_str(), asset.Source_Path.c_str(), PackageFileChunkType::Language, false, asset.Priority.c_str(), asset.Out_Of_Date);
}