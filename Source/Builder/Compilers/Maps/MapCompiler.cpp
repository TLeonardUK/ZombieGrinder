// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Maps/MapCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "libsquish/squish.h"

#include "Engine/Platform/Platform.h"

int MapCompiler::Get_Version()
{
	return 2;
}

std::vector<AssetToCompile> MapCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("maps"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("maps/map");

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

		// Check for preview file as well.
		std::string preview_file = Platform::Get()->Change_Filename(asset.Source_Path, "preview.png");
		std::string resolved_preview_file = builder->Resolve_Asset_Path(preview_file);
		if (Platform::Get()->Is_File(resolved_preview_file.c_str()))
		{
			asset.Dependent_Files.push_back(preview_file);
		}

		result.push_back(asset);
	}

	return result;
}

bool MapCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile asset)
{
	/*
	// Don't do this, preview images may be arbitraily sized, dosen't work well with DXT compression.

	// Compress preview images to DXT5.
	for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
	{
		std::string package_location = *iter;
		std::string resolved_asset_path = builder->Resolve_Asset_Path(package_location);
		std::string compiled_asset_path = builder->Get_Compiled_Asset_File(package_location);

		// Save compiled binary output.
		PackageFile* file = builder->Get_Package_File();
		PatchedBinaryStream* stream = new PatchedBinaryStream(compiled_asset_path.c_str(), StreamMode::Write);

		Pixelmap* pixmap = PixelmapFactory::Load(resolved_asset_path.c_str());
		DBG_ASSERT_STR(pixmap != NULL, "Failed to load pixmap '%s'.", resolved_asset_path.c_str());

		// CompiledPixmap
		stream->Write<u32>(pixmap->Get_Width());
		stream->Write<u32>(pixmap->Get_Height());
		stream->Write<u32>(PixelmapFormat::DXT5);
		int data_ptr = stream->Create_Pointer();
		stream->Align_To_Pointer();

		// Compile and write DXT5 texture.
		DBG_LOG("Compressing pixmap '%s' to DXT5.", resolved_asset_path.c_str());

		u8* data = pixmap->Get_Data();
		int output_size = squish::GetStorageRequirements(pixmap->Get_Width(), pixmap->Get_Height(), squish::kDxt5);
		DBG_ASSERT(output_size == pixmap->Get_Width() * pixmap->Get_Height());

		char* output = new char[output_size];

		squish::CompressImage(data, pixmap->Get_Width(), pixmap->Get_Height(), output, squish::kDxt5);
		stream->Patch_Pointer(data_ptr);
		stream->WriteBuffer(output, 0, output_size);

		SAFE_DELETE(output);

		stream->Close();
		SAFE_DELETE(stream);
	}
	*/

	return true;
}

void MapCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	// Add raw map file.
	std::string package_location = asset.Source_Path;
	std::string source_location = builder->Resolve_Asset_Path(package_location);
	builder->Get_Package_File()->Add_Chunk(source_location.c_str(), package_location.c_str(), PackageFileChunkType::Map, false, asset.Priority.c_str(), asset.Out_Of_Date);

	// Add preview images 
	for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
	{
		//std::string package_location = *iter;
		//std::string resolved_asset_path = builder->Resolve_Asset_Path(package_location);
		//std::string compiled_asset_path = builder->Get_Compiled_Asset_File(package_location);
		std::string package_location = *iter;
		std::string source_location = builder->Resolve_Asset_Path(package_location);

		builder->Get_Package_File()->Add_Chunk(source_location.c_str(), package_location.c_str(), PackageFileChunkType::Raw, false, asset.Priority.c_str(), asset.Out_Of_Date);
	}
}

