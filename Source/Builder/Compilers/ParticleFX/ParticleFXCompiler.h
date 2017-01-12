// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _BUILDER_ASSET_PARTICLEFX_COMPILER_
#define _BUILDER_ASSET_PARTICLEFX_COMPILER_

#include "Builder/Compilers/AssetCompiler.h"

class ParticleFXCompiler : public AssetCompiler
{
private:

public:
	std::vector<AssetToCompile> Gather_Assets(AssetBuilder* builder);
	bool Build_Asset(AssetBuilder* builder, AssetToCompile compile);
	void Add_Chunks(AssetBuilder* builder, AssetToCompile asset);
	int Get_Version();

};

#endif

