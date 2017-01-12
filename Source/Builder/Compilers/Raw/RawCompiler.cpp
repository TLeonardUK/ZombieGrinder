// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Raw/RawCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

int RawCompiler::Get_Version()
{
	return 1;
}

std::vector<AssetToCompile> RawCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("raw"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("raw/raw");

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

bool RawCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile compile)
{
	return true;
}

void RawCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	std::string package_location = asset.Source_Path;
	std::string source_location = builder->Resolve_Asset_Path(package_location);
	builder->Get_Package_File()->Add_Chunk(source_location.c_str(), package_location.c_str(), PackageFileChunkType::Raw, false, asset.Priority.c_str(), asset.Out_Of_Date);
}