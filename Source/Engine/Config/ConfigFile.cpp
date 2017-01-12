// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Config/ConfigFile.h"
#include "Engine/IO/StreamFactory.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Helper/StringHelper.h"

ConfigFile::ConfigFile()
	: m_source_buffer(NULL)
	, m_source_buffer_len(0)
	, m_xml_document(new rapidxml::xml_document<>())
	, m_root_element("xml")
{
}

ConfigFile::ConfigFile(const ConfigFile& other)
	: m_root_element("xml")
{
	m_source_buffer = NULL;
	m_source_buffer_len = 0;
	m_xml_document = NULL;
}

ConfigFile::~ConfigFile()
{
	SAFE_DELETE(m_xml_document);
	SAFE_DELETE_ARRAY(m_source_buffer);
}

ConfigFileBuffer ConfigFile::Get_Buffer()
{
	ConfigFileBuffer buffer;

	buffer.m_source_buffer		= m_source_buffer;
	buffer.m_source_buffer_len  = m_source_buffer_len;
	buffer.m_xml_document		= m_xml_document;

	m_xml_document		= NULL;
	m_source_buffer		= NULL;
	m_source_buffer_len = 0;

	return buffer;
}

void ConfigFile::Set_Buffer(ConfigFileBuffer buffer)
{
	m_source_buffer		= buffer.m_source_buffer;
	m_source_buffer_len = buffer.m_source_buffer_len;
	m_xml_document		= buffer.m_xml_document;
}

bool ConfigFile::Resize_Buffer(int size)
{
	if (m_source_buffer == NULL || m_source_buffer_len < size)
	{
		if (m_source_buffer != NULL)
		{
			SAFE_DELETE_ARRAY(m_source_buffer);
		}

		m_source_buffer = new char[size + 1];
		m_source_buffer[size] = '\0';

		if (m_source_buffer == NULL)
		{
			return false;
		}
	}

	return true;
}

bool ConfigFile::Save(const char* path)
{
	Stream* stream = NULL;

	ResourceFactory* factory = ResourceFactory::Try_Get();
	if (factory != NULL && Platform::Get()->Is_Path_Relative(path))
	{
		stream = factory->Open(path, (StreamMode::Type)(StreamMode::Write | StreamMode::Truncate));
	}
	else
	{
		stream = StreamFactory::Open(path, (StreamMode::Type)(StreamMode::Write | StreamMode::Truncate));
	}

	if (stream == NULL)
	{
		return false;
	}

	// Pack data.
	Pack(*this);

	// Save data.
	std::string output;
	rapidxml::print(back_inserter(output), *m_xml_document, 0);

	stream->WriteString(output.c_str());

	// Clean up and return.
	delete stream;
	return true;
}

bool ConfigFile::Save(std::string& result)
{
	// Pack data.
	Pack(*this);

	// Save data.
	std::string output;
	rapidxml::print(back_inserter(output), *m_xml_document, 0);

	result = output;

	// Clean up and return.
	return true;
}

bool ConfigFile::Load(const char* path)
{
	Stream* stream = NULL;
	ResourceFactory* factory = ResourceFactory::Try_Get();
	if (factory != NULL && Platform::Get()->Is_Path_Relative(path))
	{
		stream = factory->Open(path, StreamMode::Read);
	}
	else
	{
		stream = StreamFactory::Open(path, StreamMode::Read);
	}

	if (stream == NULL)
	{
		return false;
	}

	// Load source in a single string.
	int source_len = stream->Length();
	if (!Resize_Buffer(source_len))
	{
		DBG_LOG("Could not load xml-source string into memory.");
		delete stream;
		return false;
	}
	stream->ReadBuffer(m_source_buffer, 0, source_len);

	// Try and parse XML.
	try
	{
		m_xml_document->parse<rapidxml::parse_no_data_nodes>(m_source_buffer);
	}
	catch (rapidxml::parse_error error)
	{
		const char* offset = error.where<char>();
		int line = 0;
		int column = 0;

		StringHelper::Find_Line_And_Column(m_source_buffer, offset - m_source_buffer, line, column);

		DBG_LOG("Failed to parse XML with error @ %i:%i: %s", line, column, error.what());
		delete stream;
		return false;
	}

	// Unpack data.
	Unpack(*this);

	// Clean up and return.
	delete stream;
	return true;
}
	
bool ConfigFile::Load(const char* buffer, int size)
{
	// Load source in a single string.
	if (!Resize_Buffer(size))
	{
		DBG_LOG("Could not load xml-source string into memory.");
		return false;
	}
	memcpy(m_source_buffer, buffer, size);

	// Try and parse XML.
	try
	{
		m_xml_document->parse<rapidxml::parse_no_data_nodes>(m_source_buffer);
	}
	catch (rapidxml::parse_error error)
	{
		const char* offset = error.where<char>();
		int line = 0;
		int column = 0;

		StringHelper::Find_Line_And_Column(m_source_buffer, offset - m_source_buffer, line, column);

		DBG_LOG("Failed to parse XML with error @ %i:%i: %s", line, column, error.what());
		return false;
	}

	// Unpack data.
	Unpack(*this);

	// Clean up and return.
	return true;
}
	
void ConfigFile::Unpack(ConfigFile& file)
{
}

void ConfigFile::Pack(ConfigFile& file)
{
}

