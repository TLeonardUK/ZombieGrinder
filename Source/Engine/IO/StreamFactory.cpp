// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/IO/StreamFactory.h"
#include "Engine/Resources/ResourceFactory.h"

#include <string>

LinkedList<StreamFactory*> StreamFactory::m_factories;

bool StreamFactory::Load_Text(const char* url, std::string* data)
{
	Stream* stream = Open(url, StreamMode::Read);
	if (stream == NULL)
	{
		return false;
	}

	*data = stream->ReadString(stream->Length());

	stream->Close();
	SAFE_DELETE(stream);

	return true;
}

bool StreamFactory::Save_Text(const char* url, std::string data)
{
	Stream* stream = Open(url, StreamMode::Write);
	if (stream == NULL)
	{
		return false;
	}

	stream->WriteString(data.c_str());

	stream->Close();
	SAFE_DELETE(stream);

	return true;
}

Stream* StreamFactory::Open(const char* url, StreamMode::Type mode)
{
	for (LinkedList<StreamFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		StreamFactory* factory = *iter;
		Stream* result = factory->Try_Open(url, mode);
		if (result != NULL)
		{
			//DBG_LOG("Successfully opened stream to: %s", url);
			return result;
		}
	}
	
	//DBG_LOG("Failed to open stream to: %s", url);
	return NULL;
}

u64 StreamFactory::Get_Last_Modified(const char* url)
{
	for (LinkedList<StreamFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		StreamFactory* factory = *iter;
		u64 result = factory->Try_Get_Last_Modified(url);
		if (result != 0)
		{
			return result;
		}
	}
	
//	DBG_LOG("Failed to get last modified timestamp for: %s", url);
	return 0;
}

StreamFactory::StreamFactory()
{
	m_factories.Add(this);
}

StreamFactory::~StreamFactory()
{
	m_factories.Remove(m_factories.Find(this));
}