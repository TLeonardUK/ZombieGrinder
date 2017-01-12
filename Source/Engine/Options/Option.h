// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_OPTION_
#define _ENGINE_OPTIONS_OPTION_

#include <string>
#include <cstdlib>

#include "Engine/IO/BinaryStream.h"

struct OptionFlags
{
	enum Type
	{
		NONE				= 0,
		Saved				= 1,
		SetByCommandLine	= 2
	};
};

class Option
{
	MEMORY_ALLOCATOR(Option, "Engine");

private:
	const char*		  m_name;
	unsigned int	  m_name_hash;
	const char*		  m_description;
	OptionFlags::Type m_flags;

public:
	Option(const char* name, const char* description, OptionFlags::Type flags);

	const char* Get_Name() 
	{
		return m_name;
	}

	unsigned int Get_Name_Hash()
	{
		return m_name_hash;
	}

	const char* Get_Description() 
	{
		return m_description;
	}

	OptionFlags::Type Get_Flags()
	{
		return m_flags;
	}

	bool Flag_Set(OptionFlags::Type type)
	{
		return ((int)m_flags & (int)type) != 0;
	}
	
	void Add_Flag(OptionFlags::Type type)
	{
		m_flags = (OptionFlags::Type) ((int)m_flags | (int)type);
	}

	virtual bool Parse(const char* value) = 0;
	virtual std::string To_String() = 0;
	
	virtual unsigned int Get_Type_ID() = 0;

	virtual bool Serialize(BinaryStream& stream) = 0;
	virtual bool Deserialize(BinaryStream& stream, unsigned int version) = 0;


};

#endif

