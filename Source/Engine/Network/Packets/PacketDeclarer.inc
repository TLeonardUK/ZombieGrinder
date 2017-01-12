// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef PACKET_LIST_FILE
#error "PACKET_LIST_FILE must be defined when including this file!"
#endif
#ifndef PACKET_TYPE_REGISTER_FUNCTION
#error "PACKET_TYPE_REGISTER_FUNCTION must be defined when including this file!"
#endif

#include "Engine/Network/NetPacket.h"
#include "Generic/Types/DataBuffer.h"

class Stream;

#define START_PACKET(name, out_of_band) \
	class name : public NetPacket \
	{ \
	MEMORY_ALLOCATOR(name, "Network"); \
	public: \
		unsigned int Get_Type_ID(); \
		const char* Get_Name(); \
		static NetPacket* Constructor_##name();  \
		bool Is_OutOfBand(); \

#define PACKET_FIELD(type, name) \
		type name;

#define PACKET_SERIALIZABLE_FIELD(type, name) \
		type name;

#define PACKET_VECTOR_FIELD(type, name) \
		std::vector<type> name;
		
#define PACKET_ENUM_FIELD(type, name) \
		type::Type name;
		
#define PACKET_ARRAY_FIELD(type, count, name) \
		type name[count];

#define PACKET_BLOB_FIELD(name) \
		DataBuffer name;

#define END_PACKET() \
		void Serialize(Stream* stream); \
		bool Deserialize(Stream* stream); \
	}; \
	
#define START_PACKET_ENUM(name) \
	struct name \
	{ \
		enum Type \
		{
#define PACKET_ENUM_ELEMENT(name) \
			name,
#define END_PACKET_ENUM() \
		}; \
	};

#include PACKET_LIST_FILE

#undef START_PACKET
#undef END_PACKET
#undef PACKET_ENUM_FIELD
#undef PACKET_ARRAY_FIELD
#undef PACKET_VECTOR_FIELD
#undef PACKET_SERIALIZABLE_FIELD
#undef PACKET_FIELD
#undef PACKET_BLOB_FIELD

#undef START_PACKET_ENUM
#undef PACKET_ENUM_ELEMENT	
#undef END_PACKET_ENUM

void PACKET_TYPE_REGISTER_FUNCTION();