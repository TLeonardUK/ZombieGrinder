// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
//#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/IO/Stream.h"
#include "Generic/Helper/StringHelper.h"

#ifndef PACKET_LIST_FILE
#error "PACKET_LIST_FILE must be defined when including this file!"
#endif
#ifndef PACKET_TYPE_REGISTER_FUNCTION
#error "PACKET_TYPE_REGISTER_FUNCTION must be defined when including this file!"
#endif

#define START_PACKET(name, out_of_band) \
	unsigned int name::Get_Type_ID() \
	{ \
		static unsigned int type_hash = StringHelper::Hash(#name);\
		return type_hash; \
	} \
	const char* name::Get_Name() \
	{ \
		return #name; \
	} \
	bool name::Is_OutOfBand() \
	{ \
		return out_of_band; \
	} \
	NetPacket* name::Constructor_##name() \
	{ \
		return new name(); \
	} \
	\
	void name::Serialize(Stream* stream) \
	{ 
#define PACKET_FIELD(type, name) \
		stream->Write<type>(name);
#define PACKET_SERIALIZABLE_FIELD(type, name) \
		name.Serialize(stream);
#define PACKET_ENUM_FIELD(type, name) \
		stream->Write<unsigned char>(static_cast<unsigned char>(name));
#define PACKET_ARRAY_FIELD(type, count, name) \
		{ \
			for (int i_ = 0; i_ < count; i_++) \
			{ \
				stream->Write<type>(name[i_]);	\
			} \
		}	
#define PACKET_VECTOR_FIELD(type, name) \
		{ \
			stream->Write<u16>((u16)name.size()); \
			for (std::vector<type>::iterator iter = name.begin(); iter != name.end(); iter++) \
			{ \
				type& value = *iter; \
				value.Serialize(stream); \
			} \
		}	
#define PACKET_BLOB_FIELD(name) \
		{ \
			stream->Write<u32>(name.Size()); \
			stream->WriteBuffer(name.Buffer(), 0, name.Size()); \
		}	

#define END_PACKET() \
	}

#define START_PACKET_ENUM(name)
#define PACKET_ENUM_ELEMENT(name)	
#define END_PACKET_ENUM()

#include PACKET_LIST_FILE

#undef START_PACKET
#undef END_PACKET
#undef PACKET_FIELD
#undef PACKET_VECTOR_FIELD
#undef PACKET_ARRAY_FIELD
#undef PACKET_ENUM_FIELD
#undef PACKET_SERIALIZABLE_FIELD
#undef START_PACKET_ENUM
#undef PACKET_ENUM_ELEMENT	
#undef END_PACKET_ENUM
#undef PACKET_BLOB_FIELD

#define START_PACKET(name, out_of_band) \
	bool name::Deserialize(Stream* stream) \
	{ 
#define PACKET_FIELD(type, name) \
		name = stream->Read<type>();
#define PACKET_ENUM_FIELD(type, name) \
		name = static_cast<type::Type>(stream->Read<unsigned char>());
#define PACKET_SERIALIZABLE_FIELD(type, name) \
		name.Deserialize(stream);
#define PACKET_ARRAY_FIELD(type, count, name) \
		{ \
			for (int i_ = 0; i_ < count; i_++) \
			{ \
				name[i_] = stream->Read<type>(); \
			} \
		}	
#define PACKET_VECTOR_FIELD(type, name) \
		{ \
			u16 c = stream->Read<u16>(); \
			for (u16 i_ = 0; i_ < c; i_++) \
			{ \
				type value; \
				value.Deserialize(stream); \
				name.push_back(value); \
			} \
		}	
#define PACKET_BLOB_FIELD(name) \
		{ \
			u32 size = stream->Read<u32>(); \
			name.Reserve(size, false); \
			stream->ReadBuffer(name.Buffer(), 0, size); \
		}	

#define END_PACKET() \
		return true; \
	}

#define START_PACKET_ENUM(name)
#define PACKET_ENUM_ELEMENT(name)	
#define END_PACKET_ENUM()

#include PACKET_LIST_FILE

#undef START_PACKET
#undef END_PACKET
#undef PACKET_FIELD
#undef PACKET_VECTOR_FIELD
#undef PACKET_ARRAY_FIELD
#undef PACKET_ENUM_FIELD
#undef PACKET_SERIALIZABLE_FIELD
#undef START_PACKET_ENUM
#undef PACKET_ENUM_ELEMENT	
#undef END_PACKET_ENUM
#undef PACKET_BLOB_FIELD

void PACKET_TYPE_REGISTER_FUNCTION()
{
#define START_PACKET(name, out_of_band) NetPacket::Register_Type_ID(StringHelper::Hash(#name), &name::Constructor_##name);
#define PACKET_FIELD(type, name) 
#define PACKET_SERIALIZABLE_FIELD(type, name) 
#define PACKET_ENUM_FIELD(type, name) 
#define PACKET_ARRAY_FIELD(type, count, name) 
#define PACKET_VECTOR_FIELD(type, name) 
#define END_PACKET() 
#define START_PACKET_ENUM(name)
#define PACKET_ENUM_ELEMENT(name)	
#define END_PACKET_ENUM()
#define PACKET_BLOB_FIELD(name)

#include PACKET_LIST_FILE

#undef START_PACKET
#undef END_PACKET
#undef PACKET_FIELD
#undef PACKET_VECTOR_FIELD
#undef PACKET_ARRAY_FIELD
#undef PACKET_ENUM_FIELD
#undef PACKET_SERIALIZABLE_FIELD
#undef START_PACKET_ENUM
#undef PACKET_ENUM_ELEMENT	
#undef END_PACKET_ENUM
#undef PACKET_BLOB_FIELD
}