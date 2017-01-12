// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NET_WEB_REQUEST_
#define _ENGINE_NET_WEB_REQUEST_

#include "Generic/Types/DataBuffer.h"

#include "Generic/Threads/Thread.h"

#include <mongoose/include/CivetServer.h>
#include <mongoose/include/civetweb.h>

#include <string>
#include <vector>

struct NetWebRequestParameter
{
	std::string name;
	DataBuffer	buffer;
	bool		is_file;
};

struct NetWebRequestType
{
	enum Type
	{
		Post,
		Get
	};
};

class NetWebRequest
{
	MEMORY_ALLOCATOR(NetWebRequest, "Network");

private:
	std::vector<NetWebRequestParameter> m_body_params;
	std::vector<NetWebRequestParameter> m_uri_params;

	NetWebRequestType::Type m_type;

	std::string m_host;
	std::string m_uri;
	int m_port;
	bool m_use_ssl;

	bool m_active;
	bool m_was_success;

	float m_progress;

	bool m_shutting_down;
	Thread* m_thread;

	DataBuffer m_result;

protected:
	void Entry_Point(Thread* self);
	static void Static_Entry_Point(Thread* self, void* ptr)
	{
		reinterpret_cast<NetWebRequest*>(ptr)->Entry_Point(self);
	}

	std::string Url_Encode(std::string input);

public:
	~NetWebRequest();
	NetWebRequest();

	static void Crack_URL(std::string url, std::string& protocol, std::string& host, int& port, std::string& uri);

	float Get_Progress();

	void Add_URI_Parameter(std::string name, std::string value);
	void Add_URI_Parameter(std::string name, int value);
	void Add_URI_Parameter(std::string name, DataBuffer value);

	void Add_Body_Parameter(std::string name, std::string value);
	void Add_Body_Parameter(std::string name, int value);
	void Add_Body_Parameter(std::string name, DataBuffer value);

	bool Post(std::string host, int port, bool use_ssl, std::string uri);
	bool Get(std::string host, int port, bool use_ssl, std::string uri);

	bool Was_Success();
	DataBuffer Get_Result();

	void Cancel();
	bool Poll();

};

#endif

