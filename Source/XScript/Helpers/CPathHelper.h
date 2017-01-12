/* *****************************************************************

		CPathHelper.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CPATHHELPER_H_
#define _CPATHHELPER_H_

#include "Generic/Types/String.h"
#include <vector>

// =================================================================
//	Class contains several path / IO related helper functions.
// =================================================================
class CPathHelper
{
private:
	CPathHelper();

public:
	static bool						IsDirectory		(String value);
	static bool						IsFile			(String value);
	static bool						IsRelative		(String value);

	static bool						LoadFile		(String path, String& output);
	static bool						SaveFile		(String path, String output);

	static String				GetRelativePath	(String path, String relative);

	static String				RealPathCase	(String value);
	static String				CleanPath		(String value);
	static String				StripDirectory	(String value);
	static String				StripFilename	(String value);
	static String				ExtractExtension(String value);
	static String				StripExtension	(String value);
	static String				GetAbsolutePath	(String value);
	
	static String				CurrentPath		();

	static void						MakeDirectory	(String value);
	static void						CopyFileTo		(String from, String to);

	static std::vector<String> ListFiles			(String value);
	static std::vector<String> ListDirs			(String value);
	static std::vector<String> ListAll				(String value);
	
	static std::vector<String> ListRecursiveFiles	(String value, String extension="");


};

#endif