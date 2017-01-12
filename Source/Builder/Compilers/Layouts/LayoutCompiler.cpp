// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Layouts/LayoutCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

int LayoutCompiler::Get_Version()
{
	return 1;
}

std::vector<AssetToCompile> LayoutCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("layouts"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("layouts/layout");

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

bool LayoutCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile compile)
{
	// TODO: Binary format?
	return true;
}

void LayoutCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	std::string package_location = asset.Source_Path;
	std::string source_location = builder->Resolve_Asset_Path(package_location);

	builder->Get_Package_File()->Add_Chunk(source_location.c_str(), package_location.c_str(), PackageFileChunkType::Layout, false, asset.Priority.c_str(), asset.Out_Of_Date);
}