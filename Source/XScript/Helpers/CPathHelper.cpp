/* *****************************************************************

		CPathHelper.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include <stdio.h>
#include "Generic/Types/String.h"
#include <assert.h>
#include <algorithm>

#ifdef _WIN32

#include <Windows.h>
#include <direct.h>

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#elif defined(__linux__) || defined(__APPLE__)

#include <unistd.h> 
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>
#include <dirent.h>
#include <cstring>

#endif

#include "CPathHelper.h"
#include "CStringHelper.h"

// =================================================================
//	Returns true if the path is a directory.
// =================================================================
bool CPathHelper::LoadFile(String path, String& result)
{
	FILE* file = fopen(path.c_str(), "rb");
	if (file == NULL)
	{
		return false;
	}

	// Figure out max size.
	unsigned int position = ftell(file);
	fseek(file, 0, SEEK_END);
	unsigned int length = ftell(file);
	fseek(file, position, SEEK_SET);

	// Buffer for output.
	char* output = new char[length + 1];
	int offset = 0;

	while (!feof(file))
	{
		int c = fgetc(file);
		if (c != EOF)
		{
			// Patch up \r\n newlines into simply \n
			if (c == '\r')
			{
				int c2 = fgetc(file);
				if (c2 == '\n')
				{
					output[offset++] = c2;
				}
				else
				{					
					output[offset++] = c;
					if (c2 != EOF)
					{
						output[offset++] = c2;
					}
					else
					{
						break;
					}
				}
			}
			else
			{				
				output[offset++] = c;
			}
		}
	}

	//DBG_ASSERT(offset <= (int)length);

	output[offset] = '\0';
	result = String(output, offset);
	SAFE_DELETE_ARRAY(output);

	fclose(file);
	return true;
}

// =================================================================
//	Returns true if the path is a directory.
// =================================================================
bool CPathHelper::SaveFile(String path, String output)
{
	FILE* file = fopen(path.c_str(), "wb");
	if (file == NULL)
	{
		return false;
	}

	unsigned int counter = 0;
	while (counter < output.size())
	{
		counter += fwrite(output.c_str() + counter, 1, output.size() - counter, file);
	}

	fclose(file);
	return true;
}

// =================================================================
//	Returns true if the path is a directory.
// =================================================================
bool CPathHelper::IsDirectory(String value)
{
#ifdef _WIN32
	DWORD flags = GetFileAttributesA(value.c_str());
	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return false;
	}
	return true;
#elif defined(__linux__) || defined(__APPLE__)
	if (access(value.c_str(), 0) != 0)
	{
		return false;
	}

	struct stat status;
	stat(value.c_str(), &status);

	if ((status.st_mode & S_IFDIR) == 0)
	{
		return false;
	}	

	return true;
#else
	assert(0);
#endif	
}

// =================================================================
//	Returns true if the path is a file.
// =================================================================
bool CPathHelper::IsFile(String value)
{
#ifdef _WIN32
	DWORD flags = GetFileAttributesA(value.c_str());
	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		return false;
	}

	return true;
#elif defined(__linux__) || defined(__APPLE__)
	if (access(value.c_str(), 0) != 0)
	{
		return false;
	}

	struct stat status;
	stat(value.c_str(), &status);
	
	if ((status.st_mode & S_IFDIR) != 0)
	{
		return false;
	}	

	return true;
#else
	assert(0);
#endif	
}

// =================================================================
//	Returns true if the path is relative.
// =================================================================
bool CPathHelper::IsRelative(String value)
{
#ifdef _WIN32
	if (value.size() <= 2)
	{
		return true;
	}

	if (value.at(1) != ':')
	{
		return true;
	}

	return false;
#elif defined(__linux__) || defined(__APPLE__) 
	if (value.size() <= 1)
	{
		return true;
	}

	if (value.at(0) != '/')
	{
		return true;
	}

	return false;
#else
	assert(0);
#endif	
}

// =================================================================
//	Returns the current path.
// =================================================================
String	CPathHelper::CurrentPath()
{
#ifdef _WIN32
	char buffer[512];
	GetCurrentDirectoryA(512, buffer);
	return String(buffer);
#elif defined(__linux__) || defined(__APPLE__) 
	char buffer[512];
	getcwd(buffer, 512);
	return String(buffer);
#else
	assert(0);
#endif	
}

// =================================================================
//	Returns the correct case of a given path.
// =================================================================
String CPathHelper::RealPathCase(String value)
{
	std::vector<String> crackedPath = CStringHelper::Split(value, '/');
	String path = "";

	for (unsigned int i = 0; i < crackedPath.size(); i++)
	{
		String crack = crackedPath.at(i);
		if (path != "")
		{
			std::vector<String> files = ListAll(path + "/");
			for (std::vector<String>::iterator iter = files.begin(); iter != files.end(); iter++)
			{
				String lower1 = CStringHelper::ToLower(*iter);
				String lower2 = CStringHelper::ToLower(crack);
				if (lower1 == lower2)
				{
					crack = *iter;
					break;
				}
			}
		}

		if (path != "")
		{
			path += "/";
		}
		path += crack;
	}

	return path;
}

// =================================================================
//	Standardizes the path.
// =================================================================
String	CPathHelper::CleanPath(String value)
{
	value = CStringHelper::Replace(value, "\\", "/"); // Turn backslashes into forward slashes.
	value = CStringHelper::Replace(value, "//", "/"); // Remove duplicate path seperators.
//	value = RealPathCase(value);					  // Convert path to correct case.
	return value;
}

// =================================================================
//	Strips the directory of a file path.
// =================================================================
String	CPathHelper::StripDirectory(String value)
{
	int offset = value.find_last_of("/\\");
	if (offset < 0)
	{
		return value;
	}
	else
	{
		return value.substr(offset + 1);
	}
}

// =================================================================
//	Strips the filename of a file path.
// =================================================================
String	CPathHelper::StripFilename(String value)
{
	int offset = value.find_last_of("/\\");
	if (offset < 0)
	{
		return value;
	}
	else
	{
		return value.substr(0, offset);
	}
}

// =================================================================
//	Strips the extension of a file path.
// =================================================================
String	CPathHelper::StripExtension(String value)
{
	int offset = value.find_last_of(".");
	if (offset < 0)
	{
		return value;
	}
	else
	{
		return value.substr(0, offset);
	}
}

// =================================================================
//	Extracts the extension of a file path.
// =================================================================
String	CPathHelper::ExtractExtension(String value)
{
	int offset = value.find_last_of(".");
	if (offset < 0)
	{
		return "";
	}
	else
	{
		return value.substr(offset + 1);
	}
}

// =================================================================
//	Copies a file from one to another.
// =================================================================
void CPathHelper::CopyFileTo(String src, String dst)
{
	src = CleanPath(src);
	dst = CleanPath(dst);
#ifdef _WIN32
	CopyFileA(src.c_str(), dst.c_str(), false);
#elif defined(__linux__) || defined(__APPLE__) 
	if (IsFile(dst) == true)
	{
		unlink(dst.c_str());
	}

	int source_fd = open(src.c_str(), O_RDWR);
	int dest_fd   = open(dst.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0777);  

	struct stat stats;
	fstat(source_fd, &stats);
	
    char buf[1024];
    size_t size;

	while ((size = read(source_fd, buf, 1024)) > 0) 
	{
        write(dest_fd, buf, size);
    }

	close(source_fd);
	close(dest_fd);
#else
	assert(0);
#endif	
}

// =================================================================
//	Creates a new directory.
// =================================================================
void CPathHelper::MakeDirectory(String value)
{
	std::vector<String> crackedPath = CStringHelper::Split(value, '/');
	for (unsigned int i = 0; i < crackedPath.size(); i++)
	{
		String path = "";
		for (unsigned int k = 0; k <= i; k++)
		{
			path += crackedPath.at(k);
			if (k + 1 <= i)
			{
				path += "/";
			}
		}

		path = CleanPath(path);
		if (!IsDirectory(path))
		{
#ifdef _WIN32
			CreateDirectoryA(path.c_str(), NULL);
#elif defined(__linux__) || defined(__APPLE__)
			mkdir(path.c_str(), 0777);
#else
			assert(0);
#endif	
		}
	}
}

// =================================================================
//	Removes . and .. entries in a path and appends the current
//  directory if its relative.
// =================================================================
String	CPathHelper::GetAbsolutePath(String value)
{
	// Add current directory.
	if (IsRelative(value) == true)
	{
		value = CurrentPath() + "/" + value;
	}

	value = CleanPath(value);
	
	// Strip out all .. and . references.
	std::vector<String> crackedPath = CStringHelper::Split(value, '/');
	String				 finalPath   = "";
	int						 skip_count = 0;

	for (int i = crackedPath.size() - 1; i >= 0; i--)
	{
		String part = crackedPath.at(i);

		if (part == "..")
		{
			skip_count++;
		}
		else if (part == ".")
		{
			continue;
		}
		else
		{
			if (skip_count > 0)
			{
				skip_count--;
				continue;
			}

			if (finalPath == "")
			{
				finalPath = part;
			}
			else
			{
				finalPath = part + "/" + finalPath;
			}
		}
	}
	
	if (value[value.size() - 1] == '/')
	{
		finalPath += "/";
	}

	return finalPath;
}

// =================================================================
//	Gets the relative path from one file to another.
// =================================================================
String	CPathHelper::GetRelativePath(String path, String relative)
{
	path = GetAbsolutePath(path);
	relative = GetAbsolutePath(relative);

	String path_file     = CPathHelper::StripDirectory(path);
	String relative_file = CPathHelper::StripDirectory(relative);

	String path_dir     = CPathHelper::StripFilename(path) + "/";
	String relative_dir = CPathHelper::StripFilename(relative) + "/";
	
	int min_size = path_dir.size() < relative_dir.size() ? path_dir.size() : relative_dir.size();
	int same_path_offset = 0;
	for (int i = 0; i < min_size; i++)
	{
		if (path_dir[i] == relative_dir[i])
		{
			same_path_offset++;
		}
	}

	String same_path_dir     = CStringHelper::StripChar(same_path_offset <= 0 ? path_dir	 : path_dir.substr(same_path_offset), '/');
	String same_relative_dir = CStringHelper::StripChar(same_path_offset <= 0 ? relative_dir : relative_dir.substr(same_path_offset), '/');

	std::vector<String> cracked_path		   = CStringHelper::Split(same_path_dir, '/');
	std::vector<String> cracked_relative_path = CStringHelper::Split(same_relative_dir, '/');

	String result = "";
	if (same_path_dir.size() > same_relative_dir.size())
	{
		result = same_path_dir + "/" + path_file;
	}
	else if (same_relative_dir.size() > same_path_dir.size())
	{
		for (unsigned int i = 0; i < cracked_relative_path.size(); i++)
		{
			result += "../";
		}
		result += path_file;
	}
	else
	{
		result = path_file;
	}

	return result;
}

// =================================================================
//	List all files in a directory recursively.
// =================================================================
std::vector<String> CPathHelper::ListRecursiveFiles(String path, String extension)
{
	std::vector<String> result;
	
	std::vector<String> files = ListFiles(path);
	std::vector<String> dirs  = ListDirs(path);

	std::vector<String> abs_files;
	for (std::vector<String>::iterator iter = files.begin(); iter != files.end(); iter++)
	{
		String file_path = CPathHelper::CleanPath(path + "/" + (*iter));
		
		if (extension != "")
		{
			if (CPathHelper::ExtractExtension(file_path) != extension)
			{
				continue;
			}
		}
		
		abs_files.push_back(file_path);
		result.push_back(file_path);
	}
	
	std::vector<String> abs_dirs;
	for (std::vector<String>::iterator iter = dirs.begin(); iter != dirs.end(); iter++)
	{
		String dir_path = CPathHelper::CleanPath(path + "/" + (*iter));
		abs_dirs.push_back(dir_path);

		std::vector<String> rel_abs_files = ListRecursiveFiles(dir_path, extension);
		for (std::vector<String>::iterator iter2 = rel_abs_files.begin(); iter2 != rel_abs_files.end(); iter2++)
		{
			result.push_back(*iter2);
		}
	}

	return result;
}

// =================================================================
//	List all files in a directory.
// =================================================================
std::vector<String> CPathHelper::ListFiles(String value)
{
	std::vector<String>	files;

	value = CleanPath(value);
	if (value.at(value.size() - 1) != '/')
	{
		value += "/";
	}
	
#ifdef _WIN32
	WIN32_FIND_DATAA			data;
	HANDLE						handle;

	handle = FindFirstFileA((value + "*").c_str(), &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		while (true)
		{
			String full_path = value + data.cFileName;
			if (IsFile(full_path))
			{
				files.push_back(data.cFileName);
			}

			if (FindNextFileA(handle, &data) == 0)
			{
				break;
			}
		}
		FindClose(handle);
	}
#elif defined(__linux__) || defined(__APPLE__) 
	DIR* dir;
	struct dirent* file;
	
	dir = opendir(value.c_str());
	if (dir != NULL)
	{
		while (true)
		{
			file = readdir(dir);
			if (file == NULL)
			{
				break;
			}
			
			String full_path = value + file->d_name;
			if (IsFile(full_path))
			{
				files.push_back(file->d_name);
			}			
		}
		closedir(dir);
	}	
#else
	assert(0);
#endif	

	return files;
}

// =================================================================
//	List all dirs in a directory.
// =================================================================
std::vector<String> CPathHelper::ListDirs(String value)
{
	std::vector<String>	files;
	value = CleanPath(value);

	if (value.at(value.size() - 1) != '/')
	{
		value += "/";
	}

#ifdef _WIN32
	WIN32_FIND_DATAA			data;
	HANDLE						handle;

	handle = FindFirstFileA((value + "*").c_str(), &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		while (true)
		{
			String full_path = value + data.cFileName;
			if (IsDirectory(full_path) == true &&
				strcmp(data.cFileName, ".") != 0 && 
				strcmp(data.cFileName, "..") != 0)
			{
				files.push_back(data.cFileName);
			}

			if (FindNextFileA(handle, &data) == 0)
			{
				break;
			}
		}
		FindClose(handle);
	}
#elif defined(__linux__) || defined(__APPLE__)
	DIR* dir;
	struct dirent* file;
	
	dir = opendir(value.c_str());
	if (dir != NULL)
	{
		while (true)
		{
			file = readdir(dir);
			if (file == NULL)
			{
				break;
			}
			
			String full_path = value + file->d_name;
			if (IsDirectory(full_path) == true &&
				strcmp(file->d_name, ".") != 0 && 
				strcmp(file->d_name, "..") != 0)
			{
				files.push_back(file->d_name);
			}			
		}
		closedir(dir);
	}	
#else
	assert(0);
#endif	

	return files;
}

// =================================================================
//	List all files and dirs in a directory.
// =================================================================
std::vector<String> CPathHelper::ListAll(String value)
{
	std::vector<String>	files;

#ifdef _WIN32
	WIN32_FIND_DATAA			data;
	HANDLE						handle;
	//value = CleanPath(value);
	//if (value.at(value.size() - 1) != '/')
	//{
	//	value += "/";
	//}

	handle = FindFirstFileA((value + "*").c_str(), &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		while (true)
		{
			String full_path = value + data.cFileName;
			if (strcmp(data.cFileName, ".") != 0 && 
				strcmp(data.cFileName, "..") != 0)
			{
				files.push_back(data.cFileName);
			}

			if (FindNextFileA(handle, &data) == 0)
			{
				break;
			}
		}
		FindClose(handle);
	}
#elif defined(__linux__) || defined(__APPLE__)
	DIR* dir;
	struct dirent* file;
	
	dir = opendir(value.c_str());
	if (dir != NULL)
	{
		while (true)
		{
			file = readdir(dir);
			if (file == NULL)
			{
				break;
			}
			
			String full_path = value + file->d_name;
			if (strcmp(file->d_name, ".") != 0 && 
				strcmp(file->d_name, "..") != 0)
			{
				files.push_back(file->d_name);
			}			
		}
		closedir(dir);
	}	
#else
	assert(0);
#endif	

	return files;
}