// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Builder/AssetBuilder.h"
#include "Builder/Builder/AssetBuilderOptions.h"

#include "Builder/Compilers/Atlases/AtlasCompiler.h"
#include "Builder/Compilers/Audio/AudioCompiler.h"
#include "Builder/Compilers/Fonts/FontCompiler.h"
#include "Builder/Compilers/Languages/LanguageCompiler.h"
#include "Builder/Compilers/Layouts/LayoutCompiler.h"
#include "Builder/Compilers/Raw/RawCompiler.h"
#include "Builder/Compilers/Scripts/ScriptCompiler.h"
#include "Builder/Compilers/Shaders/ShaderCompiler.h"
#include "Builder/Compilers/Maps/MapCompiler.h"
#include "Builder/Compilers/ParticleFX/ParticleFXCompiler.h"
#include "Builder/Compilers/Video/VideoCompiler.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#endif

#include "Engine/IO/StreamFactory.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Resources/PackageFile.h"
#include "Engine/Resources/PackageFileStream.h"

#include "Engine/Options/OptionRegistry.h"
#include "Engine/Options/OptionCommandLineParser.h"

#include "Engine/Online/OnlineMods.h"
#include "Engine/Online/OnlinePlatform.h"

#include "XScript/Compiler/CTranslationUnit.h"

#include <stdio.h>
#include <typeinfo>

AssetBuilder::AssetBuilder()
	: m_assets_rebuilt(0)
	, m_build_errors_occured(false)
	, m_package_xml_dir("")
{
	m_compilers.push_back(new ScriptCompiler());
	m_compilers.push_back(new AtlasCompiler());
	m_compilers.push_back(new AudioCompiler());
	m_compilers.push_back(new FontCompiler());
	m_compilers.push_back(new LanguageCompiler());
	m_compilers.push_back(new LayoutCompiler());
	m_compilers.push_back(new ShaderCompiler());
	m_compilers.push_back(new RawCompiler());
	m_compilers.push_back(new MapCompiler());
	m_compilers.push_back(new ParticleFXCompiler());
	m_compilers.push_back(new VideoCompiler());
}

AssetBuilder::~AssetBuilder()
{
	for (std::vector<AssetCompiler*>::iterator iter = m_compilers.begin(); iter != m_compilers.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_compilers.clear();
}

std::string AssetBuilder::Get_Compile_Dir()
{
	return m_package_compile_dir;
}

ConfigFile* AssetBuilder::Get_Package_Config_File()
{
	return &m_package_config_file;
}

PackageFile* AssetBuilder::Get_Package_File()
{
	return &m_package_file;
}

std::string AssetBuilder::Get_Root_Dir()
{
	return m_root_directory;
}

std::string AssetBuilder::Get_Package_Root_Dir()
{
	return m_package_root;
}

std::string AssetBuilder::Get_Package_Name()
{
	return m_package_name;
}

std::string AssetBuilder::Get_Package_XML_Dir()
{
	return m_package_xml_dir;
}

bool AssetBuilder::Setup_Build_Environment()
{
	// Load configuration for the resource.
	if (!m_package_config_file.Load(m_package_xml_path.c_str()))
	{
		DBG_LOG("Failed to parse configuration file '%s'.", m_package_xml_path.c_str());
		return false;
	}

	// Setup task manager.
	m_task_manager = new TaskManager(Platform::Get()->Get_Core_Count() - 1, 4096);

	m_package_xml_dir = Platform::Get()->Extract_Directory(m_package_xml_path);

	// Create resource mapping.
	m_package_name			= m_package_config_file.Get<std::string>("package/name");
	m_package_root			= m_package_config_file.Get<std::string>("package/root");
	m_package_compile_dir	= Platform::Get()->Get_Absolute_Path(m_package_xml_dir + "/" + m_package_config_file.Get<std::string>("package/compile_directory"));
	m_pak_file				= Platform::Get()->Get_Absolute_Path(m_package_xml_dir + "/" + m_package_config_file.Get<std::string>("package/pak_file"));
	m_pak_keep_source		= m_package_config_file.Get<bool>("package/pak_keep_source");
	m_pak_use_compression	= m_package_config_file.Get<bool>("package/pak_use_compression");
	m_private_key_path		 = Platform::Get()->Get_Absolute_Path(m_package_xml_dir + "/" + m_package_config_file.Get<std::string>("package/private_key"));

	// Load in the private cryptokey.
	if (Platform::Get()->Is_File(m_private_key_path.c_str()))
	{
		m_private_key.Load(m_private_key_path.c_str());
	}

	m_package_file.Use_Compression(m_pak_use_compression);
	m_package_file.Set_Tmp_Folder((m_package_compile_dir + "/.compressed").c_str());

	DBG_LOG("Parsing resource package: %s", m_package_name.c_str());
	DBG_LOG("\tRoot: %s", m_package_root.c_str());
	DBG_LOG("\tCompile Directory: %s", m_package_compile_dir.c_str());

	// Create directory.
	Platform::Get()->Create_Directory(m_package_compile_dir.c_str(), true);
	Platform::Get()->Create_Directory((m_package_compile_dir + "/.compressed").c_str(), true);
	Platform::Get()->Create_Directory((m_package_compile_dir + "/.intermediate").c_str(), true);

	if (!Platform::Get()->Is_Directory(m_package_compile_dir.c_str()))
	{
		DBG_LOG("Failed to create compile directory: %s", m_package_compile_dir.c_str());
		return false;
	}

	return true;
}

u64 AssetBuilder::Calculate_File_Hash(u64 last_modified, int version)
{
	// Ultra-shitty 4 byte hash.
	u64 lower_bits = last_modified & 0x00FFFFFFFFFFFFFF;
	u64 higher_bits = ((u64)version) << 56;
	u64 hash = higher_bits | lower_bits;
	return hash;  
}

u64 AssetBuilder::Read_Hash(std::string path)
{
	Stream* stream = StreamFactory::Open((path + ".hash").c_str(), StreamMode::Read);
	if (stream == NULL)
	{
		return 0;
	}

	u64 version = stream->Read<u64>();

	stream->Close();
	SAFE_DELETE(stream);

	return version;
}

void AssetBuilder::Write_Hash(std::string path, u64 hash)
{
	Stream* stream = StreamFactory::Open((path + ".hash").c_str(), StreamMode::Write);
	if (stream == NULL)
	{
		return;
	}

	stream->Write<u64>(hash);
	stream->Close();

	SAFE_DELETE(stream);
}

std::vector<std::string> AssetBuilder::Read_Extra_Dependencies(std::string path)
{
	std::vector<std::string> result;

	Stream* stream = StreamFactory::Open((path + ".extra").c_str(), StreamMode::Read);
	if (stream != NULL)
	{
		while (stream->Bytes_Remaining() > 0)
		{
			result.push_back(stream->ReadNullTerminatedString());
		}

		stream->Close();
		SAFE_DELETE(stream);
	}

	return result;
}

void AssetBuilder::Write_Extra_Dependencies(AssetToCompile asset, std::vector<std::string> deps)
{
	std::string file = Get_Compiled_Asset_File(asset.Source_Path);

	Stream* stream = StreamFactory::Open((file + ".extra").c_str(), StreamMode::Write);
	if (stream == NULL)
	{
		return;
	}

	for (std::vector<std::string>::iterator iter = deps.begin(); iter != deps.end(); iter++)
	{
		std::string value = *iter;
		stream->WriteNullTerminatedString(value.c_str());
	
		std::string asset_path = Resolve_Asset_Path(value.c_str());
		std::string resolved = Get_Compiled_Asset_File(value.c_str());
		
		u64 last_modified = StreamFactory::Get_Last_Modified(asset_path.c_str());
		
		u64 hash = Calculate_File_Hash(last_modified, asset.Version);
		Write_Hash(resolved.c_str(), hash);
	}

	stream->Close();
	SAFE_DELETE(stream);
}

std::string AssetBuilder::Get_Compiled_Asset_File(std::string path)
{
	Platform* platform = Platform::Get();

//	std::string output_path			= m_package_compile_dir + path;
//	std::string output_directory	= platform->Extract_Directory(output_path);

	std::string output_path = StringHelper::Format("%s/.intermediate/%08x.data", m_package_compile_dir.c_str(), StringHelper::Hash(path.c_str()));

//	if (!platform->Is_Directory(output_directory.c_str()))
//		platform->Create_Directory(output_directory.c_str(), true);

	return output_path;
}

std::string AssetBuilder::Resolve_Asset_Path(std::string path)
{
	if (path.substr(0, m_package_root.size()) == m_package_root)
	{
		path = m_root_directory + "/" + path.substr(m_package_root.size());
	}
	return path;
}

bool AssetBuilder::Is_Asset_Out_Of_Date(AssetToCompile asset)
{
	Platform* plat = Platform::Get();

	if (m_rebuild)
	{
		return true;
	}

	// Is asset not compiled yet?
	std::string file = Get_Compiled_Asset_File(asset.Source_Path);
	std::string resolved_file = Resolve_Asset_Path(asset.Source_Path);
//	if (!Platform::Get()->Is_File(file.c_str()))
//	{
//		return true;
//	}

	// Is the main source file out of date.
	u64 hash = Calculate_File_Hash(StreamFactory::Get_Last_Modified(resolved_file.c_str()), asset.Version);
	u64 current_hash = Read_Hash(file.c_str());
	if (current_hash != hash)
	{
		//DBG_LOG("[%s] Out of date due to change (old=%i new=%i).", file.c_str(), hash, current_hash);
		return true;
	}

	std::vector<std::string> extra_deps = Read_Extra_Dependencies(file.c_str());

	// Are any dependent files out of date?
	for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
	{
		std::string dep = *iter;
		std::string resolved_dep = Resolve_Asset_Path(dep);

		// Is asset not compiled yet?
		file = Get_Compiled_Asset_File(dep);
		//if (!Platform::Get()->Is_File(file.c_str()))
		//{
		//	return true;
		//}

		// Is the main source file out of date.

		hash = Calculate_File_Hash(StreamFactory::Get_Last_Modified(resolved_dep.c_str()), asset.Version);
		current_hash = Read_Hash(file.c_str());
		if (hash != current_hash)
		{
			//DBG_LOG("[%s] Out of date due to dependent change in '%s'.", file.c_str(), resolved_dep.c_str());
			return true;
		}
	}

	//DBG_LOG("------------- %s ---------------", asset.Source_Path.c_str());

	// Load extra dependencies.
	for (std::vector<std::string>::iterator iter = extra_deps.begin(); iter != extra_deps.end(); iter++)
	{
		std::string dep = *iter;
		std::string resolved_dep = Resolve_Asset_Path(dep);

		//DBG_LOG("%s", resolved_dep.c_str());

		// Is asset not compiled yet?
		file = Get_Compiled_Asset_File(dep);
		//if (!Platform::Get()->Is_File(file.c_str()))
		//{
		//	return true;
		//}

		// Is the main source file out of date.

		hash = Calculate_File_Hash(StreamFactory::Get_Last_Modified(resolved_dep.c_str()), asset.Version);
		current_hash = Read_Hash(file.c_str());
		if (hash != current_hash)
		{
			//DBG_LOG("[%s] Out of date due to dependent change in '%s'.", file.c_str(), resolved_dep.c_str());
			return true;
		}
	}

	return false;
}

bool AssetBuilder::Build_Assets()
{
	// Check for files to recompile first.
	std::vector< std::vector<AssetToCompile> > compiler_assets;

	DBG_LOG("Checking status of resources...");

	int in_need_of_recompile = 0;
	for (std::vector<AssetCompiler*>::iterator iter = m_compilers.begin(); iter != m_compilers.end(); iter++)
	{
		AssetCompiler* compiler = *iter;

		std::vector<AssetToCompile> assets = compiler->Gather_Assets(this);

		for (std::vector<AssetToCompile>::iterator iter2 = assets.begin(); iter2 != assets.end(); iter2++)
		{
			AssetToCompile& asset = *iter2;
			asset.Version = compiler->Get_Version();
			asset.Out_Of_Date = Is_Asset_Out_Of_Date(asset);

			if (asset.Out_Of_Date)
			{
				in_need_of_recompile++;
			}
		}

		compiler_assets.push_back(assets);
	}

	if (in_need_of_recompile == 0)
	{
		return true;
	}

	// Start up build tasks for every asset.
	std::vector< std::vector<AssetToCompile> >::iterator asset_iter = compiler_assets.begin();
	for (std::vector<AssetCompiler*>::iterator iter = m_compilers.begin(); iter != m_compilers.end(); iter++, asset_iter++)
	{
		AssetCompiler* compiler = *iter;
		std::vector<AssetToCompile>& assets = *(asset_iter);

		for (std::vector<AssetToCompile>::iterator iter2 = assets.begin(); iter2 != assets.end(); iter2++)
		{
			AssetToCompile& asset = *iter2;
			if (asset.Out_Of_Date)
			{
				AssetBuildTask* task = new AssetBuildTask();
				task->Builder = this;
				task->Success = false;
				task->Asset = &(*iter2);
				task->Compiler = compiler;
				task->Index = m_build_tasks.size();
				task->ID = m_task_manager->Add_Task(task);
				m_build_tasks.push_back(task);
			}
		}
	}

	// Start em all up.
	for (std::vector<AssetBuildTask*>::iterator iter = m_build_tasks.begin(); iter != m_build_tasks.end(); iter++)
	{
		AssetBuildTask* task = *iter;
		m_task_manager->Queue_Task(task->ID);
	}

	// Wait for all tasks to complete.
	m_task_manager->Wait_For_All();

	// Update timestamp info.
	bool any_failed = false;
	for (std::vector<AssetBuildTask*>::iterator iter = m_build_tasks.begin(); iter != m_build_tasks.end(); iter++)
	{
		AssetBuildTask* task = *iter;
		AssetToCompile& asset = *task->Asset;

		if (task->Success == true)
		{
			std::string file = Get_Compiled_Asset_File(asset.Source_Path);
			std::string resfile = Resolve_Asset_Path(asset.Source_Path);

			u64 hash = Calculate_File_Hash(StreamFactory::Get_Last_Modified(resfile.c_str()), asset.Version);
			Write_Hash(file.c_str(), hash);

			for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
			{
				std::string dep = *iter;
				std::string dep_file = Get_Compiled_Asset_File(dep);
				std::string resdepfile = Resolve_Asset_Path(dep);

				u64 hash = Calculate_File_Hash(StreamFactory::Get_Last_Modified(resdepfile.c_str()), asset.Version);
				Write_Hash(dep_file.c_str(), hash);
			}

			m_assets_rebuilt++; 
		}
		else
		{
			any_failed = true;
		}
	}

	// Any errors?
	if (any_failed)
	{
		return false;
	}

	// Finalize all compilers.
	asset_iter = compiler_assets.begin();
	for (std::vector<AssetCompiler*>::iterator iter = m_compilers.begin(); iter != m_compilers.end(); iter++, asset_iter++)
	{
		AssetCompiler* compiler = *iter;
		std::vector<AssetToCompile>& assets = *(asset_iter);

		std::vector<AssetToCompile> rebuilt;

		for (std::vector<AssetToCompile>::iterator iter2 = assets.begin(); iter2 != assets.end(); iter2++)
		{
			AssetToCompile& asset = *iter2;
			if (asset.Out_Of_Date)
			{
				rebuilt.push_back(asset);
			}

			// Store source file information.
			m_source_files.push_back(asset.Source_Path);
			for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
			{
				std::string dep = *iter;
 				m_source_files.push_back(dep);
			}

			// Add extra dependencies to source files.
			std::vector<std::string> extra_deps = Read_Extra_Dependencies(Get_Compiled_Asset_File(asset.Source_Path).c_str());
			for (std::vector<std::string>::iterator iter = extra_deps.begin(); iter != extra_deps.end(); iter++)
			{
				std::string dep = *iter;
				m_source_files.push_back(dep);
			}

			// Add chunks to the package file.
			compiler->Add_Chunks(this, asset);
		}

		compiler->Finalize(this, assets, rebuilt);
	}

/*
	std::vector<std::vector<AssetToCompile>>::iterator asset_iter = compiler_assets.begin();
	for (std::vector<AssetCompiler*>::iterator iter = m_compilers.begin(); iter != m_compilers.end(); iter++, asset_iter++)
	{
		AssetCompiler* compiler = *iter;
		std::vector<AssetToCompile> assets = *(asset_iter);

		int build_counter = 0;
		std::vector<AssetToCompile> rebuilt;

		DBG_LOG("Building using %s ...", typeid(*compiler).name());

		for (std::vector<AssetToCompile>::iterator iter2 = assets.begin(); iter2 != assets.end(); iter2++)
		{
			AssetToCompile& asset = *iter2;

			if (asset.Out_Of_Date)
			{
				DBG_LOG("Building asset [%i/%i]: %s.", build_counter, in_need_of_recompile, asset.Source_Path.c_str());
				build_counter++;

				//for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
				//{
				//	std::string dep = *iter;
					//DBG_LOG(" - %s", dep.c_str());
				//}
				
				rebuilt.push_back(asset);

				if (!compiler->Build_Asset(this, asset))
				{
					return false;
				}
				else
				{
					std::string file = Get_Compiled_Asset_File(asset.Source_Path);
					std::string resfile = Resolve_Asset_Path(asset.Source_Path);

					u64 hash = Calculate_File_Hash(StreamFactory::Get_Last_Modified(resfile.c_str()), asset.Version);
					Write_Hash(file.c_str(), hash);
				
					for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
					{
						std::string dep = *iter;
						std::string dep_file = Get_Compiled_Asset_File(dep);
						std::string resdepfile = Resolve_Asset_Path(dep);

						u64 hash = Calculate_File_Hash(StreamFactory::Get_Last_Modified(resdepfile.c_str()), asset.Version);
						Write_Hash(dep_file.c_str(), hash);
					}

					m_assets_rebuilt++;
				}
			}

			// Store source file information.
			m_source_files.push_back(asset.Source_Path);
			for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
			{
				std::string dep = *iter;
				m_source_files.push_back(dep);
			}

			// Add chunks to the package file.
			compiler->Add_Chunks(this, asset);
		}

		compiler->Finalize(this, assets, rebuilt);
	}
*/

	return true;
}

bool AssetBuilder::Create_Pack_File()
{
	// Add every source file to the pack file if requested.
	if (m_pak_keep_source == true)
	{
		DBG_LOG("Adding source files to package.");
		for (std::vector<std::string>::iterator iter = m_source_files.begin(); iter != m_source_files.end(); iter++)
		{
			std::string path = *iter;
			std::string source_file = Resolve_Asset_Path(path);

			// If we failed to resolve to a valid asset path its because the source asset exists in another package, so no point
			// including it in this pak files.
			if (Platform::Get()->Is_File(source_file.c_str()))
			{
				m_package_file.Add_Chunk(source_file.c_str(), path.c_str(), PackageFileChunkType::Source, false, "", false);
			}
		}
	}

	// Write everything out.
	DBG_LOG("Writing package to disk.");
	DBG_LOG("  Output: %s", m_pak_file.c_str());
	m_package_file.Set_Keys(m_private_key, CryptoKey::Empty);
	m_package_file.Write_To_Disk(m_pak_file.c_str());

	DBG_LOG("  Signature:");

	std::string sig = "";
	for (int i = 0; i < PackageFileHeader::SIGNATURE_LENGTH; i++)
	{
		sig += StringHelper::Format("%02X", (unsigned char)m_package_file.Get_Header()->signature[i] & 0xff);
		
		if ((i % 8) == 7)
		{
			DBG_LOG("%s\n", sig.c_str());
			sig = "";
		}
	}

	return true;
}

void AssetBuildTask::Run()
{
	Builder->Build(this);
}

void AssetBuilder::Build(AssetBuildTask* task)
{
	AssetToCompile& asset = *task->Asset;

	DBG_LOG("[%s] Building asset [%i/%i]: %s.", Thread::Get_Current()->Get_Name().c_str(), task->Index, m_build_tasks.size(), asset.Source_Path.c_str());

	task->Success = task->Compiler->Build_Asset(this, asset);

	if (task->Success)
	{
		DBG_LOG("[%s] Built asset [%i/%i]: %s.", Thread::Get_Current()->Get_Name().c_str(), task->Index, m_build_tasks.size(), asset.Source_Path.c_str());
	}
	else
	{
		DBG_LOG("[%s] Failed to build asset [%i/%i]: %s.", Thread::Get_Current()->Get_Name().c_str(), task->Index, m_build_tasks.size(), asset.Source_Path.c_str());
	}
}

bool AssetBuilder::Parse_Command_Line(const char* command_line)
{
	std::string command = "";
	std::vector<std::string> options = StringHelper::Crack_Command_Line(command_line);

	if (options.size() <= 1)
	{
		goto error_exit;
	}

	command = StringHelper::Lowercase(options[1].c_str());
	if (command == "create")
	{
		m_build_mode = BuilderMode::Create;
		if (options.size() < 2)
		{
			goto error_exit;
		}
		m_package_file_name = options[2];
		return true;
	}
	else if (command == "delete")
	{
		m_build_mode = BuilderMode::Delete;
		if (options.size() < 2)
		{
			goto error_exit;
		}
		m_package_file_name = options[2];
		return true;
	}
	else if (command == "build")
	{
		m_build_mode = BuilderMode::Build;
		m_rebuild = false;
		if (options.size() < 2)
		{
			goto error_exit;
		}
		m_package_file_name = options[2];
		return true;
	}
	else if (command == "rebuild")
	{
		m_build_mode = BuilderMode::Build;
		m_rebuild = true;
		if (options.size() < 2)
		{
			goto error_exit;
		}
		m_package_file_name = options[2];
		return true;
	}
	else if (command == "extract")
	{
		m_build_mode = BuilderMode::Extract;
		if (options.size() < 2)
		{
			goto error_exit;
		}
		m_package_file_name = options[2];
		return true;
	}
	else if (command == "upload")
	{
		m_build_mode = BuilderMode::Upload;
		if (options.size() < 3)
		{
			goto error_exit;
		}
		m_package_file_name = options[2];
		m_upload_description = options[3];
		return true;
	}
	else
	{
		goto error_exit;
	}

	/*
	DBG_LOG("Working Dir: %s", Platform::Get()->Get_Working_Dir().c_str());

	DBG_LOG("Registering asset builder options.");
	OptionRegistry::Create();
	AssetBuilderOptions::Register_Options();

	DBG_LOG("Parsing command line options.");
	OptionCommandLineParser parser;
	parser.Parse(command_line);

	m_package_xml_path = Platform::Get()->Get_Working_Dir() + "/" + (*AssetBuilderOptions::package);
	m_root_directory = Platform::Get()->Extract_Directory(*AssetBuilderOptions::package);

	bool exists = (Platform::Get()->Is_File(m_package_xml_path.c_str()));
	if (exists)
	{
		return true;
	}
	else
	{
		DBG_LOG("Package file dosen't exist: %s", m_package_xml_path.c_str());
		return false;
	}
	*/

error_exit:
	DBG_LOG("");
	DBG_LOG("Command Line Usage:");
	DBG_LOG("");
	DBG_LOG("\tBuilder Create <Mod_Name>");
	DBG_LOG("\t\tCreates a new mod package with the given name. Will be created under the Data directory in a folder with the packages name.");
	DBG_LOG("");
	DBG_LOG("\tBuilder Delete <Mod_Name>");
	DBG_LOG("\t\tDeletes a mod package with the given name. Be careful this will erase the mods folder and everything in it!");
	DBG_LOG("");
	DBG_LOG("\tBuilder Build <Mod_Name>");
	DBG_LOG("\t\tBuilds a distributable package file for the mod. Only build mods will be loaded by the game.");
	DBG_LOG("");
	DBG_LOG("\tBuilder Rebuild <Mod_Name>");
	DBG_LOG("\t\tRebuilds a distributable package file for the mod. Only build mods will be loaded by the game. This differs from Build as it forces a rebuild of every resources, not just an incremental build. Slow!");
	DBG_LOG("");
	DBG_LOG("\tBuilder Extract <Mod_Name>");
	DBG_LOG("\t\tExtracts all the resources held in a package. Will not overwrite existing files. This is only possible with mods that keep their sources files intact (using the pak_keep_source flag in the manifest file).");
	DBG_LOG("");
	DBG_LOG("\tBuilder Upload <Mod_Name> <Change_Note>");
	DBG_LOG("\t\tUploads the mod to the steam workshop so other users can download it. Make sure you package is signed before you do this, or everyone who uses your mod will have their games restricted!");
	DBG_LOG("");
	DBG_LOG("gl;hf");
	DBG_LOG("");

	return false;
}

void AssetBuilder::CreateMod()
{	
	std::string base_dir = StringHelper::Format("%s/%s", m_data_directory.c_str(), m_package_file_name.c_str());
	std::string xml_file = StringHelper::Format("%s/%s/%s.xml", m_data_directory.c_str(), m_package_file_name.c_str(), m_package_file_name.c_str());

	if (Platform::Get()->Is_Directory(base_dir.c_str()))
	{
		DBG_LOG("Mod directory already exists. Mod creation failed.");
		DBG_LOG("Directory: %s", base_dir.c_str());
		return;
	}

	Platform::Get()->Create_Directory(base_dir.c_str(), true);

	Stream* stream = StreamFactory::Open(xml_file.c_str(), StreamMode::Write);

	stream->WriteLine("<xml>");
	stream->WriteLine("	<package>");
	stream->WriteLine(StringHelper::Format("		<name>%s</name>", m_package_file_name.c_str()).c_str());
	stream->WriteLine("		<root>Data/</root>");
	stream->WriteLine(StringHelper::Format("		<compile_directory>../../Output/Data/%s/.compiled/</compile_directory>", m_package_file_name.c_str()).c_str());
	stream->WriteLine(StringHelper::Format("		<script_directory>../../Data/Base/Scripts/</script_directory>").c_str());
	stream->WriteLine(StringHelper::Format("		<pak_file>../../Data/%s/%s.dat</pak_file>", m_package_file_name.c_str(), m_package_file_name.c_str()).c_str());
	stream->WriteLine("		<private_key></private_key>");
	stream->WriteLine("		<pak_keep_source>True</pak_keep_source>");
	stream->WriteLine("		<pak_use_compression>True</pak_use_compression>");
	stream->WriteLine("		<workshop>");
	stream->WriteLine("			<id></id>");
	stream->WriteLine("			<title></title>");
	stream->WriteLine("			<preview_file></preview_file>");
	stream->WriteLine("			<description></description>");
	stream->WriteLine("			<tags></tags>");
	stream->WriteLine("		</workshop>");
	stream->WriteLine("	</package>");
	stream->WriteLine("	<languages />");
	stream->WriteLine("	<atlases />");
	stream->WriteLine("	<audio_banks />");
	stream->WriteLine("	<fonts />");
	stream->WriteLine("	<particlefx />");
	stream->WriteLine("	<scripts />");
	stream->WriteLine("	<shaders />");
	stream->WriteLine("	<raw />");
	stream->WriteLine("	<layouts />");
	stream->WriteLine("	<maps />");
	stream->WriteLine("	<videos />");
	stream->WriteLine("</xml>");

	SAFE_DELETE(stream);

	DBG_LOG("Mod created at: %s", base_dir.c_str());
}

void AssetBuilder::DeleteMod()
{
	DBG_LOG("This option is currently not implemented to prevent accidents! Delete the directory manually instead!");
}

void AssetBuilder::BuildMod()
{
	double start_time = Platform::Get()->Get_Ticks();

	m_package_xml_path = StringHelper::Format("%s/%s/%s.xml", m_data_directory.c_str(), m_package_file_name.c_str(), m_package_file_name.c_str());
	m_root_directory = Platform::Get()->Extract_Directory(m_package_xml_path.c_str());

	DBG_LOG("Setting up build environment.");
	if (!Setup_Build_Environment())
	{
		DBG_LOG("Failed to setup build environment.");
		return;
	}

	DBG_LOG("Building assets.");
	if (!Build_Assets())
	{
		DBG_LOG("Asset Builder: Failed to build assets.");
		printf("Asset Builder: Failed to build assets.\n"); // So VS picks up on it.
		return;
	}

	if (m_assets_rebuilt == 0 &&
		Platform::Get()->Is_File(m_pak_file.c_str()))
	{
		DBG_LOG("No assets to rebuild.");
	}
	else
	{
		DBG_LOG("%i assets were rebuilt.", m_assets_rebuilt);
		DBG_LOG("Creating pack file.");
		if (!Create_Pack_File())
		{
			DBG_LOG("Failed to create pack file.");
			return;
		}
	}

	double elapsed = Platform::Get()->Get_Ticks() - start_time;
	int seconds = (int)(elapsed / 1000) % 60;
	int minutes = (int)((elapsed / 1000) / 60) % 60;
	int hours = (int)((elapsed / 1000) / 60) / 60;
	DBG_LOG("Built in %02i:%02i:%02i", hours, minutes, seconds);
	DBG_LOG("");
	CTranslationUnit::Print_Compile_Stats();
}

void AssetBuilder::ExtractMod()
{
	m_package_xml_path = StringHelper::Format("%s/%s/%s.xml", m_data_directory.c_str(), m_package_file_name.c_str(), m_package_file_name.c_str());
	m_root_directory = Platform::Get()->Extract_Directory(m_package_xml_path.c_str());

	DBG_LOG("Setting up build environment.");

	if (!Setup_Build_Environment())
	{
		DBG_LOG("Failed to setup build environment.");
		return;
	}

	m_package_file.Open(m_pak_file.c_str());

	for (int i = 0; i < m_package_file.Get_Chunk_Count(); i++)
	{
		PackageFileChunk* chunk = m_package_file.Get_Chunk(i);
		if (chunk->type == PackageFileChunkType::Source)
		{
			std::string filename = m_package_file.Get_String(chunk->name_offset);
			std::string output_filename = filename;
			if (output_filename.size() >= 5 && StringHelper::Lowercase(output_filename.c_str()).substr(0, 5) == "data/")
			{
				output_filename = StringHelper::Format("%s/%s", m_root_directory.c_str(), output_filename.substr(5).c_str());
			}

			std::string output_dir = Platform::Get()->Extract_Directory(output_filename.c_str());
			Platform::Get()->Create_Directory(output_dir.c_str(), true);

			if (!Platform::Get()->Is_File(output_filename.c_str()))
			{
				DBG_LOG("Extracting: %s -> %s", filename.c_str(), output_filename.c_str());

				Stream* stream = m_package_file.Open_Chunk_Stream(chunk);
				Stream* output_stream = StreamFactory::Open(output_filename.c_str(), StreamMode::Write);

				int len = stream->Length();
				char* buffer = new char[len];
				stream->ReadBuffer(buffer, 0, len);
				output_stream->WriteBuffer(buffer, 0, len);

				SAFE_DELETE_ARRAY(buffer);
				SAFE_DELETE(output_stream)
				SAFE_DELETE(stream);
			}
			else
			{
				DBG_LOG("Ignoring, file already exists: %s -> %s", filename.c_str(), output_filename.c_str());
			}
		}
	}
}

void AssetBuilder::UploadMod()
{
	m_package_xml_path = StringHelper::Format("%s/%s/%s.xml", m_data_directory.c_str(), m_package_file_name.c_str(), m_package_file_name.c_str());
	m_root_directory = Platform::Get()->Extract_Directory(m_package_xml_path.c_str());

	DBG_LOG("Setting up build environment.");
	if (!Setup_Build_Environment())
	{
		DBG_LOG("Failed to setup build environment.");
		return;
	}

	std::string workshop_id = m_package_config_file.Get<std::string>("package/workshop/id");
	std::string workshop_title = m_package_config_file.Get<std::string>("package/workshop/title");
	std::string workshop_preview = Platform::Get()->Get_Absolute_Path(m_package_xml_dir + "/" + m_package_config_file.Get<std::string>("package/workshop/preview_file"));
	std::string workshop_description = m_package_config_file.Get<std::string>("package/workshop/description");
	std::string workshop_tags = m_package_config_file.Get<std::string>("package/workshop/tags");

	if (workshop_title == "")
	{
		DBG_LOG("No workshop title defined in manifest file.");
		return;
	}
	if (workshop_description == "")
	{
		DBG_LOG("No workshop description defined in manifest file.");
		return;
	}
	if (workshop_tags == "")
	{
		DBG_LOG("No workshop tags defined in manifest file.");
		return;
	}
	if (workshop_preview == "")
	{
		DBG_LOG("No workshop preview image defined in manifest file.");
		return;
	}
	else
	{
		Stream* stream = StreamFactory::Open(workshop_preview.c_str(), StreamMode::Read);
		if (stream->Length() > 1024 * 1024)
		{
			SAFE_DELETE(stream);
			DBG_LOG("Workshop preview file must be less than 1MiB in size.");
			return;
		}
	}

	FrameTime time;

	OnlinePlatform::Create();
	OnlineMods::Create();

	OnlinePlatform::Get()->Init_Client();
	OnlineMods::Get()->Initialize();

#ifdef OPT_STEAM_PLATFORM
	static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get())->Set_AppID(STEAM_APP_ID);
#endif

	u64 mod_id = 0;

	if (workshop_id == "")
	{
		bool bFinished = false;
		OnlineMods::Get()->Create_Mod(&mod_id, &bFinished);

		while (!bFinished)
		{
			OnlinePlatform::Get()->Tick(time);
			OnlineMods::Get()->Tick(time);
			Platform::Get()->Sleep(100.0f);
		}
	}
	else
	{
		mod_id = StringHelper::To_U64(workshop_id);
	}

	if (mod_id != 0)
	{
		// Save the new id into the mod file.
		m_package_config_file.Set<std::string>("package/workshop/id", StringHelper::Format("%llu", mod_id));
		m_package_config_file.Save(m_package_xml_path.c_str());

		// Create a temporary folder to dump everything in.
		std::string dir_path = m_root_directory + "/.workshop";
		std::string data_path = dir_path + "/" + m_package_name.c_str() + ".dat";
		std::string manifest_path = dir_path + "/" + m_package_name.c_str() + ".xml";
		std::string preview_path = dir_path + "/" + m_package_name.c_str() + ".png";
		Platform::Get()->Create_Directory(dir_path.c_str(), false);

		// If you go!
		Platform::Get()->Copy_File(m_package_xml_path.c_str(), manifest_path.c_str());
		Platform::Get()->Copy_File(workshop_preview.c_str(), preview_path.c_str());

		// Patch workshop id in pak file.
		Platform::Get()->Copy_File(m_pak_file.c_str(), data_path.c_str());

		PackageFile file;
		file.Open(m_pak_file.c_str());
		file.Get_Header()->workshop_id = mod_id;
		strncpy(file.Get_Header()->workshop_name, workshop_title.c_str(), PackageFileHeader::MAX_WORKSHOP_NAME);
		file.Write_Patched_Header(data_path.c_str());

		// Upload new state!
		std::vector<std::string> tags;
		StringHelper::Split(workshop_tags.c_str(), ' ', tags);
		OnlineMods::Get()->Publish_Mod(mod_id, workshop_title, workshop_description, dir_path, preview_path, m_upload_description, tags);

		// Wait till uploaded.
		int progress_print_counter = 0;
		while (true)
		{
			OnlinePlatform::Get()->Tick(time);
			OnlineMods::Get()->Tick(time);
			Platform::Get()->Sleep(100.0f);

			bool wasSuccessful = false;
			bool legalAgreementRequired = false;
			u64 uploadedBytes = 0;
			u64 totalBytes = 0;

			if (OnlineMods::Get()->Get_Mod_Update_Progress(wasSuccessful, legalAgreementRequired, mod_id, uploadedBytes, totalBytes))
			{
				if (legalAgreementRequired)
				{
					DBG_LOG("Mod successfully uploaded, but legal agreement needs to be accepted (At http://steamcommunity.com/sharedfiles/workshoplegalagreement) before it becomes public: %s", StringHelper::Format("http://steamcommunity.com/sharedfiles/filedetails/?id=%llu", mod_id).c_str());
				}
				else
				{
					DBG_LOG("Mod successfully uploaded to: %s", StringHelper::Format("http://steamcommunity.com/sharedfiles/filedetails/?id=%llu", mod_id).c_str());
				}
				break;
			}
			else
			{
				// Update upload progress.
				float progress = totalBytes == 0 ? 0.0f : ((float)uploadedBytes / (float)totalBytes);
				if (((progress_print_counter++) % 10) == 0)
				{
					DBG_LOG("Upload progress: %.1f %%", progress);
				}
			}
		}
	}
	else
	{
		DBG_LOG("Failed to create mod, aborting.");
	}

	OnlinePlatform::Get()->Destroy_Client();
}

bool AssetBuilder::Find_Data_Directory()
{
	std::vector<std::string> segments;
	Platform::Get()->Crack_Path(Platform::Get()->Normalize_Path(Platform::Get()->Get_Working_Dir()).c_str(), segments);

	for (unsigned int i = segments.size() - 1; i > 0; i--)
	{
		std::string path = "";
		for (unsigned int j = 0; j <= i; j++)
		{
			if (j != 0)
			{
				path += "/";
			}
			path = path + segments[j];
		}

		path += "/Data";

		if (Platform::Get()->Is_Directory(path.c_str()))
		{
			m_data_directory = path;
			return true;
		}
	}

	DBG_LOG("Failed to find data directory. Are we in the game directory?");
	return false;
}

void AssetBuilder::Run(const char* command_line)
{
	DBG_LOG("");

	if (!Find_Data_Directory())
	{
		return;
	}

	if (!Parse_Command_Line(command_line))
	{
		return;
	}

	switch (m_build_mode)
	{
		case BuilderMode::Create:
		{
			CreateMod();
			break;
		}
		case BuilderMode::Delete:
		{
			DeleteMod();
			break;
		}
		case BuilderMode::Build:
		{
			BuildMod();
			break;
		}
		case BuilderMode::Extract:
		{
			ExtractMod();
			break;
		}
		case BuilderMode::Upload:
		{
			UploadMod();
			break;
		}
	}
}
