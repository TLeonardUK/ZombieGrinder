/* *****************************************************************

		CSemanter.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "Generic/Types/String.h"
#include <stdio.h>
#include <assert.h>

#include "XScript/Compiler/CCompiler.h"
#include "XScript/Semanter/CSemanter.h"
#include "XScript/Helpers/CStringHelper.h"
#include "XScript/Helpers/CPathHelper.h"

#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CPackageASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CAliasASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CLiteralExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CNewExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/Helper/CNullDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CVoidDataType.h"

// Global variable used for log tracing.
#ifndef _NDEBUG
int g_semant_trace_index = 0;
#endif

// =================================================================
//	Processes input and performs the actions requested.
// =================================================================
bool CSemanter::Process(CTranslationUnit* context)
{	
	m_context = context;
	m_internal_var_counter = 0;
	
	//context->Info("Semantic Analysis ...");
//	context->GetASTRoot()->Prepare(this);

	//context->Info("Semantic Analysis ...");
	context->GetASTRoot()->Semant(this);

	//context->Info("Semantic Finalization ...");
	context->GetASTRoot()->Finalize(this);

	return true;
}

// =================================================================
//	Gets a new internal variable name.
// =================================================================
String CSemanter::NewInternalVariableName()
{
	return "lsInternal_s__" + CStringHelper::ToString(m_internal_var_counter++);
}

// =================================================================
//	Constructs a default assignment expression that can be applyed
//	to a variable to initialize it if an initialization expression
//	is not provided.
// =================================================================	
CExpressionASTNode*	CSemanter::ConstructDefaultAssignmentExpr(CASTNode* parent, CToken& token, CDataType* type)
{
	CClassASTNode* type_class = type->GetClass(this);

	// Structs need a 'new X' constructor.
	if (type_class != NULL && type_class->IsStruct == true)
	{
		CNewExpressionASTNode* new_expr = new CNewExpressionASTNode(NULL, token);
		new_expr->DataType = type;
		new_expr->IsArray = false;

		CExpressionASTNode* expr = new CExpressionASTNode(parent, token);
		expr->LeftValue = new_expr;
		expr->ExpressionResultType = type;
		expr->AddChild(new_expr);

		return expr;
	}

	// Normal types.
	else
	{
		CLiteralExpressionASTNode* lit = NULL;
		if (dynamic_cast<CVoidDataType*>(type) != NULL)
		{
			return NULL;
		}
		else if (dynamic_cast<CBoolDataType*>(type) != NULL)
		{
			lit =  new CLiteralExpressionASTNode(NULL, token, type, "false");
		}
		else if (dynamic_cast<CIntDataType*>(type) != NULL)
		{
			lit =  new CLiteralExpressionASTNode(NULL, token, type, "0");
		}
		else if (dynamic_cast<CFloatDataType*>(type) != NULL)
		{
			lit =  new CLiteralExpressionASTNode(NULL, token, type, "0.0");
		}
		else if (dynamic_cast<CStringDataType*>(type) != NULL)
		{
			lit =  new CLiteralExpressionASTNode(NULL, token, type, "");
		}
		else
		{
			lit =  new CLiteralExpressionASTNode(NULL, token, new CNullDataType(token), "");
		}

		CExpressionASTNode* expr = new CExpressionASTNode(parent, token);
		expr->LeftValue = lit;
		expr->ExpressionResultType = type;
		expr->AddChild(lit);

		return expr;
	}
}

// =================================================================
//	Gets a unique mangled identifier.
// =================================================================
/*
String	CSemanter::GetMangled(String mangled)
{
	String originalMangled = mangled;

	int index = 1;
	while (true)
	{
		bool found = false;

		for (std::vector<String>::iterator  iter = m_mangled.begin(); iter != m_mangled.end(); iter++)
		{
			if (*iter == mangled)
			{
				mangled = originalMangled + "_" + CStringHelper::ToString(index++);
				found = true;
				break;
			}
		}

		if (found == false)
		{
			break;
		}
	}
	m_mangled.push_back(mangled);
	
	return mangled;
}
*/

// =================================================================
//	Gets the context we are semanting for.
// =================================================================
CTranslationUnit* CSemanter::GetContext()
{
	return m_context;
}

// =================================================================
//	Check for duplicate identifiers.
// =================================================================
CDataType* CSemanter::BalanceDataTypes(CDataType* lvalue, CDataType* rvalue, CToken& token)
{
	// If either are string result is string.
	if (dynamic_cast<CStringDataType*>(lvalue) != NULL) 
	{
		return lvalue;	
	}
	if (dynamic_cast<CStringDataType*>(rvalue) != NULL) 
	{
		return rvalue;	
	}

	// If either are float result is float.
	if (dynamic_cast<CFloatDataType*>(lvalue) != NULL) 
	{
		return lvalue;	
	}
	if (dynamic_cast<CFloatDataType*>(rvalue) != NULL) 
	{
		return rvalue;	
	}

	// If either are int result is int.
	if (dynamic_cast<CIntDataType*>(lvalue) != NULL) 
	{
		return lvalue;	
	}
	if (dynamic_cast<CIntDataType*>(rvalue) != NULL) 
	{
		return rvalue;	
	}

//	LVALUE = CASTNode
//	RVALUE = CAliasASTNode

	// Check which values we can cast too.
	if (rvalue->CanCastTo(this, lvalue))
	{
		return lvalue;
	}
	if (lvalue->CanCastTo(this, rvalue))
	{
		return rvalue;
	}

	// o_o
	m_context->FatalError(CStringHelper::FormatString("Unable to implicitly convert between data-types '%s' and '%s'", lvalue->ToString().c_str(), rvalue->ToString().c_str()), token);

	return NULL;
}