// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Variant.h"
#include "Generic/Helper/StringHelper.h"

#include <math.h>
#include <cstdlib>

Variant::Variant()
	: m_type(DataType::Int)
	, m_int(0)
{
}

Variant::Variant(int value)
	: m_type(DataType::Int)
	, m_int(value)
{
}

Variant::Variant(float value)
	: m_type(DataType::Float)
	, m_float(value)
{
}

Variant::Variant(std::string value)
	: m_type(DataType::String)
	, m_string(value)
{
}

Variant::Variant(const Variant& value)
	: m_type(value.m_type)
{
	switch (value.m_type)
	{
	case DataType::Int:		m_int	= value.m_int;
	case DataType::Float:	m_float	= value.m_float;
	case DataType::String:	m_string = value.m_string;
	}
}
	
// Operators.
bool Variant::operator==(Variant& rhs) const
{
	switch (m_type)
	{
	case DataType::Int:		return (m_int == rhs.Coerce_Int());
	case DataType::Float:	return (m_float == rhs.Coerce_Float());
	case DataType::String:	return (m_string == rhs.Coerce_String());
	}
	return false;
}

bool Variant::operator==(int& rhs) const
{
	return (Coerce_Int() == rhs);
}

bool Variant::operator==(float& rhs) const
{
	return (Coerce_Float() == rhs);
}

bool Variant::operator==(std::string& rhs) const
{
	return (Coerce_String() == rhs);
}

bool Variant::operator!=(Variant& rhs) const
{
	return !operator==(rhs);
}

bool Variant::operator!=(int& rhs) const
{
	return !operator==(rhs);
}

bool Variant::operator!=(float& rhs) const
{
	return !operator==(rhs);
}

bool Variant::operator!=(std::string& rhs) const
{
	return !operator==(rhs);
}

Variant& Variant::operator=(const int& rhs)
{
	m_type = Variant::DataType::Int;
	m_int = rhs;

	return *this;
}

Variant& Variant::operator=(const float& rhs)
{
	m_type = Variant::DataType::Float;
	m_float = rhs;

	return *this;
}

Variant& Variant::operator=(const std::string& rhs)
{
	m_type = Variant::DataType::String;
	m_string = rhs;

	return *this;
}
	
Variant& Variant::operator=(const Variant& rhs)
{
	m_type = rhs.m_type;
	m_float = rhs.m_float;
	m_int = rhs.m_int;
	m_string = rhs.m_string;

	return *this;
}
	
int Variant::Coerce_Int() const
{
	switch (m_type)
	{
	case DataType::Int:		return m_int;
	case DataType::Float:	return (int)floorf(m_float);
	case DataType::String:	return atoi(m_string.c_str());
	}
	return 0;
}

float Variant::Coerce_Float() const
{
	switch (m_type)
	{
	case DataType::Int:		return (float)m_int;
	case DataType::Float:	return m_float;
	case DataType::String:	return (float)atof(m_string.c_str());
	}
	return 0.0f;
}

std::string Variant::Coerce_String() const
{
	switch (m_type)
	{
	case DataType::Int:		return StringHelper::To_String(m_int);
	case DataType::Float:	return StringHelper::To_String(m_float);
	case DataType::String:	return m_string;
	}
	return "";
}

// Getting/Setting values.
int Variant::Get_Int()
{
	if (m_type != DataType::Int)
	{
		m_int = Coerce_Int();
		m_type = DataType::Int;
	}
	return m_int;
}

float Variant::Get_Float()
{
	if (m_type != DataType::Float)
	{
		m_float = Coerce_Float();
		m_type = DataType::Float;
	}
	return m_float;
}

std::string& Variant::Get_String()
{
	if (m_type != DataType::String)
	{
		m_string = Coerce_String();
		m_type = DataType::String;
	}
	return m_string;
}

Variant::DataType::Type Variant::Get_Type()
{
	return m_type;
}

void Variant::Set_Int(std::string value)
{
	m_int = atoi(value.c_str());
	m_type = DataType::Int;
}

void Variant::Set_Float(std::string value)
{
	m_float = (float)atof(value.c_str());
	m_type = DataType::Float;
}

void Variant::Set_String(std::string value)
{
	m_string = value;
	m_type = DataType::String;
}
