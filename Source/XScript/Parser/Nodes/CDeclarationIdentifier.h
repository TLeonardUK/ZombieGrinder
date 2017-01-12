/* *****************************************************************

		CDeclarationIdentifier.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CDECLARATIONIDENTIFIER_H_
#define _CDECLARATIONIDENTIFIER_H_

#include "Generic/Types/String.h"
#include "Generic/Helper/StringHelper.h"

// =================================================================
//	Holds a declaration identifier. Hashes identifier for faster
//  comparisons.
// =================================================================
struct CDeclarationIdentifier
{
private:
	int			m_hash;
	String		m_ident;

public:
	CDeclarationIdentifier()
		: m_ident("")
		, m_hash(0)
	{
	}

	CDeclarationIdentifier(const CDeclarationIdentifier& other)
		: m_ident(other.m_ident)
		, m_hash(other.m_hash)
	{
	}

	CDeclarationIdentifier(String ident)
		: m_ident(ident)
	{
		m_hash = StringHelper::Hash(ident.c_str());
	}

	bool operator ==(const CDeclarationIdentifier& other) const
	{
		return other.m_hash == m_hash && other.m_ident == m_ident; // Check hash then ident, shortcircuits. Need to do this as hashs have to high collisions.
	}

	bool operator !=(const CDeclarationIdentifier& other) const
	{
		return !operator==(other);
	}

	CDeclarationIdentifier& operator =(const String& other)
	{
		m_ident = other;
		m_hash = StringHelper::Hash(other.c_str());
		return *this;
	}

	String Get_String() const
	{
		return m_ident;
	}

	const char* c_str() const
	{
		return m_ident.c_str();
	}
};

#endif