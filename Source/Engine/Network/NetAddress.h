// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NETWORK_NET_ADDRESS_
#define _ENGINE_NETWORK_NET_ADDRESS_

#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>

struct NetAddress
{
	MEMORY_ALLOCATOR(NetAddress, "Network");

private:
	u32 m_ip;
	s16 m_port;

public:
	NetAddress()
		: m_ip(0)
		, m_port(0)
	{
	}

	NetAddress(s32 ip, s16 port)
		: m_ip(ip)
		, m_port(port)
	{
	}

	s32 Get_IP()
	{
		return m_ip;
	}

	s16 Get_Port()
	{
		return m_port;
	}

	std::string To_String()
	{
		char buffer[32];
		sprintf(buffer, "%u.%u.%u.%u:%i", (m_ip >> 24) & 0xFF, (m_ip >> 16) & 0xFF, (m_ip >> 8) & 0xFF, (m_ip) & 0xFF, m_port);
		return buffer;
	}

	static bool From_String(const char* str, NetAddress& output)
	{
		static const int max_buffer_size = 16;

		char buffer[max_buffer_size];
		int buffer_index = 0;

		unsigned int chars[4];
		memset(chars, 0, sizeof(chars));

		int char_index	= 0;
		int port		= 0;

		int len			= strlen(str);
		int index		= 0;

		int period_count = 0;

		// Step through each index of the string and
		// parse out each value using . and : as deliminators.
		while (index < len)
		{
			char c = str[index++];
			if (c == '.' || c == ':')
			{
				if (c == '.')
				{
					if (period_count++ >= 3)
					{
						return false;
					}					
				}

				if (buffer_index <= 0 || buffer_index > 3 || char_index >= 4)
				{
					return false;
				}

				buffer[buffer_index] = '\0';
				chars[char_index++] = (unsigned int)atoi(buffer);
				buffer_index = 0;
			}
			else
			{
				if (buffer_index >= max_buffer_size)
				{
					return false;
				}
				buffer[buffer_index++] = c;
			}
		}
		
		// Deal with remainder of buffer, either a port number or the last char.
		if (buffer_index > 0)
		{
			if (char_index < 4)
			{
				buffer[buffer_index] = '\0';
				chars[char_index++] = (unsigned int)atoi(buffer);
			}
			else
			{
				buffer[buffer_index] = '\0';
				port = (unsigned int)atoi(buffer);
			}
		}

		unsigned int a = (chars[0] << 24);
		unsigned int b = (chars[1] << 16);
		unsigned int c = (chars[2] << 8);
		unsigned int d = (chars[3]);

		output.m_port = port;
		output.m_ip   = a | b | c | d;
		return true;
	}

};

#endif

