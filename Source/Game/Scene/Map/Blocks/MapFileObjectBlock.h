// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_MAP_MAPFILE_OBJECT_BLOCK_
#define _GAME_SCENE_MAP_MAPFILE_OBJECT_BLOCK_

#include "Engine/Scene/Map/Blocks/MapFileBlock.h"

#include "Engine/Scene/Map/Map.h"

#include "Generic/Types/DataBuffer.h"

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class SoundHandle;
class TextureHandle;
class AtlasHandle;

class ScriptedActor;

/*
struct ObjectValueType
{
	enum Type
	{
		Int,
		Float,
		String
	};
};

struct ObjectValue
{
	ObjectValueType::Type type;

	int			int_value;
	float		float_value;
	std::string string_value;
};

struct ObjectProperty
{
	bool		IsScope;

	std::string Name;
	ObjectValue Value;

	std::vector<ObjectProperty> Children;

	ObjectProperty()
	{
	}

	ObjectProperty(bool isScope, std::string name)
		: IsScope(isScope)
		, Name(name)
	{
	}

	ObjectProperty(bool isScope, std::string name, int value)
		: IsScope(isScope)
		, Name(name)
	{
		Value.type = ObjectValueType::Int;
		Value.int_value = value;
	}

	ObjectProperty(bool isScope, std::string name, float value)
		: IsScope(isScope)
		, Name(name)
	{
		Value.type = ObjectValueType::Float;
		Value.float_value = value;
	}
	
	ObjectProperty(bool isScope, std::string name, std::string value)
		: IsScope(isScope)
		, Name(name)
	{
		Value.type = ObjectValueType::String;
		Value.string_value = value;
	}
};
*/

struct MapFileObjectBlock : public MapFileBlock 
{
public:
	std::string					Class_Name;
	DataBuffer					Serialized_Data;
	int							Serialized_Data_Version;

	/*
	Vector3						Position;
	Vector3						Rotation;
	Vector3						Scale;
	int							Layer;
	float						Depth_Bias;
	Rect2D						Bounding_Box;

	std::vector<ObjectProperty>	Properties;
	*/

public:
	MapFileObjectBlock();
	~MapFileObjectBlock();

	//void Encode_Properties(Stream* stream, std::vector<ObjectProperty>& properties);
	//void Decode_Properties(Stream* stream, std::vector<ObjectProperty>& properties);

	//void Apply_Properties(CVirtualMachine* vm, CVMObjectHandle obj, std::vector<ObjectProperty>& properties);

	//void New_Property(bool is_scope, ObjectProperty& prop);

	bool Decode(MapVersion::Type version, Stream* stream);
	bool Encode(MapVersion::Type version, Stream* stream);
	u32  Get_ID();

	ScriptedActor* Create_Instance();

};

#endif

