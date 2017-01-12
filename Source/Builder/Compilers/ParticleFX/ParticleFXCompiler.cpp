// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/ParticleFX/ParticleFXCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

int ParticleFXCompiler::Get_Version()
{
	return 1;
}

std::vector<AssetToCompile> ParticleFXCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("particlefx"))
	{
		return result;
	}

	std::vector<ConfigFileNode> particles	= file->Get<std::vector<ConfigFileNode> >("particlefx/fx");

	// Load particles.
	for (std::vector<ConfigFileNode>::iterator atlas_iter = particles.begin(); atlas_iter != particles.end(); atlas_iter++)
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

bool ParticleFXCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile compile)
{
	// TODO: Binary format?
	return true;
}

void ParticleFXCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	std::string package_location = asset.Source_Path;
	std::string source_location = builder->Resolve_Asset_Path(package_location);

	builder->Get_Package_File()->Add_Chunk(source_location.c_str(), package_location.c_str(), PackageFileChunkType::ParticleFX, false, asset.Priority.c_str(), asset.Out_Of_Date);
}