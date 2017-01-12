// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _BUILDER_ASSET_BUILDER_
#define _BUILDER_ASSET_BUILDER_

#include "Engine/Config/ConfigFile.h"
#include "Engine/Resources/PackageFile.h"

#include "Builder/Compilers/AssetCompiler.h"

#include "Engine/Tasks/TaskManager.h"
#include "Engine/Tasks/GroupTask.h"

#include "Engine/Crypto/CryptoStream.h"

#include <vector>

class AssetCompiler;

namespace BuilderMode
{
	enum Type
	{
		Create,
		Delete,
		Build,
		Extract,
		Upload
	};
}

class AssetBuildTask : public Task
{
public:
	int Index;
	AssetCompiler* Compiler;
	AssetBuilder* Builder;
	bool Success;
	AssetToCompile* Asset;
	TaskID ID;

public:
	void Run();
};

class AssetBuilder
{
private:
	std::vector<AssetCompiler*> m_compilers;
	std::string m_package_xml_path;
	std::string m_package_xml_dir;
	ConfigFile m_package_config_file;
	std::string m_package_name;
	std::string m_package_root;
	std::string m_package_compile_dir;
	std::string m_root_directory;
	int m_assets_rebuilt;
	std::string m_pak_file;
	bool m_pak_keep_source;
	bool m_pak_use_compression;

	std::string m_package_file_name;
	BuilderMode::Type m_build_mode;
	std::string m_private_key_path;
	CryptoKey m_private_key;
	
	std::string m_data_directory;

	bool m_rebuild;

	PackageFile m_package_file;
	std::vector<std::string> m_source_files;

	TaskManager* m_task_manager;
	std::vector<AssetBuildTask*> m_build_tasks;
	bool m_build_errors_occured;

	std::string m_upload_description;

private:
	friend class AssetBuildTask;

	bool Parse_Command_Line(const char* command_line);
	bool Setup_Build_Environment();
	bool Is_Asset_Out_Of_Date(AssetToCompile asset);
	bool Build_Assets();
	bool Create_Pack_File();

	u64 Read_Hash(std::string path);
	void Write_Hash(std::string path, u64 version);
	
	u64 Calculate_File_Hash(u64 last_modified, int version);

	void Build(AssetBuildTask* task);

	void CreateMod();
	void DeleteMod();
	void BuildMod();
	void ExtractMod();
	void UploadMod();

	bool Find_Data_Directory();

public:
	AssetBuilder();
	~AssetBuilder();

	PackageFile* Get_Package_File();
	ConfigFile* Get_Package_Config_File();
	std::string Get_Root_Dir();
	std::string Get_Package_Root_Dir();
	std::string Get_Package_Name();
	std::string Get_Compile_Dir();
	std::string Get_Package_XML_Dir();

	std::string Resolve_Asset_Path(std::string path);
	std::string Get_Compiled_Asset_File(std::string path);

	std::vector<std::string> Read_Extra_Dependencies(std::string file);
	void Write_Extra_Dependencies(AssetToCompile asset, std::vector<std::string> deps);

	void Run(const char* command_line);

};

#endif

