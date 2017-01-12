// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Text/TagReplacer.h"

#include <algorithm>
#include <cstring>

std::vector<TagReplacer*> TagReplacer::m_replacers;

TagReplacer::TagReplacer()
{
	m_replacers.push_back(this);
}

TagReplacer::~TagReplacer()
{
	m_replacers.erase(std::find(m_replacers.begin(), m_replacers.end(), this));
}

std::string TagReplacer::Replace(const char* buffer)
{
	std::string result = buffer;
	for (std::vector<TagReplacer*>::iterator iter = m_replacers.begin(); iter != m_replacers.end(); iter++)
	{
		(*iter)->Do_Replace(result);
	}
	return result;
}

void TagReplacer::Inplace_Replace(const char** replacements, std::string& buffer)
{
	int index = 0;
	while (true)
	{
		const char* from = replacements[index++];
		const char* to	 = replacements[index++];

		if (from == NULL || to == NULL)
		{
			break;
		}
		
		int from_len = strlen(from);

		int offset = -1;
		while (true)
		{
			offset = buffer.find(from, offset + 1);
			if (offset == std::string::npos)
			{
				break;
			}
			
			buffer = buffer.replace(offset, from_len, to);
		}
	}
}