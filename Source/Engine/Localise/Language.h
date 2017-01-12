// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_LOCALISE_LANGUAGE_
#define _ENGINE_LOCALISE_LANGUAGE_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Generic/Patterns/Singleton.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/Resources/Compiled/Languages/CompiledLanguage.h"

#include "Generic/ThirdParty/RapidXML/rapidxml.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_iterators.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_utils.hpp"

#include "Engine/Resources/Reloadable.h"

#include <string>

class Texture;
class TextureHandle;
class Language;

struct LanguageStrings
{
public:
	ThreadSafeHashTable<const char*, unsigned int>* Strings;
	int Priority;
	Language* Base;

	static bool Sort_Predicate(LanguageStrings a, LanguageStrings b)
	{
		return a.Priority > b.Priority;
	}
};

class Language
{
	MEMORY_ALLOCATOR(Language, "Engine");

private:
	void* m_resource_data;

public:	
	ThreadSafeHashTable<const char*, unsigned int>	Strings;

	const char*								Short_Name;
	const char*								Long_Name;

	const char*								URL;

	std::vector<LanguageStrings>			Language_Strings;
	std::vector<Language*>					Sub_Languages;

	int										Priority;

public:
	Language();
	~Language();

	bool Load_Compiled_Config(CompiledLanguageHeader* config);
	void Add_Sub_Language(Language* language);
	void Refresh_Strings();

};

#endif

