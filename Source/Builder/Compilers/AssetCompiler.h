// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _BUILDER_ASSET_COMPILER_
#define _BUILDER_ASSET_COMPILER_

#include <vector>
#include <string>

class AssetBuilder;

struct AssetToCompile
{
public:
	std::string					Source_Path;
	std::vector<std::string>	Dependent_Files;
	std::string					Priority;
	int							Version;
	bool						Out_Of_Date;
};

class AssetCompiler
{
private:
	
public:
	virtual std::vector<AssetToCompile> Gather_Assets(AssetBuilder* builder) = 0;
	virtual bool Build_Asset(AssetBuilder* builder, AssetToCompile asset) = 0;
	virtual void Add_Chunks(AssetBuilder* builder, AssetToCompile asset) = 0;
	virtual int Get_Version() = 0;

	virtual void Finalize(AssetBuilder* builder, std::vector<AssetToCompile> assets, std::vector<AssetToCompile> rebuilt)
	{
		// this is used for doing things like generating manifests/linking scripts etc.
	}
};

#endif

