// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_ATLASES_ATLASRESOURCECOMPILER_
#define _ENGINE_RENDERER_ATLASES_ATLASRESOURCECOMPILER_

#include "Engine/Scene/Animation.h"

#include "Engine/Resources/ResourceCompiler.h"

#include "Engine/Config/ConfigFile.h"

#include "Generic/ThirdParty/RapidXML/rapidxml.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_iterators.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_utils.hpp"

class Atlas;

class AtlasResourceCompiler : public ResourceCompiler
{
	MEMORY_ALLOCATOR(AtlasResourceCompiler, "Rendering");

private:
	std::string m_input_path;
	std::string m_input_directory;
	std::string m_input_filename;
	std::string m_output_directory;
	std::string m_output_path;
	
	ConfigFile* m_config_file;

	std::vector<std::string> m_dependent_files;

protected:
	friend class Atlas;

	bool Load_Config();

	void Expand_Multiple_Path(const char* path, std::vector<std::string>& result);

	static AnimationMode::Type Parse_Animation_Mode(const char* mode);

public:

	// Constructors
	AtlasResourceCompiler		(const char* url);	
	~AtlasResourceCompiler		();	

	// Derived methods.
	bool			Should_Compile		();
	std::string		Get_Compiled_Path	();
	bool			Compile				();
	Atlas*			Load_Compiled		();

};

#endif

