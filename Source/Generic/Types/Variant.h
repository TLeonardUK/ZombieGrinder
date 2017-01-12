// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_VARIANT_
#define _GENERIC_VARIANT_

#include <string>

// A variant type is much what you would expect. Its a data type that can store
// a value as int/float/string's, and convert values as and when required. 
//
// You shouldn't really use this unless you have a good reason (It's slow)! It's 
// only real purpose is when loading in values from config files who's data type 
// is not fixed until later.

struct Variant
{
//	MEMORY_ALLOCATOR(Variant, "Data Types");

public:
	struct DataType
	{
		enum Type
		{
			Int,
			Float,
			String
		};
	};


private:
	DataType::Type	m_type;

	union
	{
		int			m_int;
		float		m_float;
	};
	std::string		m_string;
	
public:
	Variant();
	Variant(int value);
	Variant(float value);
	Variant(std::string value);
	Variant(const Variant& value);
	
	// Operators.
	bool operator==(Variant& rhs) const;
	bool operator==(int& rhs) const;
	bool operator==(float& rhs) const;
	bool operator==(std::string& rhs) const;

	bool operator!=(Variant& rhs) const;
	bool operator!=(int& rhs) const;
	bool operator!=(float& rhs) const;
	bool operator!=(std::string& rhs) const;

	Variant& operator=(const int& rhs);
	Variant& operator=(const float& rhs);
	Variant& operator=(const std::string& rhs);
	Variant& operator=(const Variant& rhs);

	// Coersion between data types.
	int			Coerce_Int() const;
	float		Coerce_Float() const;
	std::string Coerce_String() const;

	// Getting/Setting values.
	int				Get_Int();
	float			Get_Float();
	std::string&	Get_String();
	DataType::Type	Get_Type();
	
	// Getting/Setting values.
	void			Set_Int(std::string value);
	void			Set_Float(std::string value);
	void			Set_String(std::string value);

};

#endif