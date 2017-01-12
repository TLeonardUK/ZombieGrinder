// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_XMLFILE_
#define _ENGINE_XMLFILE_

#include "Generic/Types/IntVector3.h"
#include "Generic/Types/Vector3.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/ThirdParty/RapidXML/rapidxml.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_iterators.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_utils.hpp"
 
class XMLFile
{
private:
	rapidxml::xml_document<>*	m_xml_document;
	char*						m_source_buffer;
	int							m_source_buffer_len;

private:

	// Memory management.
	bool Resize_Buffer(int size);

public:
	
	// Constructor!
	XMLFile();
	~XMLFile();

	// Save & load options.
	bool Load(const char* path);

};

#endif

