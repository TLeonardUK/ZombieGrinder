/* *****************************************************************

		EvaluationResult.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/EvaluationResult.h"

#include "XScript/Helpers/CStringHelper.h"

// =================================================================
//	Constructors.
// =================================================================
EvaluationResult::EvaluationResult(bool value) :
	m_type(BOOL),
	m_boolValue(value)
{
}
EvaluationResult::EvaluationResult(int value) :
	m_type(INT),
	m_intValue(value)
{
}
EvaluationResult::EvaluationResult(float value) :
	m_type(FLOAT),
	m_floatValue(value)
{
}
EvaluationResult::EvaluationResult(String value) :
	m_type(STRING),
	m_stringValue(value)
{
}

// =================================================================
//	Setters.
// =================================================================
void EvaluationResult::SetBool(bool value)
{
	m_type = BOOL;
	m_boolValue = value;
}
void EvaluationResult::SetInt(int value)
{
	m_type = INT;
	m_intValue = value;
}
void EvaluationResult::SetFloat(float value)
{
	m_type = FLOAT;
	m_floatValue = value;
}
void EvaluationResult::SetString(String value)
{
	m_type = STRING;
	m_stringValue = value;
}

// =================================================================
//	Getters.
// =================================================================
bool EvaluationResult::GetBool()
{
	if (m_type != BOOL)
	{
		SetType(BOOL);
	}
	return m_boolValue;
}
int EvaluationResult::GetInt()
{
	if (m_type != INT)
	{
		SetType(INT);
	}
	return m_intValue;
}
float EvaluationResult::GetFloat()
{
	if (m_type != FLOAT)
	{
		SetType(FLOAT);
	}
	return m_floatValue;
}
String EvaluationResult::GetString()
{
	if (m_type != STRING)
	{
		SetType(STRING);
	}
	return m_stringValue;
}

// =================================================================
//	Conversion.
// =================================================================
EvaluationResult::DataType EvaluationResult::GetType()
{
	return m_type;
}
void EvaluationResult::SetType(EvaluationResult::DataType type)
{
	if (type == m_type)
	{
		return;
	}

	switch (m_type)
	{
		case BOOL:
		{
			switch (type)
			{
				case INT:
				{
					m_intValue = m_boolValue ? 1 : 0;
					break;
				}
				case FLOAT:
				{
					m_floatValue = (m_boolValue ? 1.0f : 0.0f);
					break;
				}
				case STRING:
				{
					m_stringValue = m_boolValue ? "1" : "0";
					break;
				}
			}
			break;
		}
		case INT:
		{
			switch (type)
			{
				case BOOL:
				{					
					m_boolValue = (m_intValue != 0);
					break;
				}
				case FLOAT:
				{
					m_floatValue = (float)m_intValue;
					break;
				}
				case STRING:
				{
					m_stringValue = CStringHelper::ToString(m_intValue);
					break;
				}
			}
			break;
		}
		case FLOAT:
		{
			switch (type)
			{
				case BOOL:
				{
					m_boolValue = (m_floatValue != 0);
					break;
				}
				case INT:
				{
					m_intValue = (int)m_floatValue;
					break;
				}
				case STRING:
				{
					m_stringValue = CStringHelper::ToString(m_floatValue);
					break;
				}
			}
			break;
		}
		case STRING:
		{
			switch (type)
			{
				case BOOL:
				{
					m_boolValue = (m_stringValue != "" && m_stringValue != "0");
					break;
				}
				case INT:
				{
					m_intValue = CStringHelper::ToInt(m_stringValue);
					break;
				}
				case FLOAT:
				{
					m_floatValue = CStringHelper::ToFloat(m_stringValue);
					break;
				}
			}
			break;
		}
	}

	m_type = type;
}