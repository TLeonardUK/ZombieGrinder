/* *****************************************************************

		CSymbol.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CSYMBOL_H_
#define _CSYMBOL_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/VirtualMachine/CInstruction.h"
#include "XScript/VirtualMachine/CInstructionBuffer.h"

class CASTNode;
class CSymbol;
class CDataType;

// =================================================================
//	Stores debugging information about a symbol.
// =================================================================
struct CSymbolDebugInfo
{
	int Row;
	int Column;
	int FileNameIndex;
};

// =================================================================
//	Types of symbols that can exist.
// =================================================================
struct SymbolType
{
	enum Type
	{
		Root,
		Class,
		ClassField,
		ClassMethod,
		ClassState,
		LocalVariable,
		ClassProperty,
	};
};

// =================================================================
//	Struct stores information about interface information requried
//  to cast to and from interfaces.
// =================================================================
struct CInterfaceSymbol
{
public:
	CSymbol* InterfaceClass;
	int		 VFTableOffset;
};

// =================================================================
//	Struct stores information on an individual vm symbol.
// =================================================================
class CSymbol
{
public:
	SymbolType::Type		Type;
	CASTNode*				Node;

	String				Identifier;
	int						Index;

	bool					IsSerialized;
	int						SerializeVersion;

	union 
	{
		// Class data
		struct 
		{
			int FieldCount;
			int MethodCount;
			int VirtualMethodCount;
			int StaticCount;
			int StateCount;
		};

		// Class field data.
		struct 
		{
			CDataType*	FieldDataType;
			int			FieldOffset;
		};

		// Class method data.
		struct 
		{
			CDataType*	ReturnDataType;
			bool		IsVirtual;
			int			VFTableOffset;
			int			MethodTableOffset;
			int			LocalCount;
			int			ThisRegister;
		};

		// Local variable data.
		struct 
		{
			CDataType*	LocalDataType;
			int			LocalOffset;
		};

		// State data.
		struct
		{
			bool		IsDefault;
		};

		// Property data.
		struct 
		{
			CDataType*	PropertyDataType;
		};
	};

	std::vector<CSymbol*>			VFTable;

	std::vector<CSymbol*>			Children;
	CInstructionBuffer				Instructions;
	std::vector<CInterfaceSymbol>	Interfaces;

	bool FromInherit;

	CSymbol()
		: Type(SymbolType::Root)
		, Node(NULL)
		, IsVirtual(false)
		, FromInherit(false)
	{
	}
};

#endif