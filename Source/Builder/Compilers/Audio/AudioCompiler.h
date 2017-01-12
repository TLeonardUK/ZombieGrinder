// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _BUILDER_ASSET_AUDIO_COMPILER_
#define _BUILDER_ASSET_AUDIO_COMPILER_

#include "Builder/Compilers/AssetCompiler.h"

#define AUDIO_CONVERSION_QUALITY 0.6f

class AudioCompiler : public AssetCompiler
{
private:
	enum
	{
		ogg_conversion_threshold = 1024 * 256
	};

	void Convert_To_OGG(std::string src, std::string dest);

public:
	std::vector<AssetToCompile> Gather_Assets(AssetBuilder* builder);
	bool Build_Asset(AssetBuilder* builder, AssetToCompile compile);
	void Add_Chunks(AssetBuilder* builder, AssetToCompile asset);
	int Get_Version();

};

#endif

