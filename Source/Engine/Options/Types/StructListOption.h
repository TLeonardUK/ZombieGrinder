// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_STRUCT_LIST_OPTION_
#define _ENGINE_OPTIONS_STRUCT_LIST_OPTION_

#include "Engine/Options/Option.h"

#include "Generic/Helper/StringHelper.h"

#include <vector>

/*
struct Bindings
{
	enum Type
	{

	};
};

struct KeyBinding
{
	Bindings::Type a;
	Bindings::Type b;

	bool Parse(BasicParser& parser)
	{
		while (true)
		{
			if (parser.End_Of_Tokens())
			{
				return false;
			}

			if (parser.Look_Ahead_Token() == "}")
			{
				break;
			}
		}

		return true;
	}
	
	JSONElement To_JSON()
	{
		JSONDictioary element;
		element.Add_Item("input", "GUI_Right");
		element.Add_Item("output", "GUI_Right");
		return element;
	}
};

StructListOption<std::vector<KeyBinding>> key_bindings(std::vector<KeyBinding>(KeyBinding(a), KeyBinding(b)), "key_bindings", "Key Bindings!", OptionFlags::Saved);
*/

template <typename T>
class StructListOption : public Option
{
	MEMORY_ALLOCATOR(StructListOption, "Engine");

private:
	std::vector<T> m_data;

public:
	StructListOption(std::vector<T> default_value, const char* name, const char* description, OptionFlags::Type flags)
		: Option(name, description, flags)
		, m_data(default_value)
	{
	}

	bool Parse(const char* value)
	{
		return false;
	}

	std::string To_String()
	{
		return "<cannot_cast_to_string>";
	}

	std::vector<T>& operator*()
	{
		return m_data;
	}

	unsigned int Get_Type_ID()
	{
		static unsigned int id = StringHelper::Hash(StringHelper::Format("StructListOption<%s>", typeid(T).name()).c_str());
		return id;
	}

	bool Serialize(BinaryStream& stream)
	{
		stream.Write<unsigned int>(m_data.size());

		for (typename std::vector<T>::iterator iter = m_data.begin(); iter != m_data.end(); iter++)
		{
			T& data = *iter;
			if (!data.Serialize(stream))
			{
				return false;
			}
		}

		return true;
	}

	bool Deserialize(BinaryStream& stream, unsigned int version)
	{
		unsigned int size = stream.Read<unsigned int>();

		m_data.clear();

		for (unsigned int i = 0; i < size; i++)
		{
			T data;
			if (!data.Deserialize(stream, version))
			{
				return false;
			}
			m_data.push_back(data);
		}

		return true;
	}

};

#endif

