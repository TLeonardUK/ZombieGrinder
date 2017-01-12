/* *****************************************************************

		CRuntime_Dictionary.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CRUNTIME_DICTIONARY_H_
#define _CRUNTIME_DICTIONARY_H_

class CVirtualMachine;

class CRuntime_Dictionary
{
public:
	static void Bind(CVirtualMachine* machine);

};

#endif