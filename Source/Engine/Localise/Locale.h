// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_LOCALISE_LOCALE_
#define _ENGINE_LOCALISE_LOCALE_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/HashTable.h"
#include "Generic/Patterns/Singleton.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/ThirdParty/RapidXML/rapidxml.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_iterators.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_utils.hpp"

#include "Engine/Resources/Reloadable.h"

#include <string>

class Texture;
class TextureHandle;
class Language;
class LanguageHandle;

// Quick macro to get a localised string.
#define S(id)		(Locale::Get()->Get_String((id)))
#define SF(id, ...) (StringHelper::Format(Locale::Get()->Get_String((id)), __VA_ARGS__).c_str())

class Locale : public Singleton<Locale>
{
	MEMORY_ALLOCATOR(Locale, "Engine");

private:
	LanguageHandle* m_current_language;

public:
	Locale();
	~Locale();
	
	bool Change_Language(const char* name);
	bool Change_Language(LanguageHandle* handle);
	
	const char* Get_String_Formatted(std::string& id, bool return_null = false, ...);

	const char* Get_String(std::string& id, bool return_null = false);
	const char* Get_String(const char* id, bool return_null = false);
	const char* Get_String(int id, bool return_null = false);

};

#endif

