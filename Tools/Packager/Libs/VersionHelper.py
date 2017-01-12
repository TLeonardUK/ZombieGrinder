# -*- coding: utf-8 -*-

# ///////////////////////////////////////////////////////////////////////////////
# // Copyright (C) 2013 Tim Leonard
# ///////////////////////////////////////////////////////////////////////////////	
# 
# This script takes a single argument to a specially formatted version.h file. Each
# time the script is called (typically as a post-build event) the version number in
# the file will be incremented and any other build-information updated.
# 
# Syntax:
# 	update_version.py --file <path-to-version-h> --prefix <class-prefix>
# 
# ///////////////////////////////////////////////////////////////////////////////

import os
import datetime
import platform
from optparse import OptionParser

header_template = """///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013 Tim Leonard
///////////////////////////////////////////////////////////////////////////////	
// Generated by update_version.py
// {timestamp}
///////////////////////////////////////////////////////////////////////////////	
// WARNING: Do not modify this file in any way, it's format is assumed by
//			several python scripts used during building!
///////////////////////////////////////////////////////////////////////////////	
#pragma once

#ifndef _{PREFIX}_VERSION_H_
#define _{PREFIX}_VERSION_H_

class {PREFIX}AutoVersion
{
public:
	static const char* DAY;
	static const char* MONTH;
	static const char* YEAR;
	
	static const char* HOUR;
	static const char* MINUTE;
	static const char* SECOND;
	
	static const char* STATUS;
	static const char* STATUS_SHORT;

	static const long MAJOR;
	static const long MINOR;
	static const long BUILD;
	static const long REVISION;

	static const char* FULLVERSION_STRING;

	static const long TOTAL_BUILDS;
};

#endif // _VERSION_H_
"""

source_template = """///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013 Tim Leonard
///////////////////////////////////////////////////////////////////////////////	
// Generated by update_version.py
// {timestamp}
///////////////////////////////////////////////////////////////////////////////	
// WARNING: Do not modify this file in any way, it's format is assumed by
//			several python scripts used during building!
///////////////////////////////////////////////////////////////////////////////	
#include "{HEADER_FILE}"

const char* {PREFIX}AutoVersion::DAY = "{DAY}";
const char* {PREFIX}AutoVersion::MONTH = "{MONTH}";
const char* {PREFIX}AutoVersion::YEAR = "{YEAR}";
	
const char* {PREFIX}AutoVersion::HOUR = "{HOUR}";
const char* {PREFIX}AutoVersion::MINUTE = "{MINUTE}";
const char* {PREFIX}AutoVersion::SECOND = "{SECOND}";
	
const char* {PREFIX}AutoVersion::STATUS = "{STATUS}";
const char* {PREFIX}AutoVersion::STATUS_SHORT = "{STATUS_SHORT}";

const long {PREFIX}AutoVersion::MAJOR = {MAJOR};
const long {PREFIX}AutoVersion::MINOR = {MINOR};
const long {PREFIX}AutoVersion::BUILD = {BUILD};
const long {PREFIX}AutoVersion::REVISION = {REVISION};

const char* {PREFIX}AutoVersion::FULLVERSION_STRING = "{FULLVERSION_STRING}";

const long {PREFIX}AutoVersion::TOTAL_BUILDS = {TOTAL_BUILDS};
"""

# Parses all the values of an array out of the version file and returns an array of them.
def ValuesFromVersionFile(file, values):
	
	# Extract raw values from file.
	h = open(file, "r")
	
	for line in h:
		line = line.strip()
		
		if (line == ""):
			continue;
			
		prefix = "const";
		if (line.lower()[:len(prefix)] == prefix.lower()):
			line = line.split("::", 2);
			line = line[len(line) - 1]
			
			line = line.split("=")
			
			name = line[0].strip().strip("[]")
			value = line[1].strip().strip(";").strip('"')
			
			values[name] = value
			
	h.close()
	
	return values
		
# w00t, time to actually parse and update the version information.
def GetVersionNumber(source_file):

	values = {
		"TOTAL_BUILDS":	"0",
	};

	if (os.path.isfile(source_file) == True):
		values = ValuesFromVersionFile(source_file, values);
	
	return int(values['TOTAL_BUILDS'])
			
# Gets current changelist number.
def GetChangelistNumber():

	changelist = "1"

	if (os.path.isfile("Config/app.version") == True):
		file = open("Config/app.version", mode="r")
		changelist = file.read()
		file.close()
	
	return int(changelist)
	
# w00t, time to actually parse and update the version information.
def UpdateVersionInfo(header_file, source_file, resource_file, prefix, changelist_number):

	REVISIONS_PER_BUILD = 100

	now = datetime.datetime.now()

	values = {
		"timestamp":			now.strftime("%d-%m-%Y %H:%M"),
		"DAY":					str(now.day),
		"MONTH":				str(now.month),
		"YEAR":					str(now.year),
		"HOUR":					str(now.hour),
		"MINUTE":				str(now.minute),
		"SECOND":				str(now.second),
		"STATUS":				"",
		"STATUS_SHORT":			"",
		"MAJOR":				"1",
		"MINOR":				"0",
		"BUILD":				"0",
		"REVISION":				"0",
		"FULLVERSION_STRING":	"1.0.0.0",
		"TOTAL_BUILDS":			"0",
		"PREFIX":				prefix,
		"HEADER_FILE":			os.path.basename(header_file)
	};

	if (os.path.isfile(source_file) == True):
		values = ValuesFromVersionFile(source_file, values);
	
	# Update version information.
	values["TOTAL_BUILDS"] = str(changelist_number)
	values["REVISION"] = str(changelist_number % REVISIONS_PER_BUILD)
	values["BUILD"] = str(int(changelist_number / REVISIONS_PER_BUILD))
	values["FULLVERSION_STRING"] = values["MAJOR"]+"."+values["MINOR"]+"."+values["BUILD"]+"."+values["REVISION"]+values["STATUS_SHORT"]	
	values["DAY"]   = str(now.day)
	values["MONTH"] = str(now.month)
	values["YEAR"]  = str(now.year)
	values["HOUR"] = str(now.hour)
	values["MINUTE"]  = str(now.minute)
	values["SECOND"]  = str(now.second)

	# Replace the values in the template file and use that as the contents 
	# of our new version file.
	output = source_template
	for k, v in values.items():
		output = output.replace("{" + str(k) + "}", str(v))
		
	# Write out the version file output.
	file = open(source_file, "w")
	file.write(output)
	file.close()

	# Write out header file.	
	if (os.path.isfile(header_file) == False):
		
		output = header_template
		for k, v in values.items():
			output = output.replace("{" + str(k) + "}", str(v))
		
		file = open(header_file, "w")
		file.write(output)
		file.close()

	if (resource_file != ""):
		
		# Write out the resource file.
		file = open(resource_file + ".template", mode="r", encoding='utf-16')
		output = file.read()
		file.close()
		
		# Do replacements
		for k, v in values.items():
			output = output.replace("{" + str(k) + "}", str(v))
		
		# Write out the resource file.
		file = open(resource_file, mode="w", encoding='utf-16')
		file.write(output)
		file.close()
		

if __name__=="__main__":
    main();
