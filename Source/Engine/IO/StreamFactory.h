// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STREAMFACTORY_
#define _ENGINE_STREAMFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Engine/IO/Stream.h"

// WARNING:
//
// This factory creates raw-system streams, it does not take into account
// file-mappings for different resource folders. If you want to create streams
// based on file-mappings you should use the functions provided in ResourceFactory.

class StreamFactory
{
	MEMORY_ALLOCATOR(StreamFactory, "IO");

private:
	static LinkedList<StreamFactory*> m_factories;

public:
		
	// Static methods.
	static Stream* Open(const char* url, StreamMode::Type mode);
	static u64     Get_Last_Modified(const char* url);

	static bool	   Load_Text(const char* url, std::string* data);
	static bool	   Save_Text(const char* url, std::string data);

	// Constructors
	StreamFactory();	
	virtual ~StreamFactory();	

	// Derived factory methods.
	virtual Stream*	Try_Open(const char* url, StreamMode::Type mode) = 0;
	virtual u64     Try_Get_Last_Modified(const char* url) = 0;

};

#endif

