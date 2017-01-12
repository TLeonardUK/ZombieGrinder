/* *****************************************************************

		EvaluationResult.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _EVALUATIONRESULT_H_
#define _EVALUATIONRESULT_H_

#include "Generic/Types/String.h"
#include <vector>

// =================================================================
//	Stores a constant evaluation result.
// =================================================================
struct EvaluationResult
{
public:
	enum DataType
	{
		INT,
		FLOAT,
		STRING,
		BOOL
	};

private:
	DataType	m_type;

	bool		m_boolValue;
	int			m_intValue;
	float		m_floatValue;
	String m_stringValue;

public:
	EvaluationResult(bool value);
	EvaluationResult(int value);
	EvaluationResult(float value);
	EvaluationResult(String value);

	void		SetBool(bool value);
	void		SetInt(int value);
	void		SetFloat(float value);
	void		SetString(String value);

	bool		GetBool();
	int			GetInt();
	float		GetFloat();
	String GetString();

	DataType	GetType();
	void		SetType(DataType type);

};

#endif
