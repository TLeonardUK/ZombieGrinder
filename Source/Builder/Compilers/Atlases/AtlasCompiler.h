// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _BUILDER_ASSET_ATLAS_COMPILER_
#define _BUILDER_ASSET_ATLAS_COMPILER_

#include "Builder/Compilers/AssetCompiler.h"

#include "Engine/Renderer/Atlases/Atlas.h"

class AtlasCompiler : public AssetCompiler
{
private:
	AnimationMode::Type Parse_Animation_Mode(const char* mode);
	void Expand_Multiple_Path(AssetBuilder* builder, const char* path, std::vector<std::string>& result);

public:
	std::vector<AssetToCompile> Gather_Assets(AssetBuilder* builder);
	bool Build_Asset(AssetBuilder* builder, AssetToCompile compile);	
	void Add_Chunks(AssetBuilder* builder, AssetToCompile asset);
	int Get_Version();

};

#endif

