// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _BUILDER_ASSET_SCRIPT_COMPILER_
#define _BUILDER_ASSET_SCRIPT_COMPILER_

#include "Builder/Compilers/AssetCompiler.h"

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

class CClassASTNode;
class CClassMemberASTNode;
class Stream;
class CDataType;

class ScriptCompiler : public AssetCompiler
{
private:
	Mutex* m_build_mutex;

	void Dump_Native_Interface(CClassASTNode* class_node, Stream* header_file, Stream* source_file);
	void Dump_Native_Member(std::string class_name, std::string native_class_name, CClassMemberASTNode* member, Stream* header_file, Stream* source_file, bool bind);
	std::string Native_Data_Type(CDataType* type);

public:
	ScriptCompiler();
	~ScriptCompiler();

	std::vector<AssetToCompile> Gather_Assets(AssetBuilder* builder);
	bool Build_Asset(AssetBuilder* builder, AssetToCompile compile);
	void Add_Chunks(AssetBuilder* builder, AssetToCompile asset);
	int Get_Version();
	
	void Finalize(AssetBuilder* builder, std::vector<AssetToCompile> assets, std::vector<AssetToCompile> rebuilt);

};

#endif

