// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MONITOR_BASE_PACKET_
#define _GAME_MONITOR_BASE_PACKET_

#include "Generic/ThirdParty/RapidJSON/include/rapidjson/document.h"
#include "Generic/ThirdParty/RapidJSON/include/rapidjson/writer.h"
#include "Generic/ThirdParty/RapidJSON/include/rapidjson/stringbuffer.h"

#include <string>

struct MonitorServerConnection;

#define SERIALIZE_STRING(FieldName, Name) \
	if (bEncoding) \
	{ \
		value.Writer.Key(Name); \
		value.Writer.String(FieldName.c_str()); \
	} \
	else \
	{ \
		FieldName = (*value.ReadRoot)[Name].GetString(); \
	}

#define SERIALIZE_INT(FieldName, Name) \
	if (bEncoding) \
	{ \
		value.Writer.Key(#FieldName); \
		value.Writer.Int(FieldName); \
	} \
	else \
	{ \
		FieldName = (*value.ReadRoot)[#FieldName].GetInt();	\
	}

#define SERIALIZE_BOOL(FieldName, Name) \
	if (bEncoding) \
	{ \
		value.Writer.Key(#FieldName); \
		value.Writer.Int(FieldName ? 1 : 0); \
	} \
			else \
	{ \
		FieldName = !!(*value.ReadRoot)[#FieldName].GetInt(); \
	}

// TODO: Make this shit support decoding!
#define SERIALIZE_ARRAY_START(FieldName, Name, ElementType, ElementName) \
	int Index_##ElementName = 0; \
	DBG_ASSERT(bEncoding); \
	if (bEncoding) \
	{ \
		value.Writer.Key(Name); \
		value.Writer.StartArray(); \
	} \
	while (true) \
	{ \
		if (bEncoding && Index_##ElementName >= (int)FieldName.size()) \
		{ \
			break; \
		} \
		ElementType& ElementName = FieldName[Index_##ElementName]; \
		value.Writer.StartObject(); 

#define SERIALIZE_ARRAY_END(FieldName, Name, ElementType, ElementName) \
		value.Writer.EndObject(); \
		Index_##ElementName++; \
	} \
	if (bEncoding) \
	{ \
		value.Writer.EndArray(); \
	} 

// TODO: Make this shit support decoding!
#define SERIALIZE_OBJECT_START(FieldName, Name) \
	if (Name != "") \
	{ \
		value.Writer.Key(Name); \
	} \
	DBG_ASSERT(bEncoding); \
	value.Writer.StartObject(); 

#define SERIALIZE_OBJECT_END(FieldName, Name) \
	value.Writer.EndObject(); \

struct MonitorPacketSerializer
{
public:
	rapidjson::StringBuffer Buffer;
	rapidjson::Writer<rapidjson::StringBuffer> Writer; 
	rapidjson::Value* ReadRoot;

	MonitorPacketSerializer()
		: Writer(Buffer)
	{
	}
};

class BaseMonitorPacket
{
public:
	std::string ClassName;
	int ID;
	int RequestID;

	BaseMonitorPacket(std::string ClassName);
	
	virtual void Serialize(MonitorPacketSerializer& value, bool bEncoding);
	virtual void Process(MonitorServerConnection& connection) = 0;

	static BaseMonitorPacket* Create(const char* ClassName);

};

#endif