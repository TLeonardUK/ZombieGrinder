// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Video/VideoCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "libsquish/squish.h"

#include "Engine/Platform/Platform.h"

int VideoCompiler::Get_Version()
{
	return 1;
}

std::vector<AssetToCompile> VideoCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("videos"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases = file->Get<std::vector<ConfigFileNode> >("videos/video");

	// Add each video and associated files.
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

		// Check for subtitle file as well.
		std::string subtitle_file = Platform::Get()->Change_Extension(asset.Source_Path, ".srt");
		std::string resolved_subtitle_file = builder->Resolve_Asset_Path(subtitle_file);
		if (Platform::Get()->Is_File(resolved_subtitle_file.c_str()))
		{
			asset.Dependent_Files.push_back(subtitle_file);
		}

		result.push_back(asset);
	}

	return result;
}

bool VideoCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile asset)
{
	return true;
}

void VideoCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	// Add raw video files.
	std::string package_location = asset.Source_Path;
	std::string source_location = builder->Resolve_Asset_Path(package_location);
	builder->Get_Package_File()->Add_Chunk(source_location.c_str(), package_location.c_str(), PackageFileChunkType::Video, false, asset.Priority.c_str(), asset.Out_Of_Date);

	// Add subtitle files. 
	for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
	{
		std::string package_location = *iter;
		std::string source_location = builder->Resolve_Asset_Path(package_location);

		builder->Get_Package_File()->Add_Chunk(source_location.c_str(), package_location.c_str(), PackageFileChunkType::Raw, false, asset.Priority.c_str(), asset.Out_Of_Date);
	}
}

